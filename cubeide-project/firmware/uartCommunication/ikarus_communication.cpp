/*
 * ikarus_communication.cpp
 *
 *  Created on: Oct 8, 2025
 *      Author: tizianohumpert
 */

#ifndef UARTCOMMUNICATION_IKARUS_COMMUNICATION_CPP_
#define UARTCOMMUNICATION_IKARUS_COMMUNICATION_CPP_

#include "ikarus_communication.h"
#include "firmware.hpp"
#include "uart_task.h"
#include <sstream>
#include <string>
#include <cstdio>
#include "ikarus_protocoll.h"
#include <cstring>   // für memcpy

extern IKARUS_Firmware ikarus_firmware;

// global pointer auf aktive Instanz, damit MessageTask darauf zugreifen kann
IKARUS_CommunicationManager *active_manager = nullptr;

IKARUS_CommunicationManager::IKARUS_CommunicationManager() {
}

void IKARUS_CommunicationManager::init(ikarus_communication_config_t config) {
	this->config = config;
	active_manager = this;
	UART_Comm_Init(); // Initialisiert UART + Tasks
}

/**
 * Wird vom UART MessageTask aufgerufen, wenn eine komplette Zeile empfangen wurde.
 */
void IKARUS_CommunicationManager::processMessage(const char *msg) {
	std::string cmd, target, value;

	std::stringstream ss(msg);
	ss >> cmd >> target >> value;

	if (cmd == "SET") {
		handleCommand(target, value);
	} else if (cmd == "GET") {
		handleCommand(cmd, target);
	} else if (cmd == "PING") {
		send("PONG\n");
	} else {
		send("ERR: Unknown command\n");
	}
}

/**
 * Führt den jeweiligen Befehl aus.
 * Hier wird später mit Firmware oder Controller interagiert.
 */
void IKARUS_CommunicationManager::handleCommand(const std::string &cmd,
		const std::string &value) {
	if (cmd == "MOTOR_SPEED") {
		send(("Setting motor speed to " + value).c_str());
		//ikarus_firmware.motorController.setThrust

	} else if (cmd == "PID_KP") {
		send(("Setting PID Kp to " + value + "\n").c_str());
		// firmware.controller.setKp(std::stof(value));
	} else {
		send("ERR: Unknown parameter\n");
	}
}

/**
 * Senden über den Low-Level UART.
 */
void IKARUS_CommunicationManager::send(const char *msg) {
	UART_Send(msg);
}

void IKARUS_CommunicationManager::sendBinary(const uint8_t *data, size_t len) {
	UART_SendBinary(data, len);
}

void IKARUS_CommunicationManager::sendSample(
		 ikarus_log_data_t *sample) {
	ikarus_message_t msg;
	msg.start = 0xAA;
	msg.msg_type = IKARUS_MSG_SAMPLE_UPDATE;    // e.g. IKARUS_MSG_ESTIMATION
	msg.payload_length = sizeof(ikarus_log_data_t);

	// Copy binary state struct into payload
	memcpy(msg.payload, sample, sizeof(ikarus_log_data_t));

	// Compute CRC over header (start, type, length) + payload
	uint16_t crc = 0;
	crc += msg.start;
	crc += msg.msg_type;
	crc += msg.payload_length;
	for (int i = 0; i < msg.payload_length; i++)
		crc += msg.payload[i];

	msg.crc = static_cast<uint8_t>(crc & 0xFF);


	// Now send the whole message
	sendBinary(reinterpret_cast<uint8_t*>(&msg), sizeof(ikarus_message_t));
}


void IKARUS_CommunicationManager::processBinaryMessage(const uint8_t *data,
	size_t len) {
// --- 1️⃣ Grundprüfung ---
if (len < sizeof(ikarus_message_t)) {
	send("ERR: msg too short\n");
	return;
}

const ikarus_message_t *msg = reinterpret_cast<const ikarus_message_t*>(data);

// Startbyte prüfen
if (msg->start != IKARUS_MSG_START_BYTE) {
	send("ERR: invalid start\n");
	return;
}

// Länge prüfen
if (msg->payload_length > IKARUS_MSG_MAX_PAYLOAD) {
	send("ERR: invalid length\n");
	return;
}

// CRC prüfen
uint8_t calc_crc = ikarus_calc_crc(data, 3 + msg->payload_length);
if (calc_crc != msg->crc) {
	send("ERR: CRC mismatch\n");
	return;
}

// --- 2️⃣ Nachricht auswerten ---
switch (msg->msg_type) {

// =========================================================
// =============== ARMING ==================================
// =========================================================
case IKARUS_MSG_ARMING: {
	if (msg->payload_length != 1) {
		send("ERR: invalid arming payload\n");
		return;
	}

	uint8_t arm = msg->payload[0];

	if (arm == 1) {
		ikarus_firmware.controller.setArmedStatus(true);
		send("OK: armed\n");
	} else if (arm == 0) {
		ikarus_firmware.motorController.setThrust(0, 0, 0, 0);
		ikarus_firmware.controller.setArmedStatus(false);
		send("OK: disarmed\n");
	} else {
		send("ERR: invalid arming value\n");
	}
	break;
}

	// =========================================================
	// ============== THRUST (4 MOTOR FLOATS) ==================
	// =========================================================
case IKARUS_MSG_THRUST: {
	if (msg->payload_length != sizeof(ikarus_motor_thrust_t)) {
		send("ERR: invalid thrust payload\n");
		return;
	}

	ikarus_motor_thrust_t thrust;
	memcpy(&thrust, msg->payload, sizeof(thrust));

	ikarus_firmware.motorController.setThrust(thrust.motor1, thrust.motor2,
			thrust.motor3, thrust.motor4);

	send("OK: thrust\n");
	break;
}

	// =========================================================
	// ============== PITCH / ROLL / YAW (float) ===============
	// =========================================================
case IKARUS_MSG_PITCH:
case IKARUS_MSG_ROLL:
case IKARUS_MSG_YAW: {

	if (msg->payload_length != sizeof(float)) {
		send("ERR: invalid float payload\n");
		return;
	}

	float value;
	memcpy(&value, msg->payload, sizeof(float));

	switch (msg->msg_type) {
	case IKARUS_MSG_PITCH:
		ikarus_firmware.controller.setPitch(value);
		send("OK: pitch\n");
		break;

	case IKARUS_MSG_ROLL:
		ikarus_firmware.controller.setRoll(value);
		send("OK: roll\n");
		break;

	case IKARUS_MSG_YAW:
		ikarus_firmware.controller.setYaw(value);
		send("OK: yaw\n");
		break;
	}
	break;
}

	// =========================================================
	// ============ MOTOR 1–4 (single float) ===================
	// =========================================================
case IKARUS_MSG_MOTOR1:
case IKARUS_MSG_MOTOR2:
case IKARUS_MSG_MOTOR3:
case IKARUS_MSG_MOTOR4: {

	if (msg->payload_length != sizeof(float)) {
		send("ERR: invalid motor payload\n");
		return;
	}

	float value;
	memcpy(&value, msg->payload, sizeof(float));

	switch (msg->msg_type) {
	case IKARUS_MSG_MOTOR1:
		ikarus_firmware.motorController.setThrust1(value);
		send("OK: motor1\n");
		break;

	case IKARUS_MSG_MOTOR2:
		ikarus_firmware.motorController.setThrust2(value);
		send("OK: motor2\n");
		break;

	case IKARUS_MSG_MOTOR3:
		ikarus_firmware.motorController.setThrust3(value);
		send("OK: motor3\n");
		break;

	case IKARUS_MSG_MOTOR4:
		ikarus_firmware.motorController.setThrust4(value);
		send("OK: motor4\n");
		break;
	}
	break;
}

case IKARUS_MAG_CALIBRATE: {
	ikarus_firmware.sensors.gy271.calibrate(500, 20);
	break;
}
case IKARUS_SPECIAL_COMMAND: {
	uint16_t value;
	memcpy(&value, msg->payload, sizeof(uint16_t));
	ikarus_firmware.controller.special_command =value;
	break;

}

	// =========================================================
	// ================ UNBEKANNTER TYP ========================
	// =========================================================
default:
	send("ERR: unknown type\n");
	break;
}
}

#endif /* UARTCOMMUNICATION_IKARUS_COMMUNICATION_CPP_ */
