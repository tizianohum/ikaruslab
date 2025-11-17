/*
 * uart_task.cpp
 *
 * Created on: Oct 8, 2025
 * Author: tizianohumpert
 *
 * Optimierte UART-Kommunikation mit echtem Circular DMA, Ringbuffer und CMSIS-RTOS2.
 * - DMA läuft dauerhaft (kein Restart im Callback)
 * - Neue Bytes werden per Positionserkennung in den Ringbuffer kopiert
 * - MessageTask verarbeitet kontinuierlich eingehende Daten
 */
#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#ifdef __cplusplus
}
#endif
#include "usart.h"
#include <stdio.h>
#include <string.h>
#include "uart_task_c.h"
#include "uart_task.h"
#include "RingBuffer.h"
#include "cmsis_os.h"   // CMSIS-RTOS2 API
#include "ikarus_communication.h"
#include "ikarus_protocoll.h"

extern IKARUS_CommunicationManager *active_manager;


/* === Configuration === */
#define RX_BUFFER_SIZE 128
#define TX_BUFFER_SIZE 128

/* === Handles === */
static osMessageQueueId_t uartTxQueueHandle;
static osThreadId_t uartRxTaskHandle;
static osThreadId_t uartTxTaskHandle;
static osThreadId_t uartMsgTaskHandle;

/* === Buffers === */
static uint8_t rxBuffer[RX_BUFFER_SIZE];
static char txBuffer[TX_BUFFER_SIZE];

/* === Ringbuffer === */
static RingBuffer<uint8_t, 512> uartRxBuffer;

/* === Positionstracking für Circular DMA === */
static volatile uint16_t old_pos = 0;

/* === Forward declarations === */
static void UartRxTask(void *argument);
static void UartTxTask(void *argument);
static void MessageTask(void *argument);

/* === Thread Attributes === */
const osThreadAttr_t uartRxTask_attributes = {
    .name = "UART_RX",
    .stack_size = 256 * 4, // Bytes, nicht Words
    .priority = (osPriority_t) osPriorityAboveNormal,
};

const osThreadAttr_t uartTxTask_attributes = {
    .name = "UART_TX",
    .stack_size = 256 * 4,
    .priority = (osPriority_t) osPriorityAboveNormal,/////////zu niedrig falls nicht mehr sendet
};

const osThreadAttr_t uartMsgTask_attributes = {
    .name = "UART_MSG",
    .stack_size = 512 * 4,                 // ruhig etwas Reserve
    .priority = (osPriority_t) osPriorityAboveNormal,
};

/* === Initialization === */
void UART_Comm_Init(void)
{
    uartRxBuffer.reset();

    // CMSIS-RTOS2 MessageQueue anstelle von FreeRTOS Queue
    uartTxQueueHandle = osMessageQueueNew(10, TX_BUFFER_SIZE, nullptr);
    // Threads via CMSIS-RTOS2 API starten
    uartRxTaskHandle  = osThreadNew(UartRxTask,  nullptr, &uartRxTask_attributes);
    uartTxTaskHandle  = osThreadNew(UartTxTask,  nullptr, &uartTxTask_attributes);
    uartMsgTaskHandle = osThreadNew(MessageTask, nullptr, &uartMsgTask_attributes);

	osDelay(50); // Kurze Pause, damit System stabil ist

}

/* === RX Task === */
static void UartRxTask(void *argument)
{    // Starte UART DMA EINMALIG im Circular Mode
    if (osKernelGetState() == osKernelRunning) {
        HAL_UART_Receive_DMA(&huart7, rxBuffer, RX_BUFFER_SIZE);
        __HAL_UART_ENABLE_IT(&huart7, UART_IT_IDLE);
    }

    osDelay(10);
    vTaskDelete(NULL); // Task beenden, da DMA im Hintergrund läuft
}

/* === UART Idle Callback === */
extern "C" void UART_IdleCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance != UART7)
        return;

    if (__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE))
    {
        __HAL_UART_CLEAR_IDLEFLAG(huart);

        uint16_t dma_pos = RX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart->hdmarx);
        uint16_t bytes_to_copy = (dma_pos >= old_pos)
                                ? (dma_pos - old_pos)
                                : (RX_BUFFER_SIZE - old_pos + dma_pos);

        for (uint16_t i = 0; i < bytes_to_copy; i++)
        {
            uint16_t index = (old_pos + i) % RX_BUFFER_SIZE;
            uartRxBuffer.put(rxBuffer[index]);
        }

        old_pos = dma_pos;
        osThreadFlagsSet(uartMsgTaskHandle, 0x01);
    }
}

///* === Message Task === */
//static void MessageTask(void *argument)
//{
//    uint8_t byte;
//    char msgBuf[RX_BUFFER_SIZE];
//    uint8_t index = 0;
//
//    for (;;) {
//        osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);
//
//        while (uartRxBuffer.get(byte)) {
//            if (byte == '\n' || byte == '\r') {
//                msgBuf[index] = '\0';
//                if (active_manager != nullptr) {
//                    active_manager->processMessage(msgBuf);
//                }
//                index = 0;
//            } else if (index < sizeof(msgBuf) - 1) {
//                msgBuf[index++] = byte;
//            }
//        }
//    }
//}

static void MessageTask(void *argument)
{
    uint8_t byte;
    static uint8_t msgBuf[sizeof(ikarus_message_t)];
    static uint16_t index = 0;
    static uint16_t expected_length = 0;
    bool in_message = false;

    for (;;) {
        // Warten bis neue Daten da sind
        osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);

        while (uartRxBuffer.get(byte)) {

            // --- 1️⃣ Startbyte finden ---
            if (!in_message) {
                if (byte == 0xAA) {
                    index = 0;
                    msgBuf[index++] = byte;
                    in_message = true;
                    expected_length = 0; // wird gesetzt, sobald header komplett
                }
                continue;
            }

            // --- 2️⃣ Bytes in Buffer schreiben ---
            msgBuf[index++] = byte;

            // --- 3️⃣ Wenn Header komplett ist (3 Bytes), erwartete Länge berechnen ---
            if (index == 3) {
                uint8_t payload_len = msgBuf[2];
                if (payload_len > 100) {
                    // Ungültige Länge -> Reset
                    in_message = false;
                    index = 0;
                    continue;
                }
                expected_length = 3 + 100 + 1; // Header + Payload + CRC  -> payload erwartet immer 64 Bytes
            }

            // --- 4️⃣ Nachricht vollständig? ---
            if (expected_length > 0 && index >= expected_length) {
                if (active_manager != nullptr) {
                    active_manager->processBinaryMessage(msgBuf, expected_length);
                }
                // Reset für das nächste Paket
                in_message = false;
                index = 0;
                expected_length = 0;
            }

            // --- 5️⃣ Overflow-Schutz ---
            if (index >= sizeof(msgBuf)) {
                in_message = false;
                index = 0;
                expected_length = 0;
            }
        }
    }
}

/* === TX Task === */
static void UartTxTask(void *argument)
{
    char msg[TX_BUFFER_SIZE];

    for (;;)
    {
        if (osMessageQueueGet(uartTxQueueHandle, msg, nullptr, osWaitForever) == osOK)
        {
            HAL_UART_Transmit_DMA(&huart7, (uint8_t *)msg, TX_BUFFER_SIZE);
            osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);
        }
    }
}

/* === TX DMA Callback === */
extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART7)
    {
        osThreadFlagsSet(uartTxTaskHandle, 0x01);
    }
}

/* === Public Send Function === */
void UART_Send(const char *msg)
{
    if (uartTxQueueHandle != nullptr)
    {
        strncpy(txBuffer, msg, sizeof(txBuffer));
        txBuffer[sizeof(txBuffer) - 1] = '\0';
        osStatus_t st = osMessageQueuePut(uartTxQueueHandle, txBuffer, 0, 0);
    }
}

void UART_SendBinary(const uint8_t *data, size_t len)
{
    if (uartTxQueueHandle != nullptr && len > 0)
    {
        // Sicherheitscheck: nicht größer als dein txBuffer
        if (len > sizeof(txBuffer)) {
            len = sizeof(txBuffer);
        }

        memcpy(txBuffer, data, len);

        // In Queue stellen (hier wird ein Pointer übertragen oder das ganze Array, je nach Queue-Setup)
        osStatus_t st = osMessageQueuePut(uartTxQueueHandle, txBuffer, 0, 0);
    }
}
