/*
 * ikarus_protocoll.h
 *
 *  Created on: Oct 9, 2025
 *      Author: tizianohumpert
 */

#ifndef UARTCOMMUNICATION_IKARUS_PROTOCOLL_H_
#define UARTCOMMUNICATION_IKARUS_PROTOCOLL_H_

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================
 *   IKARUS UART MESSAGE PROTOCOL DEFINITIONS
 * ============================================
 * Einheitliches Protokoll zwischen PC <-> MCU
 * Struktur: Header (3B) + Payload (variable) + CRC (1B)
 * --------------------------------------------
 *  [0] start          -> 0xAA
 *  [1] msg_type       -> Nachrichtentyp (siehe enum)
 *  [2] payload_length -> Länge des Payloads in Bytes
 *  [3..N] payload     -> Nutzdaten (max. 64 Bytes)
 *  [N+1] crc          -> einfache 8-bit Summe über alles davor
 * --------------------------------------------
 */

#define IKARUS_MSG_START_BYTE 0xAA
#define IKARUS_MSG_MAX_PAYLOAD 100

/* === Nachrichtentypen === */
typedef enum {
	IKARUS_MSG_THRUST = 1,
	IKARUS_MSG_ARMING = 0,
	IKARUS_MSG_PITCH  = 2,
	IKARUS_MSG_ROLL   = 3,
	IKARUS_MSG_YAW    = 4,

	IKARUS_MSG_MOTOR1 = 5,
	IKARUS_MSG_MOTOR2 = 6,
	IKARUS_MSG_MOTOR3 = 7,
	IKARUS_MSG_MOTOR4 = 8,

	IKARUS_MSG_SAMPLE_UPDATE = 10
} ikarus_msg_type_t;

typedef struct __attribute__((packed)) {
    float value;
} ikarus_float_t;

/* === Allgemeine Nachrichtenstruktur === */
#pragma pack(push, 1)
typedef struct {
    uint8_t start;                          // 0xAA
    uint8_t msg_type;                       // Nachrichtentyp
    uint8_t payload_length;                 // Länge des Payloads
    uint8_t payload[IKARUS_MSG_MAX_PAYLOAD];// Daten
    uint8_t crc;                            // Prüfsumme (sum(header + payload))
} ikarus_message_t;
#pragma pack(pop)

/* === Payload-Strukturen === */

#pragma pack(push, 1)
typedef struct {
    float motor1;
    float motor2;
    float motor3;
    float motor4;
} ikarus_motor_thrust_t;

typedef struct {
    float kp;
    float ki;
    float kd;
} ikarus_pid_t;

typedef struct {
    float x;
    float y;
    float z;
} ikarus_waypoint_t;
#pragma pack(pop)

/* === CRC / Hilfsfunktionen === */
static inline uint8_t ikarus_calc_crc(const uint8_t *data, uint16_t len)
{
    uint16_t sum = 0;
    for (uint16_t i = 0; i < len; i++)
        sum += data[i];
    return (uint8_t)(sum & 0xFF);
}

#ifdef __cplusplus
}
#endif



#endif /* UARTCOMMUNICATION_IKARUS_PROTOCOLL_H_ */
