#include "motors.h" 
#include "globals.h"

#define MOTOR_DEBUG false  // Set to true to enable motor logs

void setupMotors() {
    if (xSemaphoreTake(i2cBusyWire1, pdMS_TO_TICKS(50)) == pdTRUE) {
        AFMS_60.begin(1600, &Wire1);
        AFMS_61.begin(1600, &Wire1);
        xSemaphoreGive(i2cBusyWire1);
    }

    motor1 = AFMS_61.getMotor(1);  // Right-Back
    motor2 = AFMS_60.getMotor(1);  // Front-Left
    motor3 = AFMS_60.getMotor(3);  // Left-Back
    motor4 = AFMS_61.getMotor(3);  // Front-Right
}

void smoothMotorUpdate() {
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate < motorUpdateInterval) return;
    lastUpdate = millis();

    if (xSemaphoreTake(i2cBusyWire1, pdMS_TO_TICKS(10)) != pdTRUE) return;

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
    int* targets[4] = {&target_m1, &target_m2, &target_m3, &target_m4};
    int* currents[4] = {&current_m1, &current_m2, &current_m3, &current_m4};

    for (int i = 0; i < 4; i++) {
        if (!motors[i]) continue;

        if (diffs[i] != 0) {
            *currents[i] += diffs[i];

            int speed = abs(*currents[i]);
            int dir = (*currents[i] >= 0) ? FORWARD : BACKWARD;

            motors[i]->setSpeed(speed);
            motors[i]->run(dir);
        }
    }

    xSemaphoreGive(i2cBusyWire1);
}

void checkMotorShields(bool doMotorTest) {
    if (xSemaphoreTake(i2cBusyWire1, pdMS_TO_TICKS(100)) == pdTRUE) {
        AFMS_60.begin(1600, &Wire1);
        AFMS_61.begin(1600, &Wire1);
        xSemaphoreGive(i2cBusyWire1);
    }

    motor1 = AFMS_61.getMotor(1);
    motor2 = AFMS_60.getMotor(1);
    motor3 = AFMS_60.getMotor(3);
    motor4 = AFMS_61.getMotor(3);

    if (doMotorTest) {
        Adafruit_DCMotor* motors[4] = {motor1, motor2, motor3, motor4};

        for (int i = 0; i < 4; i++) {
            if (motors[i]) {
                motors[i]->setSpeed(100);
                motors[i]->run(FORWARD);
                delay(400);
                motors[i]->run(RELEASE);
                delay(150);
            }
        }
    }
}
