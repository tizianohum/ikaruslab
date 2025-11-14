#include "DShot.hpp"

// Hier musst du die Werte definieren oder aus einer Konfigurationsdatei einbinden
constexpr uint32_t PERIOD = 275;             // Beispiel: Timer-Periode
constexpr uint32_t DSHOT_BIT_1_DUTY = 80;     // Prozent
constexpr uint32_t DSHOT_BIT_0_DUTY = 40;     // Prozent

void prepare_dshot_buffer(uint16_t value, uint32_t* dshot_pwm_buffer) {
    // 1. Limit value to 11 bits
    value &= 0x7FF;

    // 2. Build 12-bit payload (11 bits value + 1 bit telemetry)
    uint16_t frame = (value << 1) | 0; // Telemetry bit = 0

    // 3. Calculate 4-bit CRC (checksum)
    uint8_t csum = 0;
    uint16_t csum_data = frame;
    for (int i = 0; i < 3; i++) {
        csum ^= (csum_data & 0xF); // XOR lower 4 bits
        csum_data >>= 4;
    }
    csum &= 0xF;

    // 4. Compose final 16-bit DSHOT frame
    frame = (frame << 4) | csum;

    // 5. Encode to PWM buffer (DSHOT300: 75% for '1', 37.5% for '0')
    for (int i = 0; i < 16; i++) {
        if (frame & (1 << (15 - i))) {
            dshot_pwm_buffer[i] = (PERIOD * DSHOT_BIT_1_DUTY) / 100;
        } else {
            dshot_pwm_buffer[i] = (PERIOD * DSHOT_BIT_0_DUTY) / 100;
        }
    }
    dshot_pwm_buffer[16] = 0; // Reset / End-Bit
}
