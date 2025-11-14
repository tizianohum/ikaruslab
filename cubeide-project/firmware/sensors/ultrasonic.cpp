#include "ultrasonic.hpp"
#include "stm32h7xx_hal.h"
#include "firmware.hpp"
#include "main.h"
#include "cmsis_os.h"

extern IKARUS_Firmware ikarus_firmware;

//define thread attributes
static const osThreadAttr_t ultrasonicTask_attributes = { .name = "ultrasonicSensor",
		.stack_size = 512,   // Achtung: CMSIS will BYTES, nicht Words!
		.priority = (osPriority_t) osPriorityNormal2, };

static osThreadId_t ultrasonicThreadId;


UltrasonicSensor::UltrasonicSensor() {
}

void UltrasonicSensor::init(ultrasonic_config_t *config) {
	this->frequence_tim = config->frequence_tim;
	this->counter_tim = config->counter_tim;
	this->triggerChannel = config->triggerChannel;
	this->echoChannel = config->echoPin;
}

void UltrasonicSensor::start() {
    // Start the timer that counts the time between sending and receiving the ultrasonic pulse
    HAL_TIM_Base_Start(this->counter_tim);

    // Ensure the trigger timer channel is in PWM mode and start it
    // This will generate the 10us pulse on the trigger pin
    __HAL_TIM_SET_COMPARE(this->frequence_tim, this->triggerChannel, 2); // 10us pulse, assuming 1MHz timer
    HAL_TIM_PWM_Start(this->frequence_tim, this->triggerChannel);

    // Start input capture in interrupt mode to detect the echo
    HAL_TIM_IC_Start_IT(this->frequence_tim, this->echoChannel);  // echoChannel used for input capture

    // Start the ultrasonic task thread (if using RTOS)
    ultrasonicThreadId = osThreadNew(start_ultrasonic_task, this,
            &ultrasonicTask_attributes);

    this->initialized = true;
}

void UltrasonicSensor::task() {
    for (;;) {
        osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);
        ikarus_firmware.sensors.ultrasonicSensor.handleExti(echo_Pin);
    }
}

float UltrasonicSensor::getDistance() {
	return this->distance;
}

void UltrasonicSensor::handleExti(uint16_t GPIO_Pin) {
	if (this->initialized == false) {
		return;
	}
	if (HAL_GPIO_ReadPin(echo_GPIO_Port, echo_Pin) == GPIO_PIN_SET) {
		this->echo_start = __HAL_TIM_GET_COUNTER(this->counter_tim);
		this->measuring = 1;
	}
	else if (this->measuring) {
		this->echo_end = __HAL_TIM_GET_COUNTER(this->counter_tim);
		if (this->echo_end >= this->echo_start)
			this->echo_duration = this->echo_end - this->echo_start;
		else
			this->echo_duration = (this->counter_tim->Init.Period + 1
					- this->echo_start) + this->echo_end;
		this->distance = (float) this->echo_duration / 58.0f; // Convert to cm
		if (this->distance <= 10) {
			HAL_GPIO_WritePin(ACT_LED_GPIO_Port, ACT_LED_Pin, GPIO_PIN_SET);
			} else {
			HAL_GPIO_WritePin(ACT_LED_GPIO_Port, ACT_LED_Pin, GPIO_PIN_RESET);
		}
         measuring = 0;

	}
}


void start_ultrasonic_task(void *argument) {
	UltrasonicSensor* sensor = (UltrasonicSensor*) argument;
	sensor->task();
}

extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == echo_Pin) {
		//ikarus_firmware.sensors.ultrasonicSensor.handleExti(GPIO_Pin);
        osThreadFlagsSet(ultrasonicThreadId, 0x01);

		return;
	}
}
