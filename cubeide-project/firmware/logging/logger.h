/*
 * logger.h
 *
 *  Created on: Nov 17, 2025
 *      Author: tizianohumpert
 */

#ifndef LOGGING_LOGGER_H_
#define LOGGING_LOGGER_H_
#include "sensors/IKARUS_Sensors.hpp"
#include "estimation/estimation.hpp"
#include "Controller/Controller.hpp"
#include "Control/Control.hpp"
#include "uartCommunication/ikarus_communication.h"
#include "logging_sample.h"


typedef struct ikarus_logger_config_t {
IKARUS_CommunicationManager *comm;
} ikarus_logger_config_t;



class IKARUS_Logger {
	public:
		IKARUS_Logger();
		void init(ikarus_logger_config_t);
		void sendLog(void);

		private:
		IKARUS_CommunicationManager *comm;
		ikarus_log_data_t _data;
};



#endif /* LOGGING_LOGGER_H_ */
