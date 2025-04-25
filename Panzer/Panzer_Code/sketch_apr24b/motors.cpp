#include "motors.h"

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
    leftMotor->setSpeed(leftMotorSpeed);
    rightMotor->setSpeed(rightMotorSpeed);
    leftMotor->run(FORWARD);
    rightMotor->run(FORWARD);
    Serial.print("✅ Vänster motor körs framåt med hastighet ");
    Serial.println(leftMotorSpeed);
    Serial.print("✅ Höger motor körs framåt med hastighet ");
    Serial.println(rightMotorSpeed);
  } else {
    Serial.println("❌ Kunde inte hitta motorer på M3/M4");
  }
}
