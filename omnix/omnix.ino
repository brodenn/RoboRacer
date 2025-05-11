#include <Arduino.h>
#include "globals.h"
#include "wifi_setup.h"
#include "mux.h"
#include "xshut.h"
#include "sensors.h"
#include "motors.h"
#include "ai.h"
#include "udp_comm.h"
#include "params.h"
#include "controller.h"
#include "imu.h"
#include "tasks.h"
#include "interrupts.h"

void setup() {
    Serial.begin(115200);
    Serial.println("🛠️ Booting system...");

    // === Create I2C Semaphores ===
    i2cBusyWire0 = xSemaphoreCreateMutex();
    i2cBusyWire1 = xSemaphoreCreateMutex();

    // === I2C Bus Setup ===
    Wire.begin();
    Wire.setClock(400000);
    Wire1.begin(15, 14, 400000);  // Wire1: motor shields (SDA=15, SCL=14)

    // === Core Initialization ===
    setupWiFi();
    setupMux();
    setupParams();
    applyThresholdsToSensors();
    setupSensors();
    setupMotors();
    checkMotorShields(false);  // Skip motor test

    controllerSetup();
    setupIMU();

    // === INA219 Current Sensors ===
    deselectAllMux();
    selectMuxINA60(); delay(2); ina60.begin();
    deselectAllMux();
    selectMuxINA61(); delay(2); ina61.begin();

    setupSensorInterrupts();
    startAllSensors();
    

    // === Launch Tasks ===
    xTaskCreatePinnedToCore(sensorTask,       "SensorTask",       8192, nullptr, 4, &sensorTaskHandle, 0);
    xTaskCreatePinnedToCore(motorControlTask, "MotorControlTask", 8192, nullptr, 3, nullptr, 1); // Combined AI + Motor
    xTaskCreatePinnedToCore(controllerTask,   "ControllerTask",   8192, nullptr, 1, nullptr, 1);
    xTaskCreatePinnedToCore(auxTask,          "AuxTask",          8192, nullptr, 1, nullptr, 1);

    Serial.println("🚀 Setup complete — system is now running!");
}

void loop() {
    // All logic handled in FreeRTOS tasks
}
