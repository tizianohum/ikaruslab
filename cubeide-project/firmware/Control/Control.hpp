/*
 * Control.hpp
 *
 * Created on: 11 Sep 2025
 * 	Author: Tiziano Humpert
 */
#ifndef IKARUS_FIRMWARE_CONTROL_CONTROL_HPP
#define IKARUS_FIRMWARE_CONTROL_CONTROL_HPP
#include "estimation/estimation.hpp"
#include "Controller/Controller.hpp"
#include "motors/MotorController.hpp"

typedef struct ikarus_control_init_config_t {
	IKARUS_Estimation *estimation;
	Controller *controller;
	IKARUS_MotorController *motorController;

} ikarus_control_init_config_t;

typedef struct ikarus_control_configuratioin{
	//TODO
	float K[8];

}ikarus_control_configuratioin;

typedef struct ikarus_control_data_t{
	//TODO needed?
}ikarus_control_data_t;

typedef enum ikarus_control_mode_t {
	IKARUS_CONTROL_MODE_OFF = 0, IKARUS_CONTROL_MODE_PID = 1,
} ikarus_control_mode_t;

typedef enum ikarus_control_status_t {
//TODO
	IKARUS_CONTROL_STATUS_IDLE = 0,
	IKARUS_CONTROL_STATUS_OK = 1,
} ikarus_control_status_t;



typedef struct ikarus_control_output_t {
	uint16_t omega1;
	uint16_t omega2;
	uint16_t omega3;
	uint16_t omega4;

} ikarus_control_output_t;

class IKARUS_ControlManager{
public:
	IKARUS_ControlManager();

	void init(ikarus_control_init_config_t config);
	uint8_t start();

	void stop();

	void reset();

	void update();

	uint8_t setMode(ikarus_control_mode_t mode);
	ikarus_control_status_t getStatus();

	ikarus_control_status_t status = IKARUS_CONTROL_STATUS_IDLE;
	ikarus_control_mode_t mode = IKARUS_CONTROL_MODE_OFF;

	ikarus_control_init_config_t config;
	ikarus_control_configuratioin control_config;

private:
	ikarus_control_output_t _output;
	ikarus_estimation_state_t _dynamic_state;
	ikarus_control_data_t _data;

	IKARUS_Estimation *_estimation = NULL;

};

#endif // IKARUS_FIRMWARE_CONTROL_CONTROL_HPP
