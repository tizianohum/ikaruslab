/*
 * GY271.h
 *
 *  Created on: Oct 25, 2025
 *      Author: tizianohumpert
 */

#ifndef SENSORS_GY271_H_
#define SENSORS_GY271_H_

#ifndef GY271_H
#define GY271_H

#include "stm32h7xx_hal.h"  // ggf. anpassen
#include "stm32h7xx_hal_i2c.h"
#include <stdint.h>

#define GY271_ADDR (0x2C << 1)  // 8-bit-Adresse für STM32-HAL

// Registermap laut QMC5883P
#define REG_CHIP_ID      0x00
#define REG_XOUT_LSB     0x01
#define REG_XOUT_MSB     0x02
#define REG_YOUT_LSB     0x03
#define REG_YOUT_MSB     0x04
#define REG_ZOUT_LSB     0x05
#define REG_ZOUT_MSB     0x06
#define REG_STATUS       0x09
#define REG_CONTROL1     0x0A
#define REG_CONTROL2     0x0B
// Kalibrierungsparameter
typedef struct {
    float offsetX;
    float offsetY;
    float offsetZ;
    float scaleX;
    float scaleY;
    float scaleZ;
    float avgScale;
} gy271_calibration_t;
// Werte für CONTROL1
typedef enum {
    QMC_MODE_SUSPEND   = 0x00,
    QMC_MODE_NORMAL    = 0x01,
    QMC_MODE_SINGLE    = 0x02,
    QMC_MODE_CONTINUOUS= 0x03
} qmc_mode_t;

typedef enum {
    QMC_ODR_10HZ  = 0x00,
    QMC_ODR_50HZ  = 0x01,
    QMC_ODR_100HZ = 0x02,
    QMC_ODR_200HZ = 0x03
} qmc_odr_t;

typedef enum {
    QMC_OSR_8 = 0x00,
    QMC_OSR_4 = 0x01,
    QMC_OSR_2 = 0x02,
    QMC_OSR_1 = 0x03
} qmc_osr_t;

typedef enum {
    QMC_DSR_1 = 0x00,
    QMC_DSR_2 = 0x01,
    QMC_DSR_4 = 0x02,
    QMC_DSR_8 = 0x03
} qmc_dsr_t;

// CONTROL2
typedef enum {
    QMC_RANGE_30G = 0x00,
    QMC_RANGE_12G = 0x01,
    QMC_RANGE_8G  = 0x02,
    QMC_RANGE_2G  = 0x03
} qmc_range_t;


typedef enum {
    QMC_SETRESET_ON      = 0x00,
    QMC_SETRESET_SETONLY = 0x01,
    QMC_SETRESET_OFF     = 0x02
} qmc_setreset_t;

// STM32 Config Struct
typedef struct {
    uint8_t address;
    I2C_HandleTypeDef* hi2c;
} gy271_config_t;

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} gy271_Raw3Axis;

typedef struct {
    float x;
    float y;
    float z;
} gy271_mag;

class GY271 {
public:
    GY271();
    void init(gy271_config_t *config);
    void read();
    float getScaleForRange(qmc_range_t range);
    void calibrate(uint16_t samples, uint16_t delay_ms);
    gy271_mag getMag() { return _mag; }

    gy271_calibration_t cal = {
        .offsetX   =  0.0244333334f,
        .offsetY   = -0.0130333342f,
        .offsetZ   = -0.00800000038f,
        .scaleX    =  0.0215666667f,
        .scaleY    =  0.0210999995f,
        .scaleZ    =  0.0268666667f,
        .avgScale  =  0.0231777783f
    };
private:
    gy271_mag _mag;
    float heading = 0.0f;
    float lsb_per_gauss= 0;
    gy271_config_t config;
    HAL_StatusTypeDef write8(uint8_t reg, uint8_t value);
    HAL_StatusTypeDef readBytes(uint8_t reg, uint8_t* buffer, uint8_t length);
};



#endif



#endif /* SENSORS_GY271_H_ */
