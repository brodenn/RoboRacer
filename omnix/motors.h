#ifndef MOTORS_H
#define MOTORS_H

#include <Adafruit_MotorShield.h>

// --- Motor Shields ---
extern Adafruit_MotorShield AFMS_60;
extern Adafruit_MotorShield AFMS_61;

// --- Motors ---
extern Adafruit_DCMotor *motor1;
extern Adafruit_DCMotor *motor2;
extern Adafruit_DCMotor *motor3;
extern Adafruit_DCMotor *motor4;

// --- Motor control variables ---
extern int target_m1, target_m2, target_m3, target_m4;
extern int current_m1, current_m2, current_m3, current_m4;
extern const int MAX_STEP;
extern bool motorsEnabled;

// --- Functions ---
void setupMotors();
void smoothMotorUpdate();
void checkMotorShields(bool spinTest = false);   // ✅ Optional spin test (default = false)

#endif
