#ifndef EVADE_H
#define EVADE_H

#include <Arduino.h>
#include <Adafruit_MotorShield.h>

// Externa motorer (definieras i .ino)
extern Adafruit_DCMotor* motorLeft;
extern Adafruit_DCMotor* motorRight;
extern unsigned long avoidUntil;
extern int currentPWM_L;
extern int currentPWM_R;
extern bool dirLeftForward;
extern bool dirRightForward;

// Konstanter
#define BASE_SPEED        170   // Normal hastighet vid hinder
#define BOOST_SPEED       220   // Boostad hastighet på raksträcka
#define BOOST_THRESH_VL   300   // Minsta VL-avstånd för raksträcka
#define BOOST_THRESH_OPT  500   // Minsta OPT-avstånd för raksträcka
#define MAX_CORRECTION    55    // Maximal styrkorrigering
#define OBSTACLE_CLOSE_OPT 500  // Tröskel för OPT-avstånd som nära
#define OBSTACLE_CLOSE_VL  200  // Tröskel för VL-avstånd som nära
#define BACKUP_DURATION   2000
#define BACKUP_PWM        170
#define INVALID_DISTANCE 9999

// Funktioner
void performAvoidance(uint16_t vlL, uint16_t vlR, uint16_t optL, uint16_t optR, uint16_t front);
void backUp(uint8_t pwmL, uint8_t pwmR, uint16_t duration);
void rotateLeft(uint16_t duration);
void rotateRight(uint16_t duration);
float calcWeight(uint16_t dist, uint16_t maxRange = 1000);

// Skyddsfunktion för mjuk riktningändring
void safeDirectionChange(uint8_t dirLeft, uint8_t dirRight, uint8_t pwmL, uint8_t pwmR);

void setLeftSpeed(int percent);
void setRightSpeed(int percent);
void moveForward();
void moveBackward();
void turnLeft();
void turnRight();
void stopOrReverse();


#endif
