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

	this->_output.omega1 = 0;
	this->_output.omega2 = 0;
	this->_output.omega3 = 0;
	this->_output.omega4 = 0;

	return HAL_OK;
}

void IKARUS_ControlManager::update(){
	//1. get inputs from controller
	ikarus_control_external_input_t inputs = this->config.controller->getControlInputs();

	//2. get state estimation
	ikarus_estimation_state_t state = this->_estimation->getStateEstimation();




	this->_output.omega1 = 0;
	this->_output.omega2 = 0;
	this->_output.omega3 = 0;
	this->_output.omega4 = 0;

	//this->config.motorController->setThrust(this->_output.omega1, this->_output.omega2, this->_output.omega3, this->_output.omega4);

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
