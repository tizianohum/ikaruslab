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

IKARUS_CommunicationManager::IKARUS_CommunicationManager() {}

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
void IKARUS_CommunicationManager::handleCommand(const std::string &cmd, const std::string &value) {
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

void IKARUS_CommunicationManager::processBinaryMessage(const uint8_t *data, size_t len)
{
    // --- 1️⃣ Gültigkeit prüfen ---
    if (len < sizeof(ikarus_message_t)) {
        send("ERR: msg too short\n");
        return;
    }

    const ikarus_message_t *msg = reinterpret_cast<const ikarus_message_t *>(data);

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

    // --- 2️⃣ Nachrichtentyp auswerten ---
    switch (msg->msg_type) {

        // ====== MOTOR THRUST ======
        case IKARUS_MSG_THRUST: {
            if (msg->payload_length < sizeof(ikarus_motor_thrust_t)) {
                send("ERR: invalid thrust payload\n");
                return;
            }

            ikarus_motor_thrust_t thrust;
            memcpy(&thrust, msg->payload, sizeof(thrust));

            // Motorsteuerung aufrufen
            ikarus_firmware.motorController.setThrust(
                thrust.motor1, thrust.motor2, thrust.motor3, thrust.motor4);

            send("OK: thrust\n");
            break;
        }

        case IKARUS_MSG_ARMING: {
			if (msg->payload_length < 1) {
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

//        // ====== PID CONFIG ======
//        case IKARUS_MSG_PID: {
//            if (msg->payload_length < sizeof(ikarus_pid_t)) {
//                send("ERR: invalid pid payload\n");
//                return;
//            }
//
//            ikarus_pid_t pid;
//            memcpy(&pid, msg->payload, sizeof(pid));
//
//            ikarus_firmware.controller.setPID(pid.kp, pid.ki, pid.kd);
//            send("OK: pid\n");
//            break;
//        }



        // ====== PING ======
        case IKARUS_MSG_PING: {
            ikarus_message_t pong = {
                .start = IKARUS_MSG_START_BYTE,
                .msg_type = IKARUS_MSG_PING,
                .payload_length = 0,
                .crc = ikarus_calc_crc(reinterpret_cast<const uint8_t*>(&pong), 3)
            };
            //send(reinterpret_cast<const char*>(&pong), 4);
            break;
        }

        // ====== UNBEKANNTER TYP ======
        default:
            send("ERR: unknown type\n");
            break;
    }
}

#endif /* UARTCOMMUNICATION_IKARUS_COMMUNICATION_CPP_ */
