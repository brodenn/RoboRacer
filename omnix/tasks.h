#pragma once
#include <Arduino.h>

// === Task Handles ===
extern TaskHandle_t sensorTaskHandle;  // Used by ISR to notify the sensor task

// === Task Function Declarations ===
void sensorTask(void* pvParameters);      // Handles VL53L4CD sensor reading
void controllerTask(void* pvParameters);  // Handles gamepad input
void motorTask(void* pvParameters);       // Handles motor updates
void auxTask(void* pvParameters);         // Handles AI, IMU, UDP, deferred restarts
