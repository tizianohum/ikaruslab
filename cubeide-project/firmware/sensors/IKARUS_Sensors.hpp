// IKARUS_Sensors.hpp
#pragma once
#include "stm32h7xx_hal.h"
#include "ultrasonic.hpp"
#include "mpu6050.h"
#include "GY271.h"
#include "IMU/bmi160.h"

typedef struct ikarus_sensors_data_t {
    float accX;
    float accY;
    float accZ;
    float gyrX;
    float gyrY;
    float gyrZ;
	bmi160_acc acc;
	bmi160_gyr gyr;
    float magX;
    float magY;
    float magZ;
//    float baro;
} ikarus_sensors_data_t;

class IKARUS_Sensors {
public:
    IKARUS_Sensors();
    void init(ultrasonic_config_t *ultrasonic_config, mpu6050_config_t *imu_config, gy271_config_t *gy_config); // Initialize sensors
    void start(); // Start sensor data acquisition
    void update();

    // Getters for sensor data
    void getAccelerometer(float& x, float& y, float& z) const;
    void getGyroscope(float& x, float& y, float& z) const;
    float getBarometer() const;
    float getUltrasonic(void);
    ikarus_sensors_data_t getData();

    UltrasonicSensor ultrasonicSensor;
    MPU6050 mpu6050;
    GY271 gy271;

private:
    BMI160 imu;
	void _readImu();

    // Sensor data members
    float accX= 0, accY= 0, accZ = 0;
    float gyrX= 0, gyrY=0, gyrZ = 0;
    float magX=0, magY=0, magZ=0;
    float baro = 0;
    float ultrasonic_front_distance = 0;
    ikarus_sensors_data_t _data;

    // Add hardware interface members if needed
};
