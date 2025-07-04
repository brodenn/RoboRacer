#include <Arduino.h>                     // Grundläggande Arduino-funktioner
#include <Wire.h>                        // I2C-kommunikation
#include <Adafruit_MotorShield.h>        // Styrning av Adafruit Motor FeatherWing

#include "mux.h"                         // Multiplexer-hantering (VL53L4CD)
#include "opt.h"                         // OPT3101-sensorhantering
#include "tft.h"                         // TFT-displayhantering
#include "evade.h"                       // Hinderundvikningslogik
#include "steering.h"                    // Kurskorrigering

#define PIN_START 13

// === Globala motorobjekt ===
Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x60);  // Motor FeatherWing på I2C-adress 0x60
Adafruit_DCMotor* motorLeft;                             // Vänster motor (M3)
Adafruit_DCMotor* motorRight;                            // Höger motor (M4)

unsigned long avoidUntil = 0;  // Variabel för att hålla koll på hur länge vi ska undvika hinder

void setup() {
  Serial.begin(115200);       // Startar seriell kommunikation
  delay(1000);                // Kort delay för stabil uppstart

  // Initiera I2C-kommunikation på specifika GPIO-pinnar
  Wire.begin(5, 6);           // SDA = GPIO5, SCL = GPIO6

  // Initiera motorskölden
  if (!AFMS.begin()) {
    Serial.println("❌ Kunde inte starta Motor FeatherWing!");
    while (1);                // Stoppar systemet om motorskölden inte kan initieras
  }
  Serial.println("✅ Motor FeatherWing OK");

  // Hämta motorobjekt för M3 och M4
  motorLeft  = AFMS.getMotor(3);  // Motor M3 = vänster
  motorRight = AFMS.getMotor(4);  // Motor M4 = höger

  // Initiera sensorer
  initOPT();                // Starta OPT3101-sensorer
  initVL53Sensors();        // Initiera multiplexer och VL53-sensorer

  // Initiera TFT-display
  initTFT();                // Starta upp displayen
}

void loop() {

    while (digitalRead(PIN_START) == LOW) {
  int val = digitalRead(PIN_START);
  Serial.println(val == LOW ? "⬇️ Tryckt" : "⬆️ Släppt");
  delay(300);
}
  // === Läs sensordata ===
  readOPTSensors();            // Läs in avstånd från OPT3101: [0]=LEFT, [1]=FRONT, [2]=RIGHT
  checkVL53Obstacles();        // Läs in avstånd från VL53 via multiplexer: [0]=LEFT, [1]=RIGHT

  // === Uppdatera skärm ===
  updateTFT();                 // Visa sensorvärden på TFT-displayen

  // === Beslutslogik: kontrollera hinder ===
  bool frontBlocked = optDistances[1] < 300;                       // Hinder rakt fram (< 30 cm)
  bool sideBlockedL = vl53Distances[0] > 0 && vl53Distances[0] < 100;  // Hinder vänster (< 10 cm)
  bool sideBlockedR = vl53Distances[1] > 0 && vl53Distances[1] < 100;  // Hinder höger (< 10 cm)

  // === Hinderundvikning eller kurskorrigering ===
  if (frontBlocked || sideBlockedL || sideBlockedR) {
    performAvoidance(                       // Kör väjningslogik
      vl53Distances[0],                     // Vänster VL53
      vl53Distances[1],                     // Höger VL53
      optDistances[0],                      // Vänster OPT
      optDistances[2],                      // Höger OPT
      optDistances[1]                       // Front OPT
    );
  } else {
    correctCourse();                        // Kurskorrigering när fri väg framåt
  }

  delay(10);  // Stabiliserar loopfrekvens (~10 Hz)
}
