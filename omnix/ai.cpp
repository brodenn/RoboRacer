#include "ai.h"
#include "globals.h"
#include "params.h"
#include "motors.h"
#include "sensors.h"
#include <Arduino.h>

// Sensor index mapping
const int FRONT        = 0;
const int FRONT_LEFT   = 1;
const int LEFT         = 2;
const int BACK_LEFT    = 3;
const int BACK         = 4;
const int BACK_RIGHT   = 5;
const int RIGHT        = 6;
const int FRONT_RIGHT  = 7;

bool isValid(int dist) {
    return dist < INVALID_DISTANCE;
}

// Safe fallback for missing sensor values
int safeRead(int dist) {
    return isValid(dist) ? dist : 2000;
}

void aiSteering() {
    auto safeRead = [](int dist) {
        return isValid(dist) ? dist : 2000;
    };

    int front      = safeRead(distances[FRONT]);
    int frontLeft  = safeRead(distances[FRONT_LEFT]);
    int frontRight = safeRead(distances[FRONT_RIGHT]);
    int left       = safeRead(distances[LEFT]);
    int right      = safeRead(distances[RIGHT]);
    int backLeft   = safeRead(distances[BACK_LEFT]);
    int backRight  = safeRead(distances[BACK_RIGHT]);

    float speedMultiplier = params.speedMultiplier;
    int baseSpeed = int(params.baseSpeedFactor * speedMultiplier);
    int reverseSpeed = int(params.slowSpeedFactor * speedMultiplier);
    const int minDriveSpeed = 60;
    const int minSteerSpeed = 30;

    // === Print sensors ===
    Serial.printf("📡 Sensors | F=%d FL=%d FR=%d L=%d R=%d\n", front, frontLeft, frontRight, left, right);

    // === Reverse ===
    if (!reversing && front < 150) {
        reversing = true;
        reverseStart = millis();
        Serial.printf("⛔ Reverse triggered | front = %d mm\n", front);
    }

    if (reversing) {
        if (millis() - reverseStart < reverseTime) {
            bool turnRight = backLeft < backRight;
            int fast = -reverseSpeed;
            int slow = -reverseSpeed / 2;
            target_m1 = target_m4 = turnRight ? fast : slow;
            target_m2 = target_m3 = turnRight ? slow : fast;
            smoothMotorUpdate();
            return;
        } else {
            reversing = false;
            Serial.println("✅ Reverse complete");
        }
    }

    // === Centering logic ===
    int delta = right - left;
    int closest = min(left, right);
    float proximity = 1.0f - float(min(closest, 1200)) / 1200.0f;
    float strength = pow(proximity, 2);
    float balance = float(delta) / 1200.0f;
    float correction = strength * balance * 120.0f * params.centeringStrength;

    // === Curve hints ===
    if (frontLeft < 800)  correction += params.curveAnticipationStrength;
    if (frontRight < 800) correction -= params.curveAnticipationStrength;

    correction = constrain(correction, -90.0f, 90.0f);

    float leftSpeed = baseSpeed;
    float rightSpeed = baseSpeed;

    if (correction > 0) {
        leftSpeed -= correction;
    } else {
        rightSpeed += correction;
    }

    leftSpeed  = constrain(leftSpeed, minSteerSpeed, 255);
    rightSpeed = constrain(rightSpeed, minSteerSpeed, 255);

    target_m1 = rightSpeed;
    target_m4 = rightSpeed;
    target_m2 = leftSpeed;
    target_m3 = leftSpeed;

    Serial.printf("🚗 Speed | Left=%.1f Right=%.1f | Δ=%.1f (prox²=%.2f)\n",
                  leftSpeed, rightSpeed, correction, strength);

    smoothMotorUpdate();
}

