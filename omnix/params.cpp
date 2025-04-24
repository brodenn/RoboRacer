#include <ArduinoJson.h>
#include "params.h"
#include "globals.h"
#include "mux.h"

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

    // === Sensor filter params ===
    updateInt(MAX_RELIABLE_SIGMA,     "MAX_RELIABLE_SIGMA",        doc);
    updateInt(MIN_SIGNAL_THRESHOLD,   "MIN_SIGNAL_THRESHOLD",      doc);
    updateInt(MAX_VALID_DISTANCE,     "MAX_VALID_DISTANCE",        doc);
    updateInt(INVALID_DISTANCE,       "INVALID_DISTANCE",          doc);

    // === Soft fail & restart ===
    updateInt(softFailLimit,          "softFailLimit",             doc);
    updateBool(sensorRestartEnabled,  "sensorRestartEnabled",      doc);

    // === Sensor interrupt thresholds (group level) ===
    updateInt(threshold_front,        "threshold_front",           doc);
    updateInt(threshold_side,         "threshold_side",            doc);
    updateInt(threshold_back,         "threshold_back",            doc);

    // === New AI rear tuning ===
    update(backAvoidStrength,         "backAvoidStrength",         doc);
    update(alignCorrectionStrength,   "alignCorrectionStrength",   doc);
    updateInt(backAvoidThreshold,     "backAvoidThreshold",        doc);

    // === Live sensor threshold update (via mux) ===
    if (doc.containsKey("thresholds")) {
        JsonObject thresholds = doc["thresholds"];
        int windowMode = doc["window"] | 3;  // default to window = 3 (in range)

        for (uint8_t i = 0; i < NUM_SENSORS; i++) {
            if (!thresholds.containsKey(String(i))) continue;

            uint16_t high = thresholds[String(i)];

            deselectAllMux();
            delayMicroseconds(200);
            selectMuxChannel(i);
            delayMicroseconds(400);

            if (xSemaphoreTake(i2cBusyWire0, pdMS_TO_TICKS(30)) == pdTRUE) {
                sensor.VL53L4CD_SetDetectionThresholds(0, high, windowMode);
                xSemaphoreGive(i2cBusyWire0);
                Serial.printf("📏 Sensor %d: Detection threshold set to %d mm (window=%d)\n", i, high, windowMode);
            }
        }
    }
}

void setupParams() {
    // === Sensor filtering ===
    params.MAX_RELIABLE_SIGMA     = 20;
    params.MIN_SIGNAL_THRESHOLD   = 600;
    params.MAX_VALID_DISTANCE     = 1300;
    params.INVALID_DISTANCE       = 2000;

    // === Detection thresholds (for ISR triggering) ===
    params.threshold_front        = 600;
    params.threshold_side         = 300;
    params.threshold_back         = 250;

    // === Fail/restart settings ===
    params.softFailLimit          = 3;
    params.sensorRestartEnabled   = false;

    // === Rear behavior and correction ===
    params.backAvoidStrength      = 0.5f;
    params.alignCorrectionStrength = 0.4f;
    params.backAvoidThreshold     = 250;

    Serial.println("✅ setupParams() - Parameters initialized.");
}
