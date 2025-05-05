#include "sensors.h"
#include "mux.h"
#include "globals.h"
#include "params.h"

#define SENSOR_RESTART_THRESHOLD 3

bool sensorAvailable[NUM_SENSORS] = {true};
int sensorFailCounter[NUM_SENSORS] = {0};
int sensorConsecutiveFails[NUM_SENSORS] = {0};
bool sensorSoftFailed[NUM_SENSORS] = {false};
bool sensorsUpdated = false;

void setupSensors() {
    for (uint8_t i = 0; i < NUM_SENSORS; i++) {
        sensorFailCounter[i] = 0;
        sensorConsecutiveFails[i] = 0;
        sensorSoftFailed[i] = false;
        restartSensor(i);
    }
}

void restartSensor(uint8_t i) {
    if (i >= NUM_SENSORS) return;

    deselectAllMux();
    delay(2);
    selectMuxChannel(i);
    delay(5);
    delay(10);

    if (xSemaphoreTake(i2cBusyWire0, pdMS_TO_TICKS(30)) == pdTRUE) {
        bool ok = false;

        if (sensor.begin() == 0) {
            delay(5);
            sensor.VL53L4CD_Off();
            delay(2);
            sensor.VL53L4CD_On();
            delay(2);

            if (sensor.VL53L4CD_SensorInit() == 0) {
                sensor.VL53L4CD_SetRangeTiming(200, 0);

                // === Corrected threshold selection ===
                int threshold = 400;
                if (i == 0) { // Front
                    threshold = params.threshold_front;
                } else if (i == 3 || i == 4 || i == 5) { // Back-Left, Back, Back-Right
                    threshold = params.threshold_back;
                } else { // Front-Left, Left, Right, Front-Right
                    threshold = params.threshold_side;
                }

                sensor.VL53L4CD_SetDetectionThresholds(0, threshold, 1);  // interrupt when below high
                ok = true;
            }
        }

        sensorAvailable[i] = ok;
        xSemaphoreGive(i2cBusyWire0);
    } else {
        sensorAvailable[i] = false;
    }
}

void startAllSensors() {
    for (uint8_t i = 0; i < NUM_SENSORS; i++) {
        if (!sensorAvailable[i]) continue;

        deselectAllMux();
        delay(2);
        selectMuxChannel(i);
        delay(5);

        if (xSemaphoreTake(i2cBusyWire0, pdMS_TO_TICKS(10)) == pdTRUE) {
            sensor.VL53L4CD_StartRanging();
            xSemaphoreGive(i2cBusyWire0);
        }
    }
}

void readSensors() {
    static unsigned long lastSensorRead = 0;
    if (millis() - lastSensorRead < sensorInterval) return;
    lastSensorRead = millis();

    sensorsUpdated = false;
    VL53L4CD_Result_t results;
    int failedCount = 0;

    for (uint8_t i = 0; i < NUM_SENSORS; i++) {
        if (!sensorTriggered[i]) continue;
        sensorTriggered[i] = false;

        deselectAllMux();
        delayMicroseconds(100);
        selectMuxChannel(i);
        delayMicroseconds(500);

        bool gotI2C = false;
        for (int retry = 0; retry < 3; retry++) {
            if (xSemaphoreTake(i2cBusyWire0, pdMS_TO_TICKS(10)) == pdTRUE) {
                gotI2C = true;
                break;
            }
            delay(1);
        }

        if (!gotI2C) {
            sensorSoftFailed[i] = true;
        } else {
            uint8_t newDataReady = 0;
            bool gotData = false;

            for (int attempt = 0; attempt < 3; attempt++) {
                if (sensor.VL53L4CD_CheckForDataReady(&newDataReady) == 0 && newDataReady) {
                    gotData = true;
                    break;
                }
                delayMicroseconds(800);
            }

            if (gotData && sensor.VL53L4CD_GetResult(&results) == 0) {
                sensorConsecutiveFails[i] = 0;
                sensor.VL53L4CD_ClearInterrupt();

                uint16_t newDistance = results.distance_mm;
                uint16_t signal = results.signal_rate_kcps;
                uint16_t sigma = results.sigma_mm;

                bool unreliable = (sigma > MAX_RELIABLE_SIGMA) ||
                                  (signal < MIN_SIGNAL_THRESHOLD) ||
                                  (newDistance > MAX_VALID_DISTANCE);

                if (!unreliable) {
                    distances[i] = newDistance;
                    sensorSoftFailed[i] = false;
                    sensorsUpdated = true;
                    xSemaphoreGive(i2cBusyWire0);
                    continue;
                }

                sensorSoftFailed[i] = true;  // Unreliable data
            } else {
                sensorConsecutiveFails[i]++;
                sensorSoftFailed[i] = true;  // Data not ready or read failed
            }

            xSemaphoreGive(i2cBusyWire0);
        }

        // === Corrected fallback logic ===
        int fallbackDistance = params.INVALID_DISTANCE;
        if (i == 0) {
            fallbackDistance = params.threshold_front + 200;
        } else if (i == 3 || i == 4 || i == 5) {
            fallbackDistance = params.threshold_back + 200;
        } else {
            fallbackDistance = params.threshold_side + 200;
        }

        distances[i] = fallbackDistance;
    }

    allSensorsFailed = (failedCount >= NUM_SENSORS);
}
