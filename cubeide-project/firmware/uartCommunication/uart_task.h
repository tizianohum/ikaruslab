#ifndef UART_COMM_H
#define UART_COMM_H

#include <cstddef>
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

void UART_Comm_Init(void);

#ifdef __cplusplus
}
#endif

void UART_Send(const char* msg);
void UART_SendBinary(const uint8_t *data, size_t len);

#endif // UART_COMM_H
