#ifndef EVADE_H
#define EVADE_H

#include <Arduino.h>
#include <Adafruit_MotorShield.h>


// Externa motorer (definieras i .ino)
extern Adafruit_DCMotor* motorLeft;
extern Adafruit_DCMotor* motorRight;
extern unsigned long avoidUntil;

// Konstanter
#define BASE_SPEED     180   // Normal hastighet vid hinder
#define BOOST_SPEED    220   // Boostad hastighet på raksträcka
#define BOOST_THRESH_VL 200  // Minsta VL-avstånd för raksträcka
#define BOOST_THRESH_OPT 500 // Minsta OPT-avstånd för raksträcka
#define MAX_CORRECTION 50    // Maximal styrkorrigering
#define OBSTACLE_CLOSE_OPT 900  // Tröskel för att betrakta OPT-avstånd som nära
#define OBSTACLE_CLOSE_VL   200  // Tröskel för VL53-avstånd (vänster/höger) som nära

// Funktioner
void performAvoidance(uint16_t vlL, uint16_t vlR, uint16_t optL, uint16_t optR, uint16_t front);
void backUp(uint8_t pwmL, uint8_t pwmR, uint16_t duration);
void rotateLeft(uint16_t duration);
void rotateRight(uint16_t duration);
float calcWeight(uint16_t dist, uint16_t maxRange = 2000);

#endif
