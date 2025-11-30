// IKARUS_Sensors.cpp
#include "IKARUS_Sensors.hpp"
#include "spi.h"
#include "string.h"

extern SPI_HandleTypeDef hspi2;
IKARUS_Sensors::IKARUS_Sensors() {
}

void IKARUS_Sensors::init(ultrasonic_config_t *ultrasonic_config,mpu6050_config_t *imu_config, gy271_config_t *gy_config) {
	// Initialize sensors (e.g., configure I2C, SPI, GPIOs)
	this -> ultrasonicSensor.init(ultrasonic_config);
	//this -> mpu6050.init(imu_config);
	this->gy271.init(gy_config);

	// Initialize the IMU
		bmi160_gyr_config_t gyr_config;
		bmi160_acc_config_t acc_config;
		bmi160_config_t imu_160_config = { .hspi = &hspi2, .CS_GPIOx =
		CS_IMU_GPIO_Port, .CS_GPIO_Pin =
		CS_IMU_Pin, .gyr = gyr_config, .acc = acc_config };

		uint8_t success = imu.init(imu_160_config);
		imu.fastOffsetCalibration();
}

void IKARUS_Sensors::start() {
	// Start sensor data acquisition (e.g., start timers, interrupts)
	this -> ultrasonicSensor.start();
	//this -> mpu6050.Calibrate(50);
	//this->gy271.calibrate(1000, 50);


}

void IKARUS_Sensors::update() {
//    MPU6050_Raw3Axis rawAcc, rawGyr;
//    MPU6050_Scaled3Axis acc, gyr;
//
//    this->mpu6050.ReadAll(rawAcc, rawGyr);
//    this->mpu6050.Convert(rawAcc, rawGyr, acc, gyr);
//
//    this->accX = acc.x;
//    this->accY = acc.y;
//    this->accZ = acc.z;
//    this->gyrX = gyr.x;
//    this->gyrY = gyr.y;
//    this->gyrZ = gyr.z;
	this->_readImu();

    this->gy271.read();
}

ikarus_sensors_data_t IKARUS_Sensors::getData() {
	_data.acc = this->imu.acc;
	_data.gyr = this->imu.gyr;
	_data.mag = this->gy271.getMag();
	_data.ultrasonic_front_distance = this->ultrasonicSensor.getDistance();
	//data.baro = this->baro; // Falls Barometerdaten vorhanden sind
	return _data;
}

void IKARUS_Sensors::getAccelerometer(float &x, float &y, float &z) const {
	x = accX;
	y = accY;
	z = accZ;
}

void IKARUS_Sensors::getGyroscope(float &x, float &y, float &z) const {
	x = gyrX;
	y = gyrY;
	z = gyrZ;
}

float IKARUS_Sensors::getBarometer() const {
	return baro;
}

float IKARUS_Sensors::getUltrasonic(void) {
	return this->ultrasonic_front_distance;
}

void IKARUS_Sensors::_readImu() {
	this->imu.update();
	memcpy(&this->_data.acc, &this->imu.acc, sizeof(this->_data.acc));
	memcpy(&this->_data.gyr, &this->imu.gyr, sizeof(this->_data.gyr));
}
