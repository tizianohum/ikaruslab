#pragma once
#include <cstdint>
#include "main.h"
#include "cmsis_os.h"

typedef struct controller_config_t {
	// Add configuration parameters as needed
	UART_HandleTypeDef *huart;
	//IKARUS_MotorController *motorController;
} controller_config_t;

// Inputs from the Controller
typedef struct ikarus_control_external_input_t {
	uint32_t thrust;
	uint32_t roll;
	uint32_t pitch;
	uint32_t yaw;
} ikarus_control_external_input_t;

class Controller {
public:
	Controller();
	void init(controller_config_t config);
	void start();
	void task_fuction();
	uint16_t getThrust(void);
	bool getButtonState(void);
	void setArmedStatus(bool status) { armed = status; }
	bool getArmedStatus(void){return armed;}
	ikarus_control_external_input_t getControlInputs();


private:
	// Add private members for controller state
	controller_config_t config;
	bool initialized = false;
	bool armed = false;
	ikarus_control_external_input_t _inputs;
};

void controller_task(void *argument);
