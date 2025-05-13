#ifndef EVADE_H
#define EVADE_H

#include <Arduino.h>
#include <Adafruit_MotorShield.h>


// Externa motorer (definieras i .ino)
extern Adafruit_DCMotor* motorLeft;
extern Adafruit_DCMotor* motorRight;
extern unsigned long avoidUntil;

// Konstanter
#define BASE_SPEED 150
#define MAX_CORRECTION 50

// Funktioner
void performAvoidance(uint16_t vlL, uint16_t vlR, uint16_t optL, uint16_t optR, uint16_t front);
void backUp(uint8_t pwmL, uint8_t pwmR, uint16_t duration);
void rotateLeft(uint16_t duration);
void rotateRight(uint16_t duration);
float calcWeight(uint16_t dist, uint16_t maxRange = 2000);

#endif
