#include "main.h"

#include "firmware.hpp"
#include "firmware_c.h"
#include "stm32h7xx_hal.h"
#include "cmsis_os.h"   // CMSIS-RTOS API statt FreeRTOS direkt
#include "firmware_settings.h"
#include "stm32h7xx_hal_i2c.h"

/*Global Firmware Instance */
IKARUS_Firmware ikarus_firmware;

// extern handles from main.c
extern TIM_HandleTypeDef htim1; // CH1,5 for Motors
extern TIM_HandleTypeDef htim15; // CH1,2 for Motors
extern TIM_HandleTypeDef htim4; // C
extern TIM_HandleTypeDef htim17; //
extern TIM_HandleTypeDef htim16; //counter
extern UART_HandleTypeDef huart7;
extern I2C_HandleTypeDef hi2c2;

/* Set the global tick */
uint32_t tick_global = 0;

//define thread attributes
static const osThreadAttr_t firmwareTask_attributes = {
		.name = "FirmwareHelper", .stack_size = 1200 * 4, // Achtung: CMSIS will BYTES, nicht Words!
		.priority = (osPriority_t) osPriorityNormal2, };

static const osThreadAttr_t control_task_attributes =
		{ .name = "control", .stack_size = 2560 * 4, .priority =
				(osPriority_t) osPriorityAboveNormal, };

// start firmware task -> called in main.c
void firmware(void) {
	osThreadNew(start_firmware_task, (void*) &ikarus_firmware,
			&firmwareTask_attributes);
}
void start_firmware_task(void *argument) {
	IKARUS_Firmware *firmware = (IKARUS_Firmware*) argument;
	// Start the helper task (core firmware loop)
	firmware->helperTask();
}

IKARUS_Firmware::IKARUS_Firmware() {
	// Additional initialization if needed
}

void IKARUS_Firmware::helperTask() {
	// Initialize firmware modules and configurations

	for (uint8_t addr = 1; addr < 127; addr++) {
		uint8_t i2cAddr = addr << 1;
		HAL_StatusTypeDef result;

		// Versuch mit leerem Write (reagiert am zuverlässigsten)
		uint8_t dummy = 0;
		result = HAL_I2C_Master_Transmit(&hi2c2, i2cAddr, &dummy, 1, 5);

		if (result == HAL_OK) {
			uint8_t test = 1;
		}
//	        else if (result == HAL_BUSY) {
//	            // Bus hängt, resetten
//	        	if (__HAL_I2C_GET_FLAG(&hi2c2, I2C_FLAG_BUSY)) {
//	        	        __HAL_I2C_DISABLE(&hi2c2);
//	        	        HAL_Delay(1);
//	        	        __HAL_I2C_ENABLE(&hi2c2);
//	        	    }
//	        		        }

		HAL_Delay(2);
	}

	HAL_StatusTypeDef status;
	status = this->init();
	if (status == HAL_ERROR) {
		//TODO: Error Handling
		return;
	}
	status = this->start();
	if (status == HAL_ERROR) {
		//TODO: Error Handling
		return;
	}
//tasks always have to run infinity unless they are deleted correctly -> otherwise jumps into a fault handler
//    for (;;) {
//        osDelay(10);
//    }
	vTaskDelete(NULL); //stop task because right now it is not needed anymore

}
HAL_StatusTypeDef IKARUS_Firmware::init() {

	ikarus_communication_config_t ikarus_comm_config = { .huart = &huart7 };
	this->comm.init(ikarus_comm_config);
	this->comm.send("Communication ready for commands"); // test communication

	motor_controller_config_t motorontrollerConfig = { .htim1 = &htim1,
			.channel_1 = TIM_CHANNEL_1, .htim2 = &htim1, .channel_2 =
					TIM_CHANNEL_4, .htim3 = &htim15, .channel_3 = TIM_CHANNEL_1,
			.htim4 = &htim4, .channel_4 = TIM_CHANNEL_3, };
	this->motorController.init(&motorontrollerConfig);

	controller_config_t controllerConfig = {
	//		.motorController = &motorController
			};
	this->controller.init(controllerConfig);

	//config of the sensors -> initialization in sensors.init()
	ultrasonic_config_t ultrasonicConfig = { .frequence_tim = &htim17,
			.counter_tim = &htim16, .triggerChannel = TIM_CHANNEL_1, .echoPin =
			echo_Pin };
	mpu6050_config_t imuConfig = { .address = MPU6050_ADDR, .hi2c = &hi2c2,
			.acc_range = MPU6050_ACC_RANGE_8G, .gyr_range =
					MPU6050_GYR_RANGE_500, };
	gy271_config_t gyConfig = { .hi2c = &hi2c2, };

	this->sensors.init(&ultrasonicConfig, &imuConfig, &gyConfig);

	// Estimation
	ikarus_estimation_config_t estimationConfig = { .sensors = &sensors };
	this->estimation.init(estimationConfig);

	ikarus_control_init_config_t controlManagerConfig = { .estimation =
			&estimation, .controller = &controller, .motorController =
			&motorController };
	this->controlManager.init(controlManagerConfig);
	return HAL_OK;
}

HAL_StatusTypeDef IKARUS_Firmware::start() {

	this->motorController.start();
	this->sensors.start();
	this->estimation.start();
	this->controller.start();
	this->controlManager.start();

	osThreadNew(start_firmware_control_task, (void*) &ikarus_firmware,
			&control_task_attributes);

	this->firmware_state = IKARUS_FIRMWARE_STATE_UNARMED;

	//sometimes when too short time after opening new thread -> thread not started
	osDelay(50);
	return HAL_OK;
}

void IKARUS_Firmware::task() {

    while (true) {

        switch (this->firmware_state) {

        case IKARUS_FIRMWARE_STATE_UNARMED: {
            uint8_t status = 1;

            if (this->controller.getArmedStatus()) {
                for (uint8_t i = 0; i < 160; i++) {
                    if (!this->controller.getArmedStatus()) {
                        status = 0;
                        break;
                    }

                    this->motorController.update();
                    osDelay(25);
                }

                if (status) {
                    this->firmware_state = IKARUS_FIRMWARE_STATE_RUNNING;
                    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
                }
            }

            osDelay(25);
            break;
        }

        case IKARUS_FIRMWARE_STATE_RUNNING: {
            if (!this->controller.getArmedStatus()) {
                this->firmware_state = IKARUS_FIRMWARE_STATE_UNARMED;
                HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
                this->motorController.setThrust(0, 0, 0, 0);
                osDelay(25);
                break;
            }

            this->controlManager.update();
            this->motorController.update();

            this->samples_counter++;
            if (this->samples_counter >= 10) {
                this->samples_counter = 0;

                comm.sendSample(&estimation.state);
            }

            // Wait until next cycle
            osDelay(25);
            break;
        }

        case IKARUS_FIRMWARE_STATE_ERROR: {
            //TODO
            while (1) {
                osDelay(1000);
            }
            break;
        }

        } // switch
    } // while
}

void start_firmware_control_task(void *argument) {
	IKARUS_Firmware *firmware = (IKARUS_Firmware*) argument;
	firmware->task();
}
