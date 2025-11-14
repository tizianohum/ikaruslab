/*
 * firmware_defs.h
 *
 *  Created on: Sep 16, 2025
 *      Author: tizianohumpert
 */

#ifndef FIRMWARE_DEFS_H_
#define FIRMWARE_DEFS_H_

typedef enum ikarus_firmware_state_t {
	IKARUS_FIRMWARE_STATE_ERROR = -1,
	IKARUS_FIRMWARE_STATE_RUNNING = 1,
	IKARUS_FIRMWARE_STATE_UNARMED = 0,
} ikarus_firmware_state_t;

//#define IKARUS_FIRMWARE_SAMPLE_BUFFER_SIZE (uint16_t) (IKARUS_FIRMWARE_SAMPLE_BUFFER_TIME * 1000 / IKARUS_CONTROL_TS_MS)
#define IKARUS_FIRMWARE_SAMPLE_DMA_STREAM 0
#define IKARUS_FIRMWARE_SAMPLE_BUFFER_SIZE (uint16_t) 256

#endif /* FIRMWARE_DEFS_H_ */
