#include "motors.h"
#include "globals.h"

bool motorShield60Found = false;
bool motorShield61Found = false;

void setupMotors() {
    if (xSemaphoreTake(i2cBusyWire1, pdMS_TO_TICKS(50)) == pdTRUE) {
        motorShield60Found = AFMS_60.begin(1600, &Wire1);
        motorShield61Found = AFMS_61.begin(1600, &Wire1);
        xSemaphoreGive(i2cBusyWire1);
    }

    Serial.println(motorShield60Found ? "✅ Motor Shield 0x60 detected" : "❌ Motor Shield 0x60 NOT found");
    Serial.println(motorShield61Found ? "✅ Motor Shield 0x61 detected" : "❌ Motor Shield 0x61 NOT found");

    motor1 = AFMS_61.getMotor(4);  // Right-Back
    motor2 = AFMS_60.getMotor(1);  // Front-Left
    motor3 = AFMS_60.getMotor(3);  // Left-Back
    motor4 = AFMS_61.getMotor(3);  // Front-Right
}

void smoothMotorUpdate() {
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate < motorUpdateInterval) return;
    lastUpdate = millis();

    if (xSemaphoreTake(i2cBusyWire1, pdMS_TO_TICKS(10)) != pdTRUE) {
        Serial.println("⚠️ smoothMotorUpdate: Failed to acquire i2cBusyWire1");
        return;
    }

    int diffs[4] = {
        constrain(target_m1 - current_m1, -MAX_STEP, MAX_STEP),
        constrain(target_m2 - current_m2, -MAX_STEP, MAX_STEP),
        constrain(target_m3 - current_m3, -MAX_STEP, MAX_STEP),
        constrain(target_m4 - current_m4, -MAX_STEP, MAX_STEP)
    };

    if (diffs[0] == 0 && diffs[1] == 0 && diffs[2] == 0 && diffs[3] == 0) {
        xSemaphoreGive(i2cBusyWire1);
        return;
    }

    Adafruit_DCMotor* motors[4] = {motor1, motor2, motor3, motor4};
    int* targets[4]  = {&target_m1, &target_m2, &target_m3, &target_m4};
    int* currents[4] = {&current_m1, &current_m2, &current_m3, &current_m4};
    const char* labels[4] = {"M1 (RB)", "M2 (FL)", "M3 (LB)", "M4 (FR)"};

    for (int i = 0; i < 4; i++) {
        if (!motors[i]) continue;

        *currents[i] += diffs[i];
        int rawSpeed = *currents[i];
        int absSpeed = constrain(abs(rawSpeed), 0, 255);

        if (absSpeed == 0) {
            motors[i]->run(RELEASE);
            Serial.printf("🌀 %s: RELEASED (target=%d)\n", labels[i], *targets[i]);
        } else {
            bool forward = rawSpeed > 0;
            motors[i]->run(forward ? FORWARD : BACKWARD);
            motors[i]->setSpeed(absSpeed);
            Serial.printf("🌀 %s: Dir=%s Speed=%d Target=%d Current=%d\n",
                          labels[i], forward ? "FWD" : "BWD",
                          absSpeed, *targets[i], *currents[i]);
        }
    }

    xSemaphoreGive(i2cBusyWire1);
}



void checkMotorShields(bool doMotorTest) {
    if (xSemaphoreTake(i2cBusyWire1, pdMS_TO_TICKS(100)) == pdTRUE) {
        motorShield60Found = AFMS_60.begin(1600, &Wire1);
        motorShield61Found = AFMS_61.begin(1600, &Wire1);
        xSemaphoreGive(i2cBusyWire1);
    }

    Serial.println(motorShield60Found ? "✅ Re-check: Shield 0x60 OK" : "❌ Re-check: Shield 0x60 MISSING");
    Serial.println(motorShield61Found ? "✅ Re-check: Shield 0x61 OK" : "❌ Re-check: Shield 0x61 MISSING");

    motor1 = AFMS_61.getMotor(4);
    motor2 = AFMS_60.getMotor(1);
    motor3 = AFMS_60.getMotor(3);
    motor4 = AFMS_61.getMotor(3);

    if (doMotorTest) {
        Adafruit_DCMotor* motors[4] = {motor1, motor2, motor3, motor4};
        const char* labels[4] = {"Right-Back", "Front-Left", "Left-Back", "Front-Right"};

        for (int i = 0; i < 4; i++) {
            if (!motors[i]) continue;
            Serial.printf("🔁 Testing %s motor FORWARD...\n", labels[i]);
            motors[i]->run(FORWARD);
            motors[i]->setSpeed(150);
            delay(500);
            motors[i]->run(BACKWARD);
            motors[i]->setSpeed(150);
            delay(500);
            motors[i]->run(RELEASE);
            delay(200);
        }
    }
}
