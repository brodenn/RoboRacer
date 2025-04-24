#ifndef XSHUT_H
#define XSHUT_H

#include <Arduino.h>

#define NUM_SENSORS 8

// --- Sensor indexes ---
#define SENSOR_FRONT        0
#define SENSOR_FRONT_RIGHT  1
#define SENSOR_RIGHT        2
#define SENSOR_RIGHT_BACK   3
#define SENSOR_BACK         4
#define SENSOR_BACK_LEFT    5
#define SENSOR_LEFT         6
#define SENSOR_FRONT_LEFT   7

// --- Actual GPIO pin mapping for sensors ---
extern const uint8_t SENSOR_GPIO[NUM_SENSORS];  // Use extern to avoid multiple definitions

// --- GPIO pins for interrupt ---
extern const uint8_t INTERRUPT_GPIO[NUM_SENSORS];

// --- Control Functions ---
void setupSensors();
void sensorPowerOff(uint8_t sensorIndex);
void sensorPowerOn(uint8_t sensorIndex);
void sensorRestart(uint8_t sensorIndex); // New function for restarting a sensor

void configureSensorInterrupt(uint8_t sensorIndex);
void configureSensorThreshold(uint8_t sensorIndex, uint16_t lowThreshold, uint16_t highThreshold);
void dataReadyISR(); // Interrupt Service Routine to handle data ready
#endif
