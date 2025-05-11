#pragma once

#include <Arduino.h>

extern TaskHandle_t sensorTaskHandle;

void sensorTask(void* pvParameters);
void motorControlTask(void* pvParameters);  // Combined steering + motor logic
void controllerTask(void* pvParameters);
void auxTask(void* pvParameters);
