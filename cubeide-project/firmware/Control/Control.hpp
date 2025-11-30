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



typedef struct ikarus_control_outputs_t {
	uint16_t thrust1;
	uint16_t thrust2;
	uint16_t thrust3;
	uint16_t thrust4;
} ikarus_control_outputs_t;

typedef struct ikarus_control_params_t {

    // --- Roll Controller (Angle) ---
    float Kp_roll;   // proportional gain for roll angle
    float Kd_roll;   // derivative gain (gyro-based)

    // --- Pitch Controller (Angle) ---
    float Kp_pitch;
    float Kd_pitch;

    // --- Yaw Controller (Angle or rate) ---
    float Kp_yaw;
    float Ki_yaw;    // optional, can be 0
    float Kd_yaw;

    float yaw_integrator;   // integrator storage
    float yaw_i_limit;      // anti-windup limit

    // --- Mixer Scaling ---
    float mix_roll;   // scales roll control output -> motor delta
    float mix_pitch;  // scales pitch control output -> motor delta
    float mix_yaw;    // scales yaw control output -> motor delta

    // --- Thrust Limits ---
    uint16_t thrust_min;   // e.g. 47  (DShot300)
    uint16_t thrust_max;   // e.g. 2048

    // --- Collective Thrust ---
    float base_thrust;     // 0..1 as normalized throttle input

    // --- Optional Filters ---
    float gyro_lpf_cutoff;   // low-pass filter for gyro [Hz]
    float dterm_lpf_cutoff;  // low-pass for D-term

} ikarus_control_params_t;

class IKARUS_ControlManager{
public:
	IKARUS_ControlManager();

	void init(ikarus_control_init_config_t config);
	uint8_t start();

	void stop();

	void reset();

	void update();

	ikarus_control_outputs_t getOutputs(){
        return this->_output;
    }

	uint8_t setMode(ikarus_control_mode_t mode);
	ikarus_control_status_t getStatus();

	ikarus_control_status_t status = IKARUS_CONTROL_STATUS_IDLE;
	ikarus_control_mode_t mode = IKARUS_CONTROL_MODE_OFF;

	ikarus_control_init_config_t config;
	ikarus_control_configuratioin control_config;

private:
	ikarus_control_params_t params;

	ikarus_control_outputs_t _output;
	ikarus_estimation_state_t _dynamic_state;
	ikarus_control_data_t _data;

	IKARUS_Estimation *_estimation = NULL;

};

#endif // IKARUS_FIRMWARE_CONTROL_CONTROL_HPP
