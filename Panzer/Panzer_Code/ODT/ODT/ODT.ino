#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MotorShield.h>

#include "mux.h"
#include "opt.h"
#include "tft.h"
#include "evade.h"
#include "steering.h"

// === Globala motorobjekt ===
Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x60);
Adafruit_DCMotor* motorLeft;
Adafruit_DCMotor* motorRight;

unsigned long avoidUntil = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Initiera I2C
  Wire.begin(5, 6);  // SDA, SCL

  // Init motorer
  if (!AFMS.begin()) {
    Serial.println("❌ Kunde inte starta Motor FeatherWing!");
    while (1);
  }
  Serial.println("✅ Motor FeatherWing OK");

  motorLeft  = AFMS.getMotor(3);  // M3
  motorRight = AFMS.getMotor(4);  // M4

  // Initiera sensorer
  initOPT();
  initVL53Sensors();

  // Initiera display
  initTFT();
}

void loop() {
  // === Läs sensorer ===
  readOPTSensors();            // OPT: [0]=LEFT, [1]=FRONT, [2]=RIGHT
  checkVL53Obstacles();        // VL53: [0]=LEFT, [1]=RIGHT

  // === Uppdatera display ===
  updateTFT();

  // === Välj logik ===
  bool frontBlocked = optDistances[1] < 300;
  bool sideBlockedL = vl53Distances[0] > 0 && vl53Distances[0] < 100;
  bool sideBlockedR = vl53Distances[1] > 0 && vl53Distances[1] < 100;

  if (frontBlocked || sideBlockedL || sideBlockedR) {
    performAvoidance(
      vl53Distances[0], vl53Distances[1],
      optDistances[0], optDistances[2],
      optDistances[1]
    );
  } else {
    correctCourse();  // Ny flytande kurskorrigering
  }

  delay(100);  // Stabilisera loopfrekvens (~10 Hz)
}
