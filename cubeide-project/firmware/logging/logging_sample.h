/*
 * logging_sample.h
 *
 *  Created on: Nov 17, 2025
 *      Author: tizianohumpert
 */

#ifndef LOGGING_LOGGING_SAMPLE_H_
#define LOGGING_LOGGING_SAMPLE_H_

#include "sensors/IKARUS_Sensors.hpp"
#include "estimation/estimation.hpp"
#include "Control/Control.hpp"
#include "Controller/Controller.hpp"


typedef struct ikarus_log_data_t {
	ikarus_sensors_data_t sensors;
	ikarus_estimation_state_t estimation;
	ikarus_control_outputs_t control_outputs;
	ikarus_control_external_input_t controller_inputs;

} ikarus_log_data_t;



#endif /* LOGGING_LOGGING_SAMPLE_H_ */
