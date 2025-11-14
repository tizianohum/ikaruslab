/*
 * MPU6050.hpp
 *
 *  Created on: Oct 18, 2025
 *      Author: tizianohumpert
 */

#ifndef MPU6050_H
#define MPU6050_H

#include "stm32h7xx_hal.h" // oder dein passender HAL Header
#include "stm32h7xx_hal_i2c.h"
#include <cstdint>

// I2C-Adresse (8-bit, für STM32 HAL)
#define MPU6050_ADDR           (0x68 << 1)

// MPU6050 Register
#define MPU6050_REG_PWR_MGMT_1     0x6B
#define MPU6050_REG_ACCEL_CONFIG   0x1C
#define MPU6050_REG_GYRO_CONFIG    0x1B
#define MPU6050_REG_ACCEL_XOUT_H   0x3B
#define MPU6050_REG_CONFIG         0x1A



typedef enum {
    MPU6050_ACC_RANGE_2G = 0,
    MPU6050_ACC_RANGE_4G,
    MPU6050_ACC_RANGE_8G,
    MPU6050_ACC_RANGE_16G
} mpu6050_acc_range_t;

typedef enum {
    MPU6050_GYR_RANGE_250 = 0,
    MPU6050_GYR_RANGE_500,
    MPU6050_GYR_RANGE_1000,
    MPU6050_GYR_RANGE_2000
} mpu6050_gyr_range_t;


typedef struct mpu6050_config_t{
	uint8_t address;
	I2C_HandleTypeDef* hi2c;
	mpu6050_acc_range_t acc_range;
	mpu6050_gyr_range_t gyr_range;
} mpu6050_config_t;

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} MPU6050_Raw3Axis;

typedef struct {
    float x;
    float y;
    float z;
} MPU6050_Scaled3Axis;

class MPU6050 {
public:
    MPU6050();

    void init(mpu6050_config_t *config);

    void ReadAll(MPU6050_Raw3Axis& accel, MPU6050_Raw3Axis& gyro);
    void Convert(const MPU6050_Raw3Axis& accel_raw, const MPU6050_Raw3Axis& gyro_raw,
                 MPU6050_Scaled3Axis& accel_g, MPU6050_Scaled3Axis& gyro_dps);

    void Calibrate(uint16_t samples = 500);

private:
    I2C_HandleTypeDef* _hi2c;
    mpu6050_acc_range_t _accRange;
    mpu6050_gyr_range_t _gyrRange;

    mpu6050_config_t _config;

    float _acc_sens;
    float _gyro_sens;

    MPU6050_Raw3Axis _accOffset = { -52, 9, -468 };
    MPU6050_Raw3Axis _gyroOffset = { -179, 28, 57 };

    void WriteReg(uint8_t reg, uint8_t value);
    void WakeUp();
    void SetAccRange(mpu6050_acc_range_t range);
    void SetGyrRange(mpu6050_gyr_range_t range);

};

#endif // MPU6050_H
