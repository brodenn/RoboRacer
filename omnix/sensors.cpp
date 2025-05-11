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
unsigned long lastSensorTriggerTime[NUM_SENSORS] = {0};

void setupSensors() {
    for (uint8_t i = 0; i < NUM_SENSORS; i++) {
        sensorFailCounter[i] = 0;
        sensorConsecutiveFails[i] = 0;
        sensorSoftFailed[i] = false;
        lastSensorTriggerTime[i] = millis();

        distances[i] = (i == 0) ? params.threshold_front + 400 :
                       (i == 3 || i == 4 || i == 5) ? params.threshold_back + 400 :
                       params.threshold_side + 400;

        restartSensor(i);
    }
}

void restartSensor(uint8_t i) {
    if (i >= NUM_SENSORS) return;

    deselectAllMux();
    delay(2);
    selectMuxChannel(i);
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

                int threshold = (i == 0) ? params.threshold_front :
                                (i == 3 || i == 4 || i == 5) ? params.threshold_back :
                                params.threshold_side;

                sensor.VL53L4CD_SetDetectionThresholds(0, threshold, params.windowMode);
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

    for (uint8_t i = 0; i < NUM_SENSORS; i++) {
        bool handled = false;

        while (sensorEvents[i] > 0) {
            sensorEvents[i]--;
            lastSensorTriggerTime[i] = millis();

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
                continue;
            }

            uint8_t newDataReady = 0;
            bool gotData = false;
            bool resultOk = false;

            for (int attempt = 0; attempt < 3; attempt++) {
                if (sensor.VL53L4CD_CheckForDataReady(&newDataReady) == 0 && newDataReady) {
                    gotData = true;
                    break;
                }
                delayMicroseconds(800);
            }

            if (gotData) {
                resultOk = (sensor.VL53L4CD_GetResult(&results) == 0);
                sensor.VL53L4CD_ClearInterrupt();

                if (resultOk) {
                    uint16_t newDistance = results.distance_mm;
                    uint16_t signal = results.signal_rate_kcps;
                    uint16_t sigma = results.sigma_mm;

                    bool reliable = (sigma <= MAX_RELIABLE_SIGMA) &&
                                    (signal >= MIN_SIGNAL_THRESHOLD) &&
                                    (newDistance <= MAX_VALID_DISTANCE) &&
                                    (newDistance > 0);

                    if (reliable) {
                        distances[i] = newDistance;
                        sensorSoftFailed[i] = false;
                        sensorsUpdated = true;
                        handled = true;
                    } else {
                        sensorSoftFailed[i] = true;
                    }
                } else {
                    sensorSoftFailed[i] = true;
                    sensorConsecutiveFails[i]++;
                }
            } else {
                sensorSoftFailed[i] = true;
                sensorConsecutiveFails[i]++;
            }

            xSemaphoreGive(i2cBusyWire0);
        }

        // === Fallback: No interrupt in time ===
        const unsigned long timeoutMs = 350;
        if (!handled && millis() - lastSensorTriggerTime[i] > timeoutMs) {
            distances[i] = 2000;
            sensorSoftFailed[i] = true;
        }

        // === Watchdog: manually re-check sensor if it's been too long
        const unsigned long irqWatchdog = 500;
        if (sensorEvents[i] == 0 && millis() - lastSensorTriggerTime[i] > irqWatchdog) {
            sensorEvents[i]++;
        }
    }
}


