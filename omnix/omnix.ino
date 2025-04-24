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

bool areAllMotorsBusy() {
    return abs(target_m1) > 100 &&
           abs(target_m2) > 100 &&
           abs(target_m3) > 100 &&
           abs(target_m4) > 100;
}

void setup() {
    Serial.begin(115200);

    i2cBusyWire0 = xSemaphoreCreateMutex();
    i2cBusyWire1 = xSemaphoreCreateMutex();

    Wire.begin();
    Wire.setClock(400000);
    Wire1.begin(15, 14, 400000); // Motor shields on Wire1

    setupWiFi();
    setupMux();
    setupSensors();
    setupMotors();
    checkMotorShields(true);
    setupParams();
    controllerSetup();
    setupIMU();

    deselectAllMux();
    selectMuxINA60(); delay(2);
    ina60.begin();
    deselectAllMux();
    selectMuxINA61(); delay(2);
    ina61.begin();

    // Interrupts before tasks
    setupSensorInterrupts();

    // Start ranging after init
    startAllSensors();

    // Start tasks
    xTaskCreate(sensorTask,     "SensorTask",     8192, nullptr, 3, &sensorTaskHandle);
    xTaskCreate(controllerTask, "ControllerTask", 8192, nullptr, 1, nullptr);
    xTaskCreate(motorTask,      "MotorTask",      8192, nullptr, 1, nullptr);
    xTaskCreate(auxTask,        "AuxTask",        8192, nullptr, 1, nullptr);

    Serial.println("🚀 Setup complete — system is now running!");
}

void loop() {
    // Everything is handled by FreeRTOS
}
