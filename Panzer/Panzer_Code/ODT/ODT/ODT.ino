#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "mux.h"
#include "opt.h"

// === Motorinstanser ===
Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x60);
Adafruit_DCMotor *motorLeft = AFMS.getMotor(3);
Adafruit_DCMotor *motorRight = AFMS.getMotor(4);

// === Körläge ===
enum DriveMode {
  MODE_STOPPED,
  MODE_FORWARD,
  MODE_TURN_LEFT_SOFT,
  MODE_TURN_RIGHT_SOFT
};

DriveMode currentMode = MODE_STOPPED;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000);
  AFMS.begin();

  initOPT();
  initVL53Sensors();  // <- ersätter all direkt VL53-init
}

void loop() {
  // === Fråga OPT-status ===
  OptStatus opt = checkOptObstacles();
  if (opt == OPT_CRITICAL_FRONT) {
    Serial.println("⚠️ OPT: Hinder fram → sväng höger");
    turnRightSoft();
    return;
  } else if (opt == OPT_CRITICAL_LEFT) {
    Serial.println("⚠️ OPT: För nära vänster → sväng höger");
    turnRightSoft();
    return;
  } else if (opt == OPT_CRITICAL_RIGHT) {
    Serial.println("⚠️ OPT: För nära höger → sväng vänster");
    turnLeftSoft();
    return;
  }

  // === Fråga VL53-status via mux ===
  MuxStatus mux = checkVL53Obstacles();
  if (mux == MUX_CRITICAL_LEFT) {
    Serial.println("⚠️ VL53: För nära vänster → sväng höger");
    turnRightSoft();
    return;
  } else if (mux == MUX_CRITICAL_RIGHT) {
    Serial.println("⚠️ VL53: För nära höger → sväng vänster");
    turnLeftSoft();
    return;
  }

  // === Inget kritiskt → kör rakt
  driveForward();
}

// === Motorstyrning ===
void driveForward() {
  if (currentMode != MODE_FORWARD) {
    motorLeft->setSpeed(255);
    motorRight->setSpeed(255);
    motorLeft->run(FORWARD);
    motorRight->run(FORWARD);
    currentMode = MODE_FORWARD;
    Serial.println("▶️ Framåt");
  }
}

void turnLeftSoft() {
  if (currentMode != MODE_TURN_LEFT_SOFT) {
    motorLeft->setSpeed(100);
    motorRight->setSpeed(200);
    motorLeft->run(FORWARD);
    motorRight->run(FORWARD);
    currentMode = MODE_TURN_LEFT_SOFT;
    Serial.println("⬅️ Vänster (mild)");
  }
}

void turnRightSoft() {
  if (currentMode != MODE_TURN_RIGHT_SOFT) {
    motorLeft->setSpeed(200);
    motorRight->setSpeed(100);
    motorLeft->run(FORWARD);
    motorRight->run(FORWARD);
    currentMode = MODE_TURN_RIGHT_SOFT;
    Serial.println("➡️ Höger (mild)");
  }
}
