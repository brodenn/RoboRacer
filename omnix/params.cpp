#include <ArduinoJson.h>
#include "params.h"
#include "globals.h"
#include "mux.h"

// ✅ Global definition (single instance)
SteeringParams params;

void SteeringParams::loadFromJson(JsonDocument& doc) {
    auto update = [](float& var, const char* key, JsonDocument& doc) {
        if (doc.containsKey(key)) {
            float newVal = doc[key];
            if (var != newVal) {
                Serial.printf("🔧 %s changed: %.2f -> %.2f\n", key, var, newVal);
                var = newVal;
            }
        }
    };

    auto updateInt = [](int& var, const char* key, JsonDocument& doc) {
        if (doc.containsKey(key)) {
            int newVal = doc[key];
            if (var != newVal) {
                Serial.printf("🔧 %s changed: %d -> %d\n", key, var, newVal);
                var = newVal;
            }
        }
    };

    auto updateBool = [](bool& var, const char* key, JsonDocument& doc) {
        if (doc.containsKey(key)) {
            bool newVal = doc[key];
            if (var != newVal) {
                Serial.printf("🔧 %s changed: %s -> %s\n", key, var ? "true" : "false", newVal ? "true" : "false");
                var = newVal;
            }
        }
    };

    // === Sensor filtering & fault handling ===
    updateInt(MAX_RELIABLE_SIGMA,   "MAX_RELIABLE_SIGMA",   doc);
    updateInt(MIN_SIGNAL_THRESHOLD, "MIN_SIGNAL_THRESHOLD", doc);
    updateInt(MAX_VALID_DISTANCE,   "MAX_VALID_DISTANCE",   doc);
    updateInt(INVALID_DISTANCE,     "INVALID_DISTANCE",     doc);
    updateInt(softFailLimit,        "softFailLimit",        doc);
    updateBool(sensorRestartEnabled,"sensorRestartEnabled", doc);

    // === Detection thresholds ===
    updateInt(threshold_front, "threshold_front", doc);
    updateInt(threshold_side,  "threshold_side",  doc);
    updateInt(threshold_back,  "threshold_back",  doc);

    // === Rear tuning (alignment / avoidance) ===
    update(backAvoidStrength,        "backAvoidStrength",        doc);
    update(alignCorrectionStrength,  "alignCorrectionStrength",  doc);
    updateInt(backAvoidThreshold,    "backAvoidThreshold",       doc);

    // === Speed & steering tuning ===
    update(baseSpeedFactor,           "baseSpeedFactor",           doc);
    update(slowSpeedFactor,           "slowSpeedFactor",           doc);
    update(speedMultiplier,           "speedMultiplier",           doc);
    update(centeringStrength,         "centeringStrength",         doc);
    update(curveAnticipationStrength, "curveAnticipationStrength", doc);
    update(steeringSensitivity,       "steeringSensitivity",       doc);

    // === Window mode (interrupt config) ===
    updateInt(windowMode, "windowMode", doc);

    // === Per-sensor dynamic thresholding ===
    if (doc.containsKey("thresholds")) {
        JsonObject thresholds = doc["thresholds"];
        int overrideWindow = doc["window"] | windowMode;

        for (uint8_t i = 0; i < NUM_SENSORS; i++) {
            if (!thresholds.containsKey(String(i))) continue;

            uint16_t high = thresholds[String(i)];

            deselectAllMux();
            delayMicroseconds(200);
            selectMuxChannel(i);
            delayMicroseconds(400);

            if (xSemaphoreTake(i2cBusyWire0, pdMS_TO_TICKS(30))) {
                sensor.VL53L4CD_SetDetectionThresholds(0, high, overrideWindow);
                xSemaphoreGive(i2cBusyWire0);
                Serial.printf("📏 Sensor %d: Detection threshold set to %d mm (window=%d)\n", i, high, overrideWindow);
            }
        }
    }
}

void applyThresholdsToSensors() {
    for (uint8_t i = 0; i < NUM_SENSORS; i++) {
        uint16_t high = (i == 0) ? params.threshold_front :
                        (i == 3 || i == 4 || i == 5) ? params.threshold_back :
                        params.threshold_side;

        deselectAllMux();
        delayMicroseconds(200);
        selectMuxChannel(i);
        delayMicroseconds(400);

        if (xSemaphoreTake(i2cBusyWire0, pdMS_TO_TICKS(30))) {
            sensor.VL53L4CD_SetDetectionThresholds(0, high, params.windowMode);
            xSemaphoreGive(i2cBusyWire0);
            Serial.printf("📏 Sensor %d: Detection threshold set to %d mm (window=%d)\n", i, high, params.windowMode);
        }
    }
}

void setupParams() {
    // === Sensor filtering ===
    params.MAX_RELIABLE_SIGMA   = 20;
    params.MIN_SIGNAL_THRESHOLD = 600;
    params.MAX_VALID_DISTANCE   = 1300;
    params.INVALID_DISTANCE     = 2000;

    // === Thresholds (default 1200 mm for all) ===
    params.threshold_front = 1200;
    params.threshold_side  = 1200;
    params.threshold_back  = 1200;

    // === Fail/restart logic ===
    params.softFailLimit        = 3;
    params.sensorRestartEnabled = false;

    // === Rear tuning ===
    params.backAvoidStrength        = 0.5f;
    params.alignCorrectionStrength  = 0.5f;
    params.backAvoidThreshold       = 250;

    // === Speed and steering (safe defaults) ===
    params.baseSpeedFactor          = 255;
    params.slowSpeedFactor          = 60;
    params.speedMultiplier          = 0.3f;   // Start slow for testing
    params.centeringStrength        = 0.25f;
    params.curveAnticipationStrength= 0.6f;
    params.steeringSensitivity      = 1.2f;

    // === Sensor window mode for interrupts ===
    params.windowMode = 3;  // Trigger outside window

    Serial.println("✅ setupParams() - Parameters initialized.");
}
