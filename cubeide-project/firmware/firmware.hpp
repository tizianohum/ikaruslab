#pragma once

#include "firmware_core.h"
#include "Control/Control.hpp"

#include "motors/MotorController.hpp"
#include "sensors/IKARUS_Sensors.hpp"
#include "Controller/Controller.hpp"
#include "sensors/ultrasonic.hpp"
#include "sensors/MPU6050.h"
#include "estimation/estimation.hpp"
#include "uartCommunication/ikarus_communication.h"
#ifdef __cplusplus
extern "C" {
#endif


class IKARUS_Firmware {
	public:
        IKARUS_Firmware();
        HAL_StatusTypeDef init();
        HAL_StatusTypeDef start();

        void helperTask();
        void task();

        IKARUS_CommunicationManager comm;
        IKARUS_ControlManager controlManager;
        IKARUS_MotorController motorController;
        IKARUS_Sensors sensors;
        Controller controller;
        IKARUS_Estimation estimation;

        ikarus_firmware_state_t firmware_state = IKARUS_FIRMWARE_STATE_UNARMED;

        //UltrasonicSensor ultrasonicSensor;
private:
        //core_comm_UartInterface<8, 128> uart_if;
        uint16_t samples_counter = 0;
};

void start_firmware_task(void *argument);
void start_firmware_control_task(void *argument);


#ifdef __cplusplus
}
#endif
