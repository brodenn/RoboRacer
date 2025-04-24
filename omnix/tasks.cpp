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

// Handle for sensor task (used in ISR notifications)
TaskHandle_t sensorTaskHandle = nullptr;

// 🚦 SensorTask: waits for ISR-triggered notification and reads sensors
void sensorTask(void* pvParameters) {
    sensorTaskHandle = xTaskGetCurrentTaskHandle();
    setupSensorInterrupts();
    Serial.println("👀 SensorTask initialized and waiting for interrupts");

    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        bool anyTriggered;
        do {
            anyTriggered = false;
            for (int i = 0; i < NUM_SENSORS; i++) {
                if (sensorTriggered[i]) {
                    anyTriggered = true;
                    break;
                }
            }
            if (anyTriggered) {
                readSensors();  // Only read sensors that triggered
            }
        } while (anyTriggered);
    }
}

// 🎮 ControllerTask: processes gamepad/controller input
void controllerTask(void* pvParameters) {
    TickType_t lastWake = xTaskGetTickCount();
    while (true) {
        handleController();
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(10));  // Run every 10 ms
    }
}

// 🛞 MotorTask: smooth step motor control
void motorTask(void* pvParameters) {
    TickType_t lastWake = xTaskGetTickCount();
    while (true) {
        smoothMotorUpdate();
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(20));  // Run every 20 ms
    }
}

// 🧠 AuxTask: background logic — IMU, AI steering, and data sending
void auxTask(void* pvParameters) {
    TickType_t lastWake = xTaskGetTickCount();
    while (true) {
        loopCounter++;

        handleUdpParams();  // Receive live param updates
        readIMU();

        // Restart sensors if requested
        for (uint8_t i = 0; i < NUM_SENSORS; i++) {
            if (needsRestart[i]) {
                restartSensor(i);
                needsRestart[i] = false;
            }
        }

        if (motorsEnabled && controlMode == MODE_AUTONOMOUS) {
            aiSteering();
        }

        sendData();  // Send UDP telemetry
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(20));  // Run every 20 ms
    }
}
