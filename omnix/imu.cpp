#include "imu.h"
#include <Adafruit_LSM6DSOX.h>
#include "globals.h"

Adafruit_LSM6DSOX sox;

// Optional: Control IMU read rate if needed
const unsigned long imuInterval = 5;  // in ms
static unsigned long lastIMURead = 0;

void setupIMU() {
    if (!sox.begin_I2C()) {
        // IMU not found — silent fail
        return;
    }

    sox.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);
    sox.setGyroRange(LSM6DS_GYRO_RANGE_1000_DPS);
    sox.setAccelDataRate(LSM6DS_RATE_104_HZ);
    sox.setGyroDataRate(LSM6DS_RATE_104_HZ);
}

void readIMU() {
    if (millis() - lastIMURead < imuInterval) return;
    lastIMURead = millis();

    if (xSemaphoreTake(i2cBusyWire0, pdMS_TO_TICKS(10)) != pdTRUE) {
        return;
    }

    sensors_event_t accel, gyro, temp;
    sox.getEvent(&accel, &gyro, &temp);

    imu_ax = accel.acceleration.x;
    imu_ay = accel.acceleration.y;
    imu_az = accel.acceleration.z;
    imu_gx = gyro.gyro.x;
    imu_gy = gyro.gyro.y;
    imu_gz = gyro.gyro.z;
    imu_temp = temp.temperature;

    xSemaphoreGive(i2cBusyWire0);
}
