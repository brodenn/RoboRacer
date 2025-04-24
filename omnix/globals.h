#ifndef GLOBALS_H 
#define GLOBALS_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include <vl53l4cd_class.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Adafruit_INA219.h>
#include <Adafruit_LSM6DSOX.h>
#include "params.h"
#include "freertos/semphr.h"

// ----------------------
// Constants & MUX
// ----------------------

#define NUM_SENSORS 8
#define MUX1_ADDR 0x71
#define MUX2_ADDR 0x70

// ----------------------
// Sensor Interrupts
// ----------------------

extern const uint8_t SENSOR_GPIO[NUM_SENSORS];         // GPIOs for INT pins
extern volatile bool sensorTriggered[NUM_SENSORS];     // Flags set via ISR

// ----------------------
// WiFi & UDP
// ----------------------

extern const char* ssid;
extern const char* password;
extern const char* laptop_ip;
extern const int udp_port;
extern WiFiUDP udp;
extern SemaphoreHandle_t wifiSemaphore;

// ----------------------
// I2C Mutexes
// ----------------------

extern SemaphoreHandle_t i2cBusyWire0;  // For sensors, MUX, INA
extern SemaphoreHandle_t i2cBusyWire1;  // For motors

// ----------------------
// Motor Control
// ----------------------

extern int target_m1, target_m2, target_m3, target_m4;
extern int current_m1, current_m2, current_m3, current_m4;
extern const int MAX_STEP;
extern const int motorUpdateInterval;

extern Adafruit_MotorShield AFMS_60;
extern Adafruit_MotorShield AFMS_61;

extern Adafruit_DCMotor* motor1;
extern Adafruit_DCMotor* motor2;
extern Adafruit_DCMotor* motor3;
extern Adafruit_DCMotor* motor4;

extern bool motorsEnabled;

// ----------------------
// Sensor Data (VL53L4CD)
// ----------------------

extern VL53L4CD sensor;
extern uint16_t distances[NUM_SENSORS];
extern bool allSensorsFailed;
extern int sensorFailureCount[NUM_SENSORS];
extern bool needsRestart[NUM_SENSORS];
extern const int SENSOR_FAILURE_THRESHOLD;

// ----------------------
// Sensor Filters
// ----------------------

extern int MAX_RELIABLE_SIGMA;
extern int MIN_SIGNAL_THRESHOLD;
extern int MAX_VALID_DISTANCE;
extern int INVALID_DISTANCE;

// ----------------------
// Reverse Logic
// ----------------------

extern bool reversing;
extern unsigned long reverseStart;
extern unsigned long closeFrontStart;
extern const int stuckDuration;
extern const int reverseTime;

// ----------------------
// Zigzag Suppression
// ----------------------

extern int headingHoldDir;
extern unsigned long headingLockStart;
extern const int headingLockDuration;

// ----------------------
// Current Sensing (INA219)
// ----------------------

extern Adafruit_INA219 ina60;
extern Adafruit_INA219 ina61;

// ----------------------
// Timing Control
// ----------------------

extern unsigned long lastSensorRead;
extern unsigned long lastDataSend;
extern const int sensorInterval;
extern const int sendInterval;

// ----------------------
// Operating Mode
// ----------------------

enum ControlMode { MODE_AUTONOMOUS, MODE_CONTROLLER };
extern ControlMode controlMode;
extern SteeringParams params;

// ----------------------
// IMU (LSM6DSOX)
// ----------------------

extern Adafruit_LSM6DSOX imu;
extern float imu_ax, imu_ay, imu_az;
extern float imu_gx, imu_gy, imu_gz;
extern float imu_temp;

// ----------------------
// Misc Counters
// ----------------------

extern unsigned long loopCounter;

#endif
