#ifndef MOTORS_H
#define MOTORS_H

#include <Wire.h>
#include <Adafruit_MotorShield.h>

// --- Adafruit Motor FeatherWing (I2C-adress 0x60) ---
extern Adafruit_MotorShield AFMS;
extern Adafruit_DCMotor *leftMotor;
extern Adafruit_DCMotor *rightMotor;

// Globala variabler för motorhastigheter
extern int leftMotorSpeed;
extern int rightMotorSpeed;

// Funktion för att initiera motorer
void initMotors();

#endif // MOTORS_H
