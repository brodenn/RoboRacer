#ifndef MOTORS_H
#define MOTORS_H

#include <Wire.h>
#include <Adafruit_MotorShield.h>

// --- Adafruit Motor FeatherWing (I2C-adress 0x60) ---
Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x60);
Adafruit_DCMotor *leftMotor = AFMS.getMotor(3);
Adafruit_DCMotor *rightMotor = AFMS.getMotor(4);

void initMotors() {
  Wire.begin();

  Serial.println("📡 Skannar I2C-bussen...");
  byte count = 0;
  for (byte i = 1; i < 127; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) {
      Serial.print("✅ Hittad enhet: 0x");
      Serial.println(i, HEX);
      count++;
    }
  }
  if (count == 0) Serial.println("❌ Inga I2C-enheter hittades!");
  else Serial.println("✅ Skanning klar.");

  if (!AFMS.begin()) {
    Serial.println("❌ Kunde inte starta Motor FeatherWing!");
  } else {
    Serial.println("✅ Motor FeatherWing initierad");
    if (leftMotor && rightMotor) {
      Serial.println("✅ Motorer hittade och initialiseras...");
      leftMotor->setSpeed(150);
      rightMotor->setSpeed(150);
      leftMotor->run(FORWARD);
      rightMotor->run(FORWARD);
      Serial.println("✅ Motorer körs framåt med hastighet 150");
    } else {
      Serial.println("❌ Kunde inte hitta motorer på M3/M4");
    }
  }
}

#endif
