/*
 * estimation.hpp
 *
 *  Created on: Sep 11, 2025
 *      Author: tizianohumpert
 */

#ifndef ESTIMATION_ESTIMATION_HPP_
#define ESTIMATION_ESTIMATION_HPP_

#include "sensors/IKARUS_sensors.hpp"
#include "cmsis_os.h"   // wichtig: CMSIS-RTOS API statt FreeRTOS direkt
#include "vqf.hpp"
#include "basicvqf.hpp"
#include "uartCommunication/ikarus_protocoll.h"

#define IKARUS_ESTIMATION_TS 0.01
#define IKARUS_ESTIMATION_BUFFER_SIZE 10
#define IKARUS_MSG_ORIENTATION  0x10  // frei wählbarer Typ


typedef enum ikarus_estimation_status_t {
	IKARUS_ESTIMATION_STAUTS_NONE = 0,
	IKARUS_ESTIMATION_STATUS_IDLE = 1,
	IKARUS_ESTIMATION_STATUS_OK = 2,
	IKARUS_ESTIMATION_STATUS_ERROR = -1,
} ikarus_estimation_status_t;

typedef struct ikarus_estimation_state_t {
//	float x;       // Position in x direction
//	float y;       // Position in y direction
//	float z;       // Position in z direction
	float roll;    // Roll angle
	float pitch;   // Pitch angle
	float yaw;     // Yaw angle
//	float x_dot;   // Velocity in x direction
//	float y_dot;   // Velocity in y direction
//	float z_dot;   // Velocity in z direction
	float roll_dot;  // Roll rate
	float pitch_dot; // Pitch rate
	float yaw_dot;   // Yaw rate
} ikarus_estimation_state_t;

typedef struct ikarus_estimation_config_t {
	// Add configuration parameters as needed
	IKARUS_Sensors *sensors;
} ikarus_estimation_config_t;




class IKARUS_Estimation {
public:
	IKARUS_Estimation();
	void init(ikarus_estimation_config_t config);
	void start();
	void reset();
	void stop();

	void update();
	void sendOrientationBuffer();

	void task_function();

	void setState(ikarus_estimation_state_t state);
	ikarus_estimation_state_t getStateEstimation();
	IKARUS_Sensors *sensors = nullptr;

	ikarus_estimation_status_t status;
	ikarus_estimation_state_t state;
	ikarus_estimation_state_t mean_state;
	ikarus_estimation_config_t config;
	uint16_t buffer_index = 0;
	ikarus_estimation_state_t orientation_buffer[IKARUS_ESTIMATION_BUFFER_SIZE];

private:
	BasicVQF vqf;
	float _theta_offset = 0;
};
void estimation_task(void *estimation);

#endif /* ESTIMATION_ESTIMATION_HPP_ */
