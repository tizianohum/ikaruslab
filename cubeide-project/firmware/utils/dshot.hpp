#ifndef DSHOT_HPP
#define DSHOT_HPP

#include <cstdint>

// Größe des DShot PWM-Buffers
constexpr int DSHOT_BUFFER_SIZE = 17;

// Bereitet den PWM-Buffer für DShot vor
void prepare_dshot_buffer(uint16_t value, uint32_t* dshot_pwm_buffer);

#endif // DSHOT_HPP
