#include "Controller.hpp"
#include "main.h"


static const osThreadAttr_t controller_task_attributes = { .name = "controller",
		.stack_size = 1280 * 4, .priority = (osPriority_t) osPriorityNormal, };

Controller::Controller() {
	// Initialize controller state
}

void Controller::init(controller_config_t config) {
	this->config = config;
	// Additional initialization if needed
}

void Controller::start() {
	// Start controller operations if needed

	osThreadNew(controller_task, (void*) this, &controller_task_attributes);
	this->initialized = true;
}

void Controller::task_fuction() {

	while(true){
		osDelay(10); // 100 Hz
	}
}
//bool Controller::getButtonState(void){
//	// Return the state of the specified button
//	bool state = HAL_GPIO_ReadPin(arming_GPIO_Port, arming_Pin);
//	return state;
//}

uint16_t Controller::getThrust(void) {
	// Return the current thrust value

	return 100; // Placeholder implementation
}

ikarus_control_external_input_t Controller::getControlInputs(){
	return this->_inputs;
}


void controller_task(void *argument) {
	Controller *controller = (Controller*) argument;
	controller->task_fuction();
}
