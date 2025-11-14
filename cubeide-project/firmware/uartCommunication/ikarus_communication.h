/*
 * ikarus_communication.h
 *
 *  Created on: Oct 8, 2025
 *      Author: tizianohumpert
 */

#ifndef UARTCOMMUNICATION_IKARUS_COMMUNICATION_H_
#define UARTCOMMUNICATION_IKARUS_COMMUNICATION_H_

#include "uart_task.h"
#include "stdint.h"
#include "stm32h7xx_hal.h"
#include <string>

// Define error flag macros for communication error responses.
#define IKARUS_COMM_ERROR_FLAG_UNKNOWN         0x01  ///< Unknown error flag.
#define IKARUS_COMM_ERROR_FLAG_WRONG_ADDRESS   0x02  ///< Error flag
#define IKARUS_COMM_ERROR_FLAG_WRITE           0x03  ///< Error flag for write operation errors.
#define IKARUS_COMM_ERROR_FLAG_READ            0x04  ///< Error flag for read
#define IKARUS_COMM_ERROR_FLAG_LEN             0x05  ///< Error flag for length mismatches.
#define IKARUS_COMM_ERROR_FLAG_MSG_TYPE        0x06  ///< Error flag for

typedef struct ikarus_logging_sample_t{
	uint32_t error;
};

typedef struct ikarus_communication_config_t {
	UART_HandleTypeDef *huart;  ///< UART handle for communication.
} ikarus_communication_config_t;

class IKARUS_CommunicationManager {
public:
	IKARUS_CommunicationManager();
	void init(ikarus_communication_config_t config);
	void start();
    // Verarbeitung eingehender Nachrichten
    void processMessage(const char *msg);
    void processBinaryMessage(const uint8_t *data, size_t len);

    // Senden über UART
    void send(const char *msg);
    void sendBinary(const uint8_t *data, size_t len);

    ikarus_communication_config_t config;
    ikarus_logging_sample_t _sample_buffer_tx;
private:
    void handleCommand(const std::string &cmd, const std::string &value);

};

#endif /* UARTCOMMUNICATION_IKARUS_COMMUNICATION_H_ */
