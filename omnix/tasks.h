#pragma once

#include <Arduino.h>

extern TaskHandle_t sensorTaskHandle;

void sensorTask(void* pvParameters);
void steeringTask(void* pvParameters);
void motorTask(void* pvParameters);
void controllerTask(void* pvParameters);
void auxTask(void* pvParameters);
