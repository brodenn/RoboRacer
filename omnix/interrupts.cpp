#include "interrupts.h"
#include "globals.h"
#include "tasks.h"
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void IRAM_ATTR notifySensorTask(uint8_t index) {
    sensorTriggered[index] = true;

    if (sensorTaskHandle != nullptr) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        vTaskNotifyGiveFromISR(sensorTaskHandle, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

// ISR function array
void IRAM_ATTR sensor_ISR_0() { notifySensorTask(0); }
void IRAM_ATTR sensor_ISR_1() { notifySensorTask(1); }
void IRAM_ATTR sensor_ISR_2() { notifySensorTask(2); }
void IRAM_ATTR sensor_ISR_3() { notifySensorTask(3); }
void IRAM_ATTR sensor_ISR_4() { notifySensorTask(4); }
void IRAM_ATTR sensor_ISR_5() { notifySensorTask(5); }
void IRAM_ATTR sensor_ISR_6() { notifySensorTask(6); }
void IRAM_ATTR sensor_ISR_7() { notifySensorTask(7); }

void (*sensorISRs[NUM_SENSORS])() = {
    sensor_ISR_0,
    sensor_ISR_1,
    sensor_ISR_2,
    sensor_ISR_3,
    sensor_ISR_4,
    sensor_ISR_5,
    sensor_ISR_6,
    sensor_ISR_7
};

void setupSensorInterrupts() {
    Serial.println("📡 Configuring sensor GPIO interrupts...");

    for (int i = 0; i < NUM_SENSORS; i++) {
        pinMode(SENSOR_GPIO[i], INPUT_PULLUP);
        attachInterrupt(SENSOR_GPIO[i], sensorISRs[i], FALLING);
    }

    Serial.println("✅ Sensor interrupts attached");
}
