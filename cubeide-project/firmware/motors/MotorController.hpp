// MotorController.h
#pragma once
#include "Motor.hpp"
#include "cmsis_os.h"

typedef struct motor_controller_config_t {
	TIM_HandleTypeDef *htim1;
	uint32_t channel_1;
	TIM_HandleTypeDef *htim2;
	uint32_t channel_2;
	TIM_HandleTypeDef *htim3;
	uint32_t channel_3;
	TIM_HandleTypeDef *htim4;
	uint32_t channel_4;
} motor_controller_config_t;

class IKARUS_MotorController {
public:
	IKARUS_MotorController();

	void init(motor_controller_config_t *config);
	void start();
	void update();
	void setMotorSignals(uint16_t s1, uint16_t s2, uint16_t s3, uint16_t s4);
	void setThrust(float t1, float t2, float t3, float t4);
	void setThrust1(uint32_t thrust) { thrust1 = thrust; }
	void setThrust2(uint32_t thrust) { thrust2 = thrust;}
	void setThrust3(uint32_t thrust) { thrust3 = thrust; }
	void setThrust4(uint32_t thrust) { thrust4 = thrust;}



	void updateAllMotors();

	uint16_t thrust1 = 0;
	uint16_t thrust2 = 0;
	uint16_t thrust3 = 0;
	uint16_t thrust4 = 0;

private:
	Motor *motors[4] = { nullptr, nullptr, nullptr, nullptr };
	bool initialized = false;
	osSemaphoreId_t motorSemaphore;
	motor_controller_config_t config;
};
