#include <Wire.h>
#include "motors.h"
#include "mux.h"
#include "opt.h"
#include "tft.h"
#include "obstacle.h"
#include "loop_logic.h"
#include "ota.h"
#include <ArduinoOTA.h>
#include <WebSerial.h>
#include "interrupts.h"


// Separat I2C-buss för MUX + VL53 (tidigare kallad STEMMA QT)

void scanBus(const char* label, TwoWire &bus) {
  Serial.print("\xF0\x9F\x93\xA1 Skannar "); Serial.println(label);
  byte count = 0;
  for (byte i = 1; i < 127; i++) {
    bus.beginTransmission(i);
    if (bus.endTransmission() == 0) {
      Serial.print("\xE2\x9C\x85 Hittad enhet: 0x");
      Serial.println(i, HEX);
      count++;
    }
  }
  if (count == 0) Serial.println("\xE2\x9D\x8C Inga I2C-enheter hittades!");
  else Serial.println("\xE2\x9C\x85 Skanning klar.");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin();                      // Standard I2C (GPIO 41 SDA, 42 SCL) för Motor FeatherWing + OPT3101  
  Wire.setClock(400000);

  scanBus("Wire (OPT3101 + Motor FeatherWing + MUX + VL53)", Wire);

  initMotors();             
  initOPT();                
  initMuxAndSensors();     
  initTFT();
  setupWiFiAndOTA();

  // ❌ TA BORT för nu: Interruptlogik
  // setupInterrupts();
}

void loop() {
ArduinoOTA.handle();  // Måste kallas varje loop
loopLogic();
}


