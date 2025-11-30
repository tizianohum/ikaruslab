
/*
 * logger.c
 *
 *  Created on: Nov 17, 2025
 *      Author: tizianohumpert
 */

#include "logger.h"
#include "firmware.hpp"

extern IKARUS_Firmware ikarus_firmware;

IKARUS_Logger::IKARUS_Logger() {
}
void IKARUS_Logger::init(ikarus_logger_config_t config) {
	this->comm = config.comm;
}
void IKARUS_Logger::sendLog() {
	//TODO implement logging functionality
	_data.sensors = ikarus_firmware.sensors.getData();
	_data.estimation = ikarus_firmware.estimation.getStateEstimation();
	_data.controller_inputs = ikarus_firmware.controller.getControlInputs();
	_data.control_outputs = ikarus_firmware.controlManager.getOutputs();

	this->comm->sendSample(&_data);


}
