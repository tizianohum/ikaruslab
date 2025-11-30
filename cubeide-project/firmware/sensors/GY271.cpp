/*
 * GY271.cpp
 *
 *  Created on: Oct 25, 2025
 *      Author: tizianohumpert
 */

#include "GY271.h"
#include "cmsis_os.h"
#include "math.h"
#include "firmware.hpp"

extern IKARUS_Firmware ikarus_firmware;

GY271::GY271() {
}

void GY271::init(gy271_config_t *cfg) {
	this->config = *cfg;

	// Soft Reset (Control2 Bit7 = 1)
	write8(REG_CONTROL2, 0x80);
	osDelay(50);

	// Prüfe Chip-ID (sollte 0x80 sein)
	uint8_t chip_id = 0;
	if (readBytes(REG_CHIP_ID, &chip_id, 1) == HAL_OK) {
		if (chip_id != 0x80) {
			// Kein echter QMC5883P erkannt
			return;
		}
	}

	// CONTROL2:
	// Bits [3:2] = Range (8G)
	// Bits [1:0] = Set/Reset ON
	uint8_t ctrl2 = (QMC_RANGE_8G << 2) | QMC_SETRESET_ON;
	write8(REG_CONTROL2, ctrl2);

	// CONTROL1:
	// Bits [7:6] = Downsample Ratio (4)
	// Bits [5:4] = OSR (4)
	// Bits [3:2] = ODR (100Hz)
	// Bits [1:0] = Mode (Continuous)
	uint8_t ctrl1 = (QMC_DSR_4 << 6) | (QMC_OSR_4 << 4) | (QMC_ODR_100HZ << 2)
			| QMC_MODE_CONTINUOUS;
	write8(REG_CONTROL1, ctrl1);

	osDelay(50);

	this->lsb_per_gauss = getScaleForRange(QMC_RANGE_2G);
}

void GY271::read() {
	uint8_t buffer[6];
	if (readBytes(REG_XOUT_LSB, buffer, 6) != HAL_OK) {
		return;
	}

	int16_t rawX = (int16_t) ((buffer[1] << 8) | buffer[0]);
	int16_t rawY = (int16_t) ((buffer[3] << 8) | buffer[2]);
	int16_t rawZ = (int16_t) ((buffer[5] << 8) | buffer[4]);

	float x = rawX / lsb_per_gauss;
	float y = rawY / lsb_per_gauss;
	float z = rawZ / lsb_per_gauss;

	// Kalibrierung anwenden (falls vorhanden)
	if (cal.scaleX != 0 && cal.scaleY != 0 && cal.scaleZ != 0) {
		x = (x - cal.offsetX) / cal.scaleX * cal.avgScale;
		y = (y - cal.offsetY) / cal.scaleY * cal.avgScale;
		z = (z - cal.offsetZ) / cal.scaleZ * cal.avgScale;
	}

	_mag.x = x;
	_mag.y = y;
	_mag.z = z;

    // Arctan2 gibt den Winkel im Bogenmaß zurück (-π .. +π)
    heading = atan2(_mag.y, _mag.x);

    // In Grad umrechnen
    heading = heading * 180.0f / M_PI;

    // Negative Winkel auf 0–360° verschieben
    if (heading < 0)
        heading += 360.0f;
}

float GY271::getScaleForRange(qmc_range_t range) {
    switch (range) {
        case QMC_RANGE_2G:  return 15000.0f;
        case QMC_RANGE_8G:  return 3750.0f;
        case QMC_RANGE_12G: return 2500.0f;
        case QMC_RANGE_30G: return 1000.0f;
        default:            return 15000.0f;
    }
}

HAL_StatusTypeDef GY271::write8(uint8_t reg, uint8_t value) {
	return HAL_I2C_Mem_Write(config.hi2c, GY271_ADDR, reg, I2C_MEMADD_SIZE_8BIT,
			&value, 1, HAL_MAX_DELAY);
}

HAL_StatusTypeDef GY271::readBytes(uint8_t reg, uint8_t *buffer,
		uint8_t length) {
	return HAL_I2C_Mem_Read(config.hi2c, GY271_ADDR, reg, I2C_MEMADD_SIZE_8BIT,
			buffer, length, HAL_MAX_DELAY);
}

void GY271::calibrate(uint16_t samples, uint16_t delay_ms) {
    float minX =  1e6, minY =  1e6, minZ =  1e6;
    float maxX = -1e6, maxY = -1e6, maxZ = -1e6;

    ikarus_firmware.comm.send("Starting magnetometer calibration. Please rotate the sensor in all directions.");
osDelay(1000);
    for (uint16_t i = 0; i < samples; i++) {
        read();

        if (_mag.x < minX) minX = _mag.x;
        if (_mag.y < minY) minY = _mag.y;
        if (_mag.z < minZ) minZ = _mag.z;

        if (_mag.x > maxX) maxX = _mag.x;
        if (_mag.y > maxY) maxY = _mag.y;
        if (_mag.z > maxZ) maxZ = _mag.z;

        osDelay(delay_ms);
    }

    cal.offsetX = (maxX + minX) / 2.0f;
    cal.offsetY = (maxY + minY) / 2.0f;
    cal.offsetZ = (maxZ + minZ) / 2.0f;

    cal.scaleX = (maxX - minX) / 2.0f;
    cal.scaleY = (maxY - minY) / 2.0f;
    cal.scaleZ = (maxZ - minZ) / 2.0f;
    cal.avgScale = (cal.scaleX + cal.scaleY + cal.scaleZ) / 3.0f;

    ikarus_firmware.comm.send("Magnetometer calibration completed.");

}
