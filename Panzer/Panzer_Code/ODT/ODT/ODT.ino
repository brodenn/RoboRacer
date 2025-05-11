#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "mux.h"
#include "opt.h"

// === Motorinstanser ===
Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x60);
Adafruit_DCMotor *motorLeft = AFMS.getMotor(3);
Adafruit_DCMotor *motorRight = AFMS.getMotor(4);

// === PWM-gränser ===
const uint8_t BASE_SPEED = 160;
const int MAX_CORRECTION = 100;

// === Tidskontroll ===
unsigned long avoidUntil = 0;

// === Setup ===
void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000);
  AFMS.begin();
  initOPT();
  initVL53Sensors();
  Serial.println("🚗 Panzer redo");
}

void loop() {
  if (millis() < avoidUntil) return;  // vänta ut back/rotation

  // === Läs sensorer ===
  OptStatus optStatus = checkOptObstacles();
  MuxStatus muxStatus = checkVL53Obstacles();  // detta måste köras

  uint16_t front  = opt.distanceMillimeters;
  uint16_t optL   = opt.amplitude; // kanal 0
  uint16_t optR   = opt.amplitude; // kanal 2 – används ej direkt här

  uint16_t vlL = vl53Distances[0]; // vänster
  uint16_t vlR = vl53Distances[1]; // höger

  // === Visa VL53-mätvärden
  Serial.print("📏 VL53: Vänster = ");
  Serial.print(vlL);
  Serial.print(" mm | Höger = ");
  Serial.println(vlR);

  // === VL53 prioritet om < 100 mm
  if (vlL > 0 && vlL < 100) {
    Serial.println("⚠️ VL53: Vägg nära vänster → sväng höger");
    turnRightSoft(vlL);
    return;
  }
  if (vlR > 0 && vlR < 100) {
    Serial.println("⚠️ VL53: Vägg nära höger → sväng vänster");
    turnLeftSoft(vlR);
    return;
  }

  // === Kritisk frontavstånd
  if (front < 100 && front > 0) {
    Serial.println("🚨 Front nära (<100 mm) → väljer bäst riktning");

    if (opt.distanceMillimeters == 0) return;

    if (optR > optL) {
      rotateLeft(600);
    } else {
      backUp(180, 180, 500);
    }
    return;
  }

  // === Adaptiv riktning – beräkna styrsignal
  int16_t steer = ((int16_t)vlL - (int16_t)vlR) * 0.8;
  steer = constrain(steer, -MAX_CORRECTION, MAX_CORRECTION);

  // === PWM-justering
  int pwmLeft  = constrain(BASE_SPEED - steer, 0, 255);
  int pwmRight = constrain(BASE_SPEED + steer, 0, 255);

  motorLeft->setSpeed(pwmLeft);
  motorRight->setSpeed(pwmRight);
  motorLeft->run(FORWARD);
  motorRight->run(FORWARD);

  Serial.print("▶️ Kör: L="); Serial.print(pwmLeft);
  Serial.print(" R="); Serial.println(pwmRight);
}

// === Backa med timer ===
void backUp(uint8_t pwmL, uint8_t pwmR, uint16_t duration) {
  motorLeft->setSpeed(pwmL);
  motorRight->setSpeed(pwmR);
  motorLeft->run(BACKWARD);
  motorRight->run(BACKWARD);
  Serial.println("🔙 Backar");
  avoidUntil = millis() + duration;
}

// === Roterar vänster på stället ===
void rotateLeft(uint16_t duration) {
  motorLeft->setSpeed(100);
  motorRight->setSpeed(100);
  motorLeft->run(BACKWARD);
  motorRight->run(FORWARD);
  Serial.println("🔄 Startar rotation V");
  avoidUntil = millis() + duration;
}

// === Roterar höger på stället ===
void rotateRight(uint16_t duration) {
  motorLeft->setSpeed(100);
  motorRight->setSpeed(100);
  motorLeft->run(FORWARD);
  motorRight->run(BACKWARD);
  Serial.println("🔄 Startar rotation H");
  avoidUntil = millis() + duration;
}

float calcTurnFactor(uint16_t dist) {
  if (dist == 0 || dist > 600) return 0.2;  // långt bort → mild styrning
  if (dist < 100) return 1.0;               // extremt nära → skarp styrning
  return 0.2 + (500.0 - dist) / 500.0 * 0.8;  // linjär mellan 0.2 och 1.0
}

void turnRightSoft(uint16_t dist) {
  float factor = calcTurnFactor(dist);
  int pwmL = constrain(BASE_SPEED + (int)(MAX_CORRECTION * factor), 0, 255);
  int pwmR = constrain(BASE_SPEED - (int)(MAX_CORRECTION * factor), 0, 255);

  motorLeft->setSpeed(pwmL);
  motorRight->setSpeed(pwmR);
  motorLeft->run(FORWARD);
  motorRight->run(FORWARD);

  Serial.print("➡️ Sväng höger: L="); Serial.print(pwmL);
  Serial.print(" R="); Serial.println(pwmR);
}

void turnLeftSoft(uint16_t dist) {
  float factor = calcTurnFactor(dist);
  int pwmL = constrain(BASE_SPEED - (int)(MAX_CORRECTION * factor), 0, 255);
  int pwmR = constrain(BASE_SPEED + (int)(MAX_CORRECTION * factor), 0, 255);

  motorLeft->setSpeed(pwmL);
  motorRight->setSpeed(pwmR);
  motorLeft->run(FORWARD);
  motorRight->run(FORWARD);

  Serial.print("⬅️ Sväng vänster: L="); Serial.print(pwmL);
  Serial.print(" R="); Serial.println(pwmR);
}
