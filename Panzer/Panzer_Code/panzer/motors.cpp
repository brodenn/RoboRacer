#include "motors.h"
#include <WebSerial.h>


Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x60);
Adafruit_DCMotor *leftMotor = AFMS.getMotor(3);
Adafruit_DCMotor *rightMotor = AFMS.getMotor(4);
int leftMotorSpeed = 150;
int rightMotorSpeed = 150;

void initMotors() {
  if (!AFMS.begin()) {
    Serial.println("❌ Kunde inte starta Motor FeatherWing!");
    return;
  }

  Serial.println("✅ Motor FeatherWing initierad");
  if (leftMotor && rightMotor) {
    Serial.println("✅ Motorer hittade och initialiseras...");
    moveForward();
  } else {
    Serial.println("❌ Kunde inte hitta motorer på M3/M4");
  }
}

void moveForward() {
  leftMotor->setSpeed(leftMotorSpeed);
  rightMotor->setSpeed(rightMotorSpeed);
  leftMotor->run(FORWARD);
  rightMotor->run(FORWARD);
}

void turnLeft() {
  leftMotor->setSpeed(100); // Minska eller stanna vänster
  rightMotor->setSpeed(rightMotorSpeed);
  leftMotor->run(BACKWARD);
  rightMotor->run(FORWARD);
}

void turnRight() {
  leftMotor->setSpeed(leftMotorSpeed);
  rightMotor->setSpeed(100); // Minska eller stanna höger
  leftMotor->run(FORWARD);
  rightMotor->run(BACKWARD);
}

void slightLeft() {
  leftMotor->setSpeed(leftMotorSpeed / 2);
  rightMotor->setSpeed(rightMotorSpeed);
  leftMotor->run(FORWARD);
  rightMotor->run(FORWARD);
}

void slightRight() {
  leftMotor->setSpeed(leftMotorSpeed);
  rightMotor->setSpeed(rightMotorSpeed / 2);
  leftMotor->run(FORWARD);
  rightMotor->run(FORWARD);
}

void stopOrReverse() {
  leftMotor->setSpeed(0);
  rightMotor->setSpeed(0);
  leftMotor->run(RELEASE);
  rightMotor->run(RELEASE);
}
void setLeftSpeed(int percent) {
  leftMotorSpeed = map(constrain(percent, 0, 100), 0, 100, 0, 255);
  leftMotor->setSpeed(leftMotorSpeed);
}

void setRightSpeed(int percent) {
  rightMotorSpeed = map(constrain(percent, 0, 100), 0, 100, 0, 255);
  rightMotor->setSpeed(rightMotorSpeed);
}

void moveBackward() {
  leftMotor->setSpeed(leftMotorSpeed);
  rightMotor->setSpeed(rightMotorSpeed);
  leftMotor->run(BACKWARD);
  rightMotor->run(BACKWARD);
}
