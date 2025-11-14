/*
 * firmware_core.h
 *
 *  Created on: Sep 16, 2025
 *      Author: tizianohumpert
 */

#ifndef FIRMWARE_CORE_H_
#define FIRMWARE_CORE_H_

#include "stm32h7xx.h"
//#include "core.h"
#include "firmware_adresses.h"
#include "firmware_defs.h"
#include "firmware_settings.h"

extern uint32_t tick_global;

extern void send_debug(const char *format, ...);
extern void send_info(const char *format, ...);
extern void send_warning(const char *format, ...);
extern void send_error(const char *format, ...);


//extern void setFirmwareStateError();
extern void stopControl();

#endif /* FIRMWARE_CORE_H_ */
