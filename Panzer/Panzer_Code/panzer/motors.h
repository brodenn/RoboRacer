#ifndef MOTORS_H
#define MOTORS_H

#include <Wire.h>
#include <Adafruit_MotorShield.h>

// Init
void initMotors();

// Rörelsefunktioner
void moveForward();
void turnLeft();
void turnRight();
void slightLeft();
void slightRight();
void stopOrReverse();
void setLeftSpeed(int percent);
void setRightSpeed(int percent);
void moveBackward();


#endif
