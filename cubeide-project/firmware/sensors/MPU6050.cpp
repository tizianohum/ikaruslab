/*
 * MPU6050.cpp
 *
 *  Created on: Oct 18, 2025
 *      Author: tizianohumpert
 */

#ifndef MPU6050_CPP_
#define MPU6050_CPP_

#include "MPU6050.h"
#include "math.h"
#include "string.h"

MPU6050::MPU6050(){}


void MPU6050::WriteReg(uint8_t reg, uint8_t value) {
    HAL_I2C_Mem_Write(_hi2c, MPU6050_ADDR, reg, 1, &value, 1, HAL_MAX_DELAY);
}

void MPU6050::WakeUp() {
    // Reset
    WriteReg(MPU6050_REG_PWR_MGMT_1, 0x80);
    HAL_Delay(100);
    // Wake up (set clock source)
    WriteReg(MPU6050_REG_PWR_MGMT_1, 0x01);
    HAL_Delay(50);
}

void MPU6050::SetAccRange(mpu6050_acc_range_t range) {
    _accRange = range;
    WriteReg(MPU6050_REG_ACCEL_CONFIG, range << 3);

    switch (range) {
        case MPU6050_ACC_RANGE_2G:  _acc_sens = 16384.0f; break;
        case MPU6050_ACC_RANGE_4G:  _acc_sens = 8192.0f;  break;
        case MPU6050_ACC_RANGE_8G:  _acc_sens = 4096.0f;  break;
        case MPU6050_ACC_RANGE_16G: _acc_sens = 2048.0f;  break;
    }
}

void MPU6050::SetGyrRange(mpu6050_gyr_range_t range) {
    _gyrRange = range;
    WriteReg(MPU6050_REG_GYRO_CONFIG, range << 3);

    switch (range) {
        case MPU6050_GYR_RANGE_250:  _gyro_sens = 131.0f; break;
        case MPU6050_GYR_RANGE_500:  _gyro_sens = 65.5f;  break;
        case MPU6050_GYR_RANGE_1000: _gyro_sens = 32.8f;  break;
        case MPU6050_GYR_RANGE_2000: _gyro_sens = 16.4f;  break;
    }
}

void MPU6050::init(mpu6050_config_t *config) {
	this->_hi2c = config->hi2c;
    WakeUp();
    // DLPF konfigurieren (44Hz)
    WriteReg(MPU6050_REG_CONFIG, 0x03);
    SetAccRange(config->acc_range);
    SetGyrRange(config->gyr_range);
}

void MPU6050::ReadAll(MPU6050_Raw3Axis& accel, MPU6050_Raw3Axis& gyro) {
    uint8_t data[14];
    if (HAL_I2C_Mem_Read(_hi2c, MPU6050_ADDR, MPU6050_REG_ACCEL_XOUT_H, 1, data, 14, HAL_MAX_DELAY) == HAL_OK) {
        accel.x = (int16_t)(data[0] << 8 | data[1]);
        accel.y = (int16_t)(data[2] << 8 | data[3]);
        accel.z = (int16_t)(data[4] << 8 | data[5]);
        gyro.x  = (int16_t)(data[8] << 8 | data[9]);
        gyro.y  = (int16_t)(data[10] << 8 | data[11]);
        gyro.z  = (int16_t)(data[12] << 8 | data[13]);
    }
}

void MPU6050::Convert(const MPU6050_Raw3Axis& accel_raw, const MPU6050_Raw3Axis& gyro_raw,
                      MPU6050_Scaled3Axis& accel_g, MPU6050_Scaled3Axis& gyro_dps) {
    accel_g.x = (accel_raw.x - _accOffset.x) / _acc_sens *9.81;
    accel_g.y = (accel_raw.y - _accOffset.y) / _acc_sens*9.81;
    accel_g.z = (accel_raw.z - _accOffset.z) / _acc_sens*9.81;

    gyro_dps.x = (gyro_raw.x - _gyroOffset.x) / _gyro_sens;
    gyro_dps.y = (gyro_raw.y - _gyroOffset.y) / _gyro_sens;
    gyro_dps.z = (gyro_raw.z - _gyroOffset.z) / _gyro_sens;
}

void MPU6050::Calibrate(uint16_t samples) {
    // Begrenze Samples, damit Arraygröße nicht gesprengt wird
    const uint16_t MAX_SAMPLES = 50;   // zum Beispiel
    if (samples > MAX_SAMPLES) samples = MAX_SAMPLES;

    // Arrays für Rohdaten (werden lokal angelegt)
    static int16_t acc_samples_x[MAX_SAMPLES];
    static int16_t acc_samples_y[MAX_SAMPLES];
    static int16_t acc_samples_z[MAX_SAMPLES];
    static int16_t gyr_samples_x[MAX_SAMPLES];
    static int16_t gyr_samples_y[MAX_SAMPLES];
    static int16_t gyr_samples_z[MAX_SAMPLES];

    int32_t acc_sum_x = 0, acc_sum_y = 0, acc_sum_z = 0;
    int32_t gyr_sum_x = 0, gyr_sum_y = 0, gyr_sum_z = 0;

    MPU6050_Raw3Axis a, g;

    for (uint16_t i = 0; i < samples; i++) {
    	if (i<2) {continue;} // Erste 2 Messungen verwerfen
        ReadAll(a, g);

        // In Array speichern
        acc_samples_x[i] = a.x;
        acc_samples_y[i] = a.y;
        acc_samples_z[i] = a.z;
        gyr_samples_x[i] = g.x;
        gyr_samples_y[i] = g.y;
        gyr_samples_z[i] = g.z;

        // Summe bilden für Mittelwert
        acc_sum_x += a.x;
        acc_sum_y += a.y;
        acc_sum_z += a.z;
        gyr_sum_x += g.x;
        gyr_sum_y += g.y;
        gyr_sum_z += g.z;

        HAL_Delay(100);
    }
    samples -= 2; // Korrigiere die Anzahl der Samples nach dem Verwerfen
    // Mittelwerte berechnen
    _accOffset.x = (float)acc_sum_x / samples;
    _accOffset.y = (float)acc_sum_y / samples;
    _accOffset.z = (float)acc_sum_z / samples - _acc_sens; // 1g abziehen

    _gyroOffset.x = (float)gyr_sum_x / samples;
    _gyroOffset.y = (float)gyr_sum_y / samples;
    _gyroOffset.z = (float)gyr_sum_z / samples;
}
#endif /* MPU6050_CPP_ */
