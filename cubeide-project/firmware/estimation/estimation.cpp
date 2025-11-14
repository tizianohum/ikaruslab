/*
 * estimation.cpp
 *
 *  Created on: Sep 11, 2025
 *      Author: tizianohumpert
 */

#include "estimation.hpp"
#include <math.h>
#include "string.h"
#include "firmware.hpp"

extern IKARUS_Firmware ikarus_firmware;

static const osThreadAttr_t estimation_task_attributes = {
    .name = "estimation",
    .stack_size = 2048,           // 2 KB reicht locker
    .priority = (osPriority_t) osPriorityNormal,
};
IKARUS_Estimation::IKARUS_Estimation() :
		vqf(IKARUS_ESTIMATION_TS) {
	this->vqf.setTauAcc(0.5);
			this->vqf.setTauMag(0.01);

}

void IKARUS_Estimation::init(ikarus_estimation_config_t config) {
	this->config = config;
	this->sensors = config.sensors;
	this->status = IKARUS_ESTIMATION_STATUS_IDLE;
}

void IKARUS_Estimation::start() {
	osThreadNew(estimation_task, (void*) this, &estimation_task_attributes);
}

void IKARUS_Estimation::task_function() {
	this->status = IKARUS_ESTIMATION_STATUS_OK;

	while (true) {
		this->update();

		osDelay(10); // 100 Hz
	}
}

void IKARUS_Estimation::update() {
	//TODO
	this->sensors->update();
	ikarus_sensors_data_t data = this->sensors->getData();

//	vqf_real_t gyr[3] = { data.gyrX, data.gyrY, data.gyrZ };
//	vqf_real_t acc[3] = { data.accX, data.accY, data.accZ };
	vqf_real_t gyr[3] = { data.gyr.x, data.gyr.y, data.gyr.z };
	vqf_real_t acc[3] = { data.acc.x, data.acc.y, data.acc.z };
	vqf_real_t mag[3] = { data.magY, -data.magX, data.magZ }; // verdrehte axhsen der sensoren

	float norm = sqrtf(mag[0]*mag[0] + mag[1]*mag[1] + mag[2]*mag[2]);
	if (norm > 1e-6f) {
	    mag[0] /= norm;
	    mag[1] /= norm;
	    mag[2] /= norm;
	}

	vqf.update(gyr, acc,mag);

	vqf_real_t quat[4];
	vqf.getQuat9D(quat);

	float w = quat[0];
	float x = quat[1];
	float y = quat[2];
	float z = quat[3];

    // --- Euler-Winkel berechnen ---
    // Formel aus konventioneller Quaternion->Euler-Umrechnung (Z-Y-X, also Yaw-Pitch-Roll)
    float sinr_cosp = 2.0f * (w * x + y * z);
    float cosr_cosp = 1.0f - 2.0f * (x * x + y * y);
    float roll = atan2(sinr_cosp, cosr_cosp);

    float sinp = 2.0f * (w * y - z * x);
    float pitch = (fabs(sinp) >= 1.0f) ? copysign(M_PI / 2.0f, sinp) : asin(sinp);

    float siny_cosp = 2.0f * (w * z + x * y);
    float cosy_cosp = 1.0f - 2.0f * (y * y + z * z);
    float yaw = atan2(siny_cosp, cosy_cosp);

    // --- In Grad umrechnen ---
    float roll_deg  = roll  * 180.0f / M_PI;
    float pitch_deg = pitch * 180.0f / M_PI;
    float yaw_deg   = yaw   * 180.0f / M_PI;

    // --- Zustand speichern ---
    this->state.roll  = roll_deg;
    this->state.pitch = pitch_deg;
    this->state.yaw   = yaw_deg;

    // --- In Ringbuffer schreiben ---
    this->orientation_buffer[this->buffer_index].roll  = roll_deg;
    this->orientation_buffer[this->buffer_index].pitch = pitch_deg;
    this->orientation_buffer[this->buffer_index].yaw   = yaw_deg;

    this->buffer_index++;
    if (this->buffer_index >= IKARUS_ESTIMATION_BUFFER_SIZE) {
        this->buffer_index = 0; // Ringpuffer
        //this->sendOrientationBuffer();
    }




}


void IKARUS_Estimation::sendOrientationBuffer() {

	ikarus_message_t msg;
	msg.start = IKARUS_MSG_START_BYTE;
	msg.msg_type = IKARUS_MSG_ORIENTATION;
	msg.payload_length = sizeof(ikarus_estimation_state_t) * IKARUS_ESTIMATION_BUFFER_SIZE;
	memcpy(msg.payload, &orientation_buffer, sizeof(orientation_buffer));
	msg.crc = ikarus_calc_crc(reinterpret_cast<const uint8_t*>(&msg), 3 + msg.payload_length);

	ikarus_firmware.comm.sendBinary(reinterpret_cast<const uint8_t*>(&msg), 3 + msg.payload_length + 1);
}
ikarus_estimation_state_t IKARUS_Estimation::getStateEstimation() {
	return this->state;
}

void estimation_task(void *estimation) {
	IKARUS_Estimation *estimator = (IKARUS_Estimation*) estimation;
	estimator->task_function();
}
