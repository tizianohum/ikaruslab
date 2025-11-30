/*
 * Control.cpp
 *
 *  Created on: Sep 11, 2025
 *      Author: tizianohumpert
 */


#include "Control.hpp"

IKARUS_ControlManager::IKARUS_ControlManager(){}


void IKARUS_ControlManager::init(ikarus_control_init_config_t config){
	this->config = config;
	this->_estimation = config.estimation;

	this->status = IKARUS_CONTROL_STATUS_IDLE;
	this->mode = IKARUS_CONTROL_MODE_OFF;

}

uint8_t IKARUS_ControlManager::start(){
	if(this->status != IKARUS_CONTROL_STATUS_IDLE){
		return HAL_ERROR;
	}
	this->status = IKARUS_CONTROL_STATUS_OK;

	this->_output.thrust1 = 0;
	this->_output.thrust2 = 0;
	this->_output.thrust3 = 0;
	this->_output.thrust4 = 0;

	return HAL_OK;
}

void IKARUS_ControlManager::update() {

    // 1. Inputs (desired angles)
    ikarus_control_external_input_t inputs = this->config.controller->getControlInputs();

    // 2. State estimation (angles + angular rates)
    ikarus_estimation_state_t state = this->_estimation->getStateEstimation();

    // 3. Parameter shortcut
    ikarus_control_params_t& P = this->params;

    // --- Angle errors ---
    float e_roll  = inputs.roll  - state.roll;
    float e_pitch = inputs.pitch - state.pitch;
    float e_yaw   = inputs.yaw   - state.yaw;


    // --- PD for Roll/Pitch ---
    float u_roll  = P.Kp_roll  * e_roll  - P.Kd_roll  * state.roll_dot;
    float u_pitch = P.Kp_pitch * e_pitch - P.Kd_pitch * state.pitch_dot;


    // --- PID (or PI) for Yaw ---
    // Derivative part
    float d_yaw = -P.Kd_yaw * state.yaw_dot;

    // Integrator with anti-windup
    P.yaw_integrator += P.Ki_yaw * e_yaw;
    if (P.yaw_integrator >  P.yaw_i_limit) P.yaw_integrator =  P.yaw_i_limit;
    if (P.yaw_integrator < -P.yaw_i_limit) P.yaw_integrator = -P.yaw_i_limit;

    float u_yaw = P.Kp_yaw * e_yaw + P.yaw_integrator + d_yaw;


    // --- Scale into thrust deltas ---
    float R = P.mix_roll  * u_roll;
    float T = P.mix_pitch * u_pitch;   // P for pitch → T for "tilt"
    float Y = P.mix_yaw   * u_yaw;


    // --- Base thrust (0..1 input mapped to DShot range) ---
    float T_base = P.thrust_min + P.base_thrust * (P.thrust_max - P.thrust_min);


    // --- Mixer: Quad X pattern ---

    // Motor numbering assumed:
    //   1: Front Left  (CCW)
    //   2: Front Right (CW)
    //   3: Rear  Right (CCW)
    //   4: Rear  Left  (CW)

    float T1 = T_base + R + T - Y; // FL
    float T2 = T_base - R + T + Y; // FR
    float T3 = T_base - R - T - Y; // RR
    float T4 = T_base + R - T + Y; // RL


    // --- Limit to legal DShot range ---
    auto clip = [&](float v) {
        if (v < P.thrust_min) return (float)P.thrust_min;
        if (v > P.thrust_max) return (float)P.thrust_max;
        return v;
    };

    T1 = clip(T1);
    T2 = clip(T2);
    T3 = clip(T3);
    T4 = clip(T4);


    // --- Set outputs ---
    this->_output.thrust1 = (uint16_t)T1;
    this->_output.thrust2 = (uint16_t)T2;
    this->_output.thrust3 = (uint16_t)T3;
    this->_output.thrust4 = (uint16_t)T4;


    // --- Send to motor controller ---
//    this->config.motorController->setThrust(
//        this->_output.thrust1,
//        this->_output.thrust2,
//        this->_output.thrust3,
//        this->_output.thrust4
//    );
}

//ikarus_control_mode_t IKARUS_ControlManager::getControlMode(){
//	return this->mode;
//}
//ikarus_control_status_t IKARUS_ControlManager::getControlStatus(){
//	return this->status;
//}
//ikarus_estimation_state_t IKARUS_ControlManager::getEstimatedState(){
//	return this->_estimation->getStateEstimation();
//}
