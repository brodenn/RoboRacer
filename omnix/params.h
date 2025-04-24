#ifndef PARAMS_H
#define PARAMS_H

#include <ArduinoJson.h>

// ----------
// Steering Parameters Struct
// ----------

struct SteeringParams {
    float wallEscapeStrength;
    float wallEscapeMin;
    float wallEscapeMax;
    float wallEscapeThreshold;
    float wallCenteringStrength;
    float centeringStrength;
    float softWallPush;
    float curveAnticipationStrength;
    float zigzagSuppressionThreshold;
    float antiZigzagThresholdHigh;
    float speedMultiplier;
    float baseSpeedFactor;
    float slowSpeedFactor;
    float steeringSensitivity;
    float steerAdjustScale;

    // --- ✅ Soft-fail handling ---
    int softFailLimit;
    bool sensorRestartEnabled;

    // --- ✅ Sensor filtering thresholds ---
    int MAX_RELIABLE_SIGMA;
    int MIN_SIGNAL_THRESHOLD;
    int MAX_VALID_DISTANCE;
    int INVALID_DISTANCE;

    // --- ✅ Detection thresholds per sensor group ---
    int threshold_front;
    int threshold_side;
    int threshold_back;

    // --- ✅ Rear alignment & avoidance tuning ---
    float backAvoidStrength;
    float alignCorrectionStrength;
    int backAvoidThreshold;

    void loadFromJson(ArduinoJson::JsonDocument& doc);
};

extern SteeringParams params;

// Setup defaults
void setupParams();

#endif
