#pragma once
#include "stm32h7xx_hal.h"


typedef struct ultrasonic_config_t {
	TIM_HandleTypeDef *frequence_tim;
	TIM_HandleTypeDef *counter_tim;
	uint32_t triggerChannel;
	uint32_t echoPin;
} ultrasonic_config_t;

class UltrasonicSensor {
public:
	UltrasonicSensor();
	void init(ultrasonic_config_t *config);
	void start();
	float getDistance(); // Returns distance in cm
	void handleExti(uint16_t GPIO_Pin);
	void task();
	uint32_t echo_start = 0;
	uint32_t echo_end = 0;
	uint32_t echo_duration = 0;
	bool measuring = 0;
	TIM_HandleTypeDef *frequence_tim = nullptr;
	TIM_HandleTypeDef *counter_tim = nullptr;
	uint32_t triggerChannel = 0;
	uint32_t echoChannel = 0;
	float distance = 0; // Distance in cm

private:
	ultrasonic_config_t config;
	bool initialized = false;
};

void start_ultrasonic_task(void *argument);
