#include <Wire.h>
#include "motors.h"
#include "mux.h"
#include "opt.h"
#include "tft.h"

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

  // Initiera I2C
  Wire.begin(5, 6);             // OPT3101 + Motor FeatherWing
  I2C_STEMMA.begin(42, 41);     // MUX + VL53
  I2C_STEMMA.setClock(400000);

  scanBus("Wire (OPT3101 + Motor FeatherWing)", Wire);
  scanBus("I2C_STEMMA (MUX + VL53)", I2C_STEMMA);

  initMotors();
  initOPT();
  initMuxAndSensors();
  initTFT();
}

void loop() {
  readOPTSensors();
  readVL53Sensors();
  updateTFT();

  Serial.print("VL53 Left: "); Serial.print(vl53Distances[0]); Serial.print(" mm   ");
  Serial.print("Right: "); Serial.println(vl53Distances[1]);

  delay(1000); // För test/debug
}
