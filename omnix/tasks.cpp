#include <Arduino.h>
#include "tasks.h"
#include "sensors.h"
#include "controller.h"
#include "motors.h"
#include "ai.h"
#include "imu.h"
#include "udp_comm.h"
#include "globals.h"
#include "params.h"
#include "interrupts.h"

TaskHandle_t sensorTaskHandle = nullptr;

void sensorTask(void* pvParameters) {
    sensorTaskHandle = xTaskGetCurrentTaskHandle();
    setupSensorInterrupts();

    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        bool anyPending;
        do {
            anyPending = false;
            for (int i = 0; i < NUM_SENSORS; i++) {
                if (sensorEvents[i] > 0) {
                    anyPending = true;
                    break;
                }
            }

            if (anyPending) {
                readSensors();
            }

        } while (anyPending);
    }
}

void motorControlTask(void* pvParameters) {
    TickType_t lastWake = xTaskGetTickCount();
    while (true) {
        if (motorsEnabled && controlMode == MODE_AUTONOMOUS) {
            aiSteering();  // smoothMotorUpdate is now called inside here
        }
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(20));
    }
}

void controllerTask(void* pvParameters) {
    TickType_t lastWake = xTaskGetTickCount();
    while (true) {
        handleController();
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(10));
    }
}

void auxTask(void* pvParameters) {
    TickType_t lastWake = xTaskGetTickCount();
    while (true) {
        loopCounter++;

        handleUdpParams();
        readIMU();

        for (uint8_t i = 0; i < NUM_SENSORS; i++) {
            if (needsRestart[i]) {
                restartSensor(i);
                needsRestart[i] = false;
            }
        }

        sendData();
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(20));
    }
}
