#include "ai.h"
#include "globals.h"
#include "params.h"
#include "motors.h"
#include "sensors.h"
#include <Arduino.h>

// === Sensor Indexes ===
const int FRONT        = 1;
const int FRONT_LEFT   = 3;
const int FRONT_RIGHT  = 2;
const int LEFT         = 0;
const int RIGHT        = 5;
const int BACK_LEFT    = 4;
const int BACK         = 6;
const int BACK_RIGHT   = 7;

// === Helper Functions ===
bool isValid(int dist) {
    return dist < INVALID_DISTANCE;
}

bool isTooClose(int dist, int threshold) {
    return isValid(dist) && dist < threshold;
}

bool shouldReverse(int front) {
    return isTooClose(front, params.threshold_front);
}

float computeCenteringCorrection(int left, int right) {
    if (!isValid(left) || !isValid(right)) return 0.0;
    float delta = float(right - left);
    return delta * params.centeringStrength;
}

float computeCurveAdjustment(int fl, int fr, int left, int right) {
    float adjust = 0.0;
    if (isValid(fl) && fl < params.threshold_side && left > fl)
        adjust += params.curveAnticipationStrength * (params.steeringSensitivity / 2);
    if (isValid(fr) && fr < params.threshold_side && right > fr)
        adjust -= params.curveAnticipationStrength * (params.steeringSensitivity / 2);
    return adjust;
}

float computeBackAlignment(int bl, int br) {
    if (isValid(bl) && isValid(br)) {
        int delta = bl - br;
        if (abs(delta) > 60)
            return delta * params.alignCorrectionStrength;
    }
    return 0.0;
}

float computeBackAvoidPush(int bl, int br) {
    if (isValid(bl) && isValid(br) &&
        bl < params.backAvoidThreshold && br < params.backAvoidThreshold) {
        int push = params.backAvoidThreshold - min(bl, br);
        return push * params.backAvoidStrength;
    }
    return 0.0;
}

// === Main Steering Logic ===
void aiSteering() {
    // Sensor readings
    int front      = distances[FRONT];
    int frontLeft  = distances[FRONT_LEFT];
    int frontRight = distances[FRONT_RIGHT];
    int left       = distances[LEFT];
    int right      = distances[RIGHT];
    int backLeft   = distances[BACK_LEFT];
    int back       = distances[BACK];
    int backRight  = distances[BACK_RIGHT];

    // Speed setup
    float speedMultiplier = params.speedMultiplier;
    int forwardSpeed = int(params.baseSpeedFactor * speedMultiplier);
    int reverseSpeed = int(params.slowSpeedFactor * speedMultiplier);

    // === Reversing ===
    if (!reversing && shouldReverse(front)) {
        reversing = true;
        reverseStart = millis();
    }

    if (reversing) {
        if (millis() - reverseStart < reverseTime) {
            bool turnRight = isValid(backLeft) && (!isValid(backRight) || backLeft < backRight);
            int fast = -reverseSpeed;
            int slow = -reverseSpeed / 2;
            target_m1 = target_m2 = turnRight ? fast : slow;
            target_m3 = target_m4 = turnRight ? slow : fast;
            smoothMotorUpdate();
            return;
        } else {
            reversing = false;
        }
    }

    // === Forward Steering ===
    float leftSpeed  = forwardSpeed;
    float rightSpeed = forwardSpeed;

    // Centering between walls
    float centering = computeCenteringCorrection(left, right);
    leftSpeed  -= centering;
    rightSpeed += centering;

    // Curve anticipation
    float curve = computeCurveAdjustment(frontLeft, frontRight, left, right);
    leftSpeed  -= curve;
    rightSpeed += curve;

    // Back alignment
    float align = computeBackAlignment(backLeft, backRight);
    leftSpeed  -= align;
    rightSpeed += align;

    // Back push-away correction
    float backPush = computeBackAvoidPush(backLeft, backRight);
    leftSpeed  -= backPush;
    rightSpeed -= backPush;

    // === Fallback
    if (!isValid(front) && !isValid(left) && !isValid(right)) {
        leftSpeed = rightSpeed = 30; // cautious mode
    }

    // Clamp and apply
    leftSpeed  = constrain(leftSpeed, 30, 255);
    rightSpeed = constrain(rightSpeed, 30, 255);

    target_m1 = rightSpeed;
    target_m2 = rightSpeed;
    target_m3 = leftSpeed;
    target_m4 = leftSpeed;

    smoothMotorUpdate();
}
