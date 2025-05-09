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

// 🧠 SteeringTask: runs AI steering logic
void steeringTask(void* pvParameters) {
    TickType_t lastWake = xTaskGetTickCount();
    Serial.println("🧠 SteeringTask started");
    while (true) {
        if (motorsEnabled && controlMode == MODE_AUTONOMOUS) {
            aiSteering();  // Decide target motor speeds
        }
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(20));  // Run every 20 ms
    }
}

// 🛞 MotorTask: applies smoothed motor updates
void motorTask(void* pvParameters) {
    TickType_t lastWake = xTaskGetTickCount();
    Serial.println("🛞 MotorTask started");
    while (true) {
        smoothMotorUpdate();  // Apply speed to motors
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(20));
    }
}

// 🎮 ControllerTask: handles gamepad/controller input
void controllerTask(void* pvParameters) {
    TickType_t lastWake = xTaskGetTickCount();
    Serial.println("🎮 ControllerTask started");
    while (true) {
        handleController();
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(10));
    }
}

// 🧰 AuxTask: handles IMU, param updates, restarts, telemetry
void auxTask(void* pvParameters) {
    TickType_t lastWake = xTaskGetTickCount();
    Serial.println("🧰 AuxTask started");
    while (true) {
        loopCounter++;

        handleUdpParams();  // Receive param updates via UDP
        readIMU();          // Read IMU (if enabled)

        for (uint8_t i = 0; i < NUM_SENSORS; i++) {
            if (needsRestart[i]) {
                restartSensor(i);
                needsRestart[i] = false;
            }
        }

        sendData();  // Send telemetry back over UDP
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(20));
    }
}
