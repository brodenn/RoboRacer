#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "opt.h"

Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x60);
Adafruit_DCMotor *motorLeft = AFMS.getMotor(3);
Adafruit_DCMotor *motorRight = AFMS.getMotor(4);

enum Mode { MODE_FORWARD, MODE_REVERSE, MODE_ROTATE };
Mode currentMode = MODE_FORWARD;

unsigned long avoidUntil = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000);
  AFMS.begin();
  initOPT();
}

void loop() {
  unsigned long now = millis();
  if (now < avoidUntil) return;

  // === Läs avstånd
  float dFront = readChannel(1);
  float dRight = readChannel(2);
  float dLeft  = readChannel(0);

  // === Smart backlogik
  if (dFront < 120 && dLeft < 200 && dRight < 200) {
    Serial.println("🧠 TRÅNGT! Backar och roterar mot öppen sida.");
    backUp(200, 800);
    if (dLeft > dRight) rotateLeft(800);
    else rotateRight(800);
    return;
  }

  // === Fartstyrning: framåt ju mer plats
  int baseSpeed = map(constrain(dFront, 200, 1500), 200, 1500, 100, 255);

  // === Riktningsstyrning: skillnad vänster-höger
  float balance = dLeft - dRight; // positiv = mer plats vänster
  int turn = constrain((int)(balance / 10.0), -80, 80); // skala till PWM-justering

  int pwmL = constrain(baseSpeed - turn, 0, 255);
  int pwmR = constrain(baseSpeed + turn, 0, 255);

  // === Kör framåt med styrkompensation
  motorLeft->setSpeed(pwmL);
  motorRight->setSpeed(pwmR);
  motorLeft->run(FORWARD);
  motorRight->run(FORWARD);
  currentMode = MODE_FORWARD;

  Serial.print("▶️ PWM L="); Serial.print(pwmL);
  Serial.print(" R="); Serial.print(pwmR);
  Serial.print(" | Dist F="); Serial.print(dFront);
  Serial.print(" L="); Serial.print(dLeft);
  Serial.print(" R="); Serial.println(dRight);
}

// === Funktioner ===

float readChannel(uint8_t ch) {
  opt.setChannel(ch);
  opt.startSample();
  while (!opt.isSampleDone()) delay(1);
  opt.readOutputRegs();
  float dist = (float)opt.distanceMillimeters;
  if (dist < 50 || dist > 8000) dist = 9999;
  return dist;
}

void backUp(uint8_t speed, uint16_t duration) {
  motorLeft->setSpeed(speed);
  motorRight->setSpeed(speed);
  motorLeft->run(BACKWARD);
  motorRight->run(BACKWARD);
  Serial.println("🔙 BACKAR");
  avoidUntil = millis() + duration;
  currentMode = MODE_REVERSE;
}

void rotateLeft(uint16_t duration) {
  motorLeft->setSpeed(100);
  motorRight->setSpeed(100);
  motorLeft->run(BACKWARD);
  motorRight->run(FORWARD);
  Serial.println("🔄 ROTERAR VÄNSTER");
  avoidUntil = millis() + duration;
  currentMode = MODE_ROTATE;
}

void rotateRight(uint16_t duration) {
  motorLeft->setSpeed(100);
  motorRight->setSpeed(100);
  motorLeft->run(FORWARD);
  motorRight->run(BACKWARD);
  Serial.println("🔁 ROTERAR HÖGER");
  avoidUntil = millis() + duration;
  currentMode = MODE_ROTATE;
}
