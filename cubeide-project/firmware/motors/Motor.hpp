// Motor.h
#pragma once
#include "stm32h7xx_hal.h"
#include "utils/dshot.hpp" // Angenommen, prepare_dshot_buffer ist hier drin

typedef struct motor_config_t {
    TIM_HandleTypeDef* htim;
    uint32_t channel;
} motor_config_t;

class Motor {
public:
    Motor();
    void init(motor_config_t config);
    bool start();
    void setSignal(uint16_t value);
    void updatePWM();  // Ruft prepare_dshot_buffer und HAL_TIM_PWM_Start_DMA auf
    void armingSequence();
    TIM_HandleTypeDef* _htim;
    uint32_t _channel;
private:

    uint16_t _signal = {0};
    uint32_t _dshotBuffer[17] = {0};
};
