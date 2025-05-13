#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "mux.h"
#include "opt.h"
#include "evade.h"
#include "tft.h"

#define PIN_START 14


// === Globala motorpekare ===
Adafruit_DCMotor *motorLeft = nullptr;
Adafruit_DCMotor *motorRight = nullptr;

// === Tidshantering ===
unsigned long avoidUntil = 0;

// === MotorShield-instans ===
Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x60);

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000);
  pinMode(PIN_START, INPUT_PULLUP);

 // pinMode(KILL_SWITCH_PIN, INPUT);

  AFMS.begin();
  motorLeft = AFMS.getMotor(3);
  motorRight = AFMS.getMotor(4);

  initOPT();
  initVL53Sensors();
  initTFT();

  Serial.println("🚗 Panzer redo");
}

void loop() {

  while (digitalRead(PIN_START) == LOW) {
  int val = digitalRead(PIN_START);
  Serial.println(val == LOW ? "⬇️ Tryckt" : "⬆️ Släppt");
  delay(300);
}

  if (millis() < avoidUntil) return;

  readOPTSensors();                      // Uppdaterar optDistances[]
  MuxStatus muxStatus = checkVL53Obstacles();  // Uppdaterar vl53Distances[]

  // Kör hinderlogik
  performAvoidance(
    vl53Distances[0],                   // VL vänster
    vl53Distances[1],                   // VL höger
    optDistances[0],                    // OPT vänster
    optDistances[2],                    // OPT höger
    optDistances[1]                     // OPT front
  );
}
