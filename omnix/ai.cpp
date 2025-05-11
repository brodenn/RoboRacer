#include "ai.h"
#include "globals.h"
#include "params.h"
#include "motors.h"
#include "sensors.h"
#include <Arduino.h>

// Sensor index mapping
const int FRONT = 0, FRONT_LEFT = 1, LEFT = 2, BACK_LEFT = 3;
const int BACK = 4, BACK_RIGHT = 5, RIGHT = 6, FRONT_RIGHT = 7;

bool isValid(int dist) {
    return dist < INVALID_DISTANCE;
}

int safeRead(int dist) {
    return isValid(dist) ? dist : 2000;
}

void aiSteering() {
    int front      = safeRead(distances[FRONT]);
    int frontLeft  = safeRead(distances[FRONT_LEFT]);
    int frontRight = safeRead(distances[FRONT_RIGHT]);
    int left       = safeRead(distances[LEFT]);
    int backLeft   = safeRead(distances[BACK_LEFT]);
    int right      = safeRead(distances[RIGHT]);
    int backRight  = safeRead(distances[BACK_RIGHT]);

    float speedMultiplier = params.speedMultiplier;
    int baseSpeed = int(params.baseSpeedFactor * speedMultiplier);
    int reverseSpeed = int(params.slowSpeedFactor * speedMultiplier);
    const int minSteerSpeed = 30;

    // === Enhanced Centering ===
    int leftSum  = left + backLeft + frontLeft;
    int rightSum = right + backRight + frontRight;
    int leftAvg  = leftSum / 3;
    int rightAvg = rightSum / 3;

    int delta = rightAvg - leftAvg;
    int closest = min(leftAvg, rightAvg);
    float proximity = 1.0f - float(min(closest, 1200)) / 1200.0f;
    float strength = pow(proximity, 1.5f);  // make it more aggressive
    float balance = float(delta) / 1200.0f;

    float correction = strength * balance * 200.0f * params.centeringStrength;

    // === Curve anticipation ===
    if (isValid(frontLeft) && frontLeft < 800) {
        float factor = 1.0f - float(frontLeft) / 800.0f;
        correction += factor * 70.0f * params.curveAnticipationStrength;
        Serial.printf("↩️ Curve Left | FL=%d → boost=%.1f\n", frontLeft, factor * 70.0f);
    }

    if (isValid(frontRight) && frontRight < 800) {
        float factor = 1.0f - float(frontRight) / 800.0f;
        correction -= factor * 70.0f * params.curveAnticipationStrength;
        Serial.printf("↪️ Curve Right | FR=%d → boost=%.1f\n", frontRight, factor * 70.0f);
    }

    correction = constrain(correction, -120.0f, 120.0f);  // allow sharper turn

    // === Apply steering speeds ===
    float leftSpeed = baseSpeed;
    float rightSpeed = baseSpeed;

    if (correction > 0) {
        leftSpeed -= correction;
    } else {
        rightSpeed += correction;
    }

    leftSpeed  = constrain(leftSpeed, minSteerSpeed, baseSpeed);
    rightSpeed = constrain(rightSpeed, minSteerSpeed, baseSpeed);

    target_m1 = rightSpeed;
    target_m4 = rightSpeed;
    target_m2 = leftSpeed;
    target_m3 = leftSpeed;

    const char* steerDir =
        (correction > 15)  ? "→ RIGHT" :
        (correction < -15) ? "← LEFT" :
                             "↑ STRAIGHT";

    Serial.printf("🚗 Speed | L=%.1f R=%.1f | Δ=%.1f | Prox=%.2f | Dir: %s\n",
                  leftSpeed, rightSpeed, correction, strength, steerDir);

    smoothMotorUpdate();
}

