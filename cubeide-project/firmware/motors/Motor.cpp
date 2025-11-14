// Motor.cpp
#include "Motor.hpp"
#include "main.h"

Motor::Motor() {}

void Motor::init(motor_config_t config) {
	_htim = config.htim;
	_channel = config.channel;
	_signal = 0;
}

bool Motor::start() {
	//ToDO
	return true;
}

void Motor::setSignal(uint16_t value) {
    _signal = value;
}

void Motor::updatePWM() {
    prepare_dshot_buffer(_signal, _dshotBuffer);
    if (HAL_TIM_PWM_Start_DMA(_htim, _channel, _dshotBuffer, 17)) {
        Error_Handler();
    }
}

void armingSequence(){
	//500ms nullen schicken
}
