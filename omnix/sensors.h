#ifndef SENSORS_H
#define SENSORS_H

#include <vl53l4cd_class.h>
#include <Wire.h>

// --- Constants ---
#define NUM_SENSORS 8

// --- Sensor Driver Instance ---
extern VL53L4CD sensor;  // Shared across all sensors via MUX

// --- Distance Results ---
extern uint16_t distances[NUM_SENSORS];
extern bool allSensorsFailed;
extern bool sensorsUpdated;

// --- Diagnostic Flags ---
extern bool sensorAvailable[NUM_SENSORS];
extern int sensorFailCounter[NUM_SENSORS];
extern int sensorConsecutiveFails[NUM_SENSORS];
extern bool sensorSoftFailed[NUM_SENSORS];

// --- Filtering Parameters ---
extern int MAX_RELIABLE_SIGMA;
extern int MIN_SIGNAL_THRESHOLD;
extern int MAX_VALID_DISTANCE;
extern int INVALID_DISTANCE;

// --- Core Functions ---
void setupSensors();
void readSensors();
void restartSensor(uint8_t i);
void startAllSensors();
void applyThresholdsToSensors();  // ✅ NEW: Apply default detection thresholds

#endif
