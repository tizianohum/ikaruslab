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
	float roll;
	float pitch;
	float yaw;
} ikarus_control_external_input_t;


typedef enum IKARUS_SPECIAL_COMMANDS_T {
    MOTOR1_BEEP = 1,
    MOTOR2_BEEP = 2,
    MOTOR3_BEEP = 3,
    MOTOR4_BEEP = 4,
    MOTOR1_REVERSE_SPIN = 5,
    MOTOR2_REVERSE_SPIN = 6,
    MOTOR3_REVERSE_SPIN = 7,
    MOTOR4_REVERSE_SPIN = 8
} IKARUS_SPECIAL_COMMANDS_T;



class Controller {
public:
	Controller();
	void init(controller_config_t config);
	void start();
	void task_fuction();
	uint16_t getThrust(void);
	bool getButtonState(void);
	void setArmedStatus(bool status) {
		armed = status;
	}
	bool getArmedStatus(void) {
		return armed;
	}

	void setPitch(float pitch) {
		_inputs.pitch = pitch;
	}
	void setRoll(float roll) {
		_inputs.roll = roll;
	}
	void setYaw(float yaw) {
		_inputs.yaw = yaw;
	}

	ikarus_control_external_input_t getControlInputs();
	uint16_t special_command = 0;


private:
	// Add private members for controller state
	controller_config_t config;
	bool initialized = false;
	bool armed = false;
	ikarus_control_external_input_t _inputs;
};

void controller_task(void *argument);
