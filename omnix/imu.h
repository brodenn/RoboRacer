#ifndef IMU_H
#define IMU_H

void setupIMU();
void readIMU();

extern float imu_ax, imu_ay, imu_az;
extern float imu_gx, imu_gy, imu_gz;
extern float imu_temp;

#endif
