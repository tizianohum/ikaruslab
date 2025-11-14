// MotorController.cpp
#include "MotorController.hpp"
#include "firmware.hpp"

IKARUS_MotorController::IKARUS_MotorController() {
}

extern IKARUS_Firmware ikarus_firmware;

void IKARUS_MotorController::init(motor_controller_config_t *config) {
	this->config = *config;
	motors[0] = new Motor();
	motors[1] = new Motor();
	motors[2] = new Motor();
	motors[3] = new Motor();

	motor_config_t motor1Config = { .htim = config->htim1, .channel =
			config->channel_1 };
	motor_config_t motor2Config = { .htim = config->htim2, .channel =
			config->channel_2 };
	motor_config_t motor3Config = { .htim = config->htim3, .channel =
			config->channel_3 };
	motor_config_t motor4Config = { .htim = config->htim4, .channel =
			config->channel_4 };

	motors[0]->init(motor1Config);
	motors[1]->init(motor2Config);
	motors[2]->init(motor3Config);
	motors[3]->init(motor4Config);

	for (int i = 0; i < 4; i++) {
		if (!motors[i]->start()) {
			ikarus_firmware.firmware_state = IKARUS_FIRMWARE_STATE_ERROR;
		}
	}

	this->motorSemaphore = osSemaphoreNew(1, 1, NULL);
}

void IKARUS_MotorController::start() {
	// Check if any motor is the nullptr
	for (int i = 0; i < 4; i++) {
		if (motors[i] == nullptr) {
			ikarus_firmware.firmware_state = IKARUS_FIRMWARE_STATE_ERROR;

		}
	}
	this->initialized = true;
	this->setThrust(0, 0, 0, 0);
}

void IKARUS_MotorController::update() {
	if (!this->initialized) {
		// Error: MotorController not initialized
		return;
	}
	osSemaphoreAcquire(this->motorSemaphore, osWaitForever);
	motors[0]->setSignal(this->thrust1);
	motors[1]->setSignal(this->thrust2);
	motors[2]->setSignal(this->thrust3);
	motors[3]->setSignal(this->thrust4);

	for (int i = 0; i < 4; i++) {
		motors[i]->updatePWM();
	}
	osSemaphoreRelease(this->motorSemaphore);}

void IKARUS_MotorController::setThrust(float t1, float t2, float t3, float t4) {
	if (t1 > 300) t1 = 300;
	if (t2 > 300) t2 = 300;
	if (t3 > 300) t3 = 300;
	if (t4 > 300) t4 = 300;
    osSemaphoreAcquire(this->motorSemaphore, osWaitForever);
    thrust1 = t1;
    thrust2 = t2;
    thrust3 = t3;
    thrust4 = t4;
    osSemaphoreRelease(this->motorSemaphore);
}


void IKARUS_MotorController::updateAllMotors() {
	osSemaphoreAcquire(this->motorSemaphore, osWaitForever);
	for (int i = 0; i < 4; i++) {
		motors[i]->updatePWM();
	}
	osSemaphoreRelease(this->motorSemaphore);
}

