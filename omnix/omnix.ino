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

    // === I2C Setup ===
    Wire.begin();                     // Wire0 → Sensors, MUX, INA219
    Wire.setClock(400000);           // 400kHz Fast Mode
    Wire1.begin(15, 14, 400000);     // Wire1 → Motor shields

    // === Core Init ===
    setupWiFi();                     // AP/STA WiFi mode
    setupMux();                      // MUX chip

    setupParams();                   // Load thresholds & filtering
    applyThresholdsToSensors();      // Push thresholds to sensors
    setupSensors();                  // Init VL53L4CDs with fallback
    setupMotors();                   // Adafruit Motor Shield
    checkMotorShields(true);         // Optional test

    controllerSetup();              // Xbox/PS4 controller via BLE
    setupIMU();                     // LSM6DSOX (optional)

    // === INA219 Power Monitoring ===
    deselectAllMux();
    selectMuxINA60(); delay(2); ina60.begin();
    deselectAllMux();
    selectMuxINA61(); delay(2); ina61.begin();

    // === Attach Sensor Interrupts ===
    setupSensorInterrupts();

    // === Start Sensors ===
    startAllSensors();

    // === Prioritized Task Launch ===
    xTaskCreatePinnedToCore(sensorTask,     "SensorTask",     8192, nullptr, 4, &sensorTaskHandle, 0);
    xTaskCreatePinnedToCore(steeringTask,   "SteeringTask",   8192, nullptr, 3, nullptr, 1);
    xTaskCreatePinnedToCore(motorTask,      "MotorTask",      8192, nullptr, 2, nullptr, 1);
    xTaskCreate(controllerTask, "ControllerTask", 8192, nullptr, 1, nullptr);
    xTaskCreate(auxTask,        "AuxTask",        8192, nullptr, 1, nullptr);


    Serial.println("🚀 Setup complete — system is now running!");
}

void loop() {
    // Everything runs in tasks
}
