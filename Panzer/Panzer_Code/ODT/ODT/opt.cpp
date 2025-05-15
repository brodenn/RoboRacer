#include "opt.h"

OPT3101 opt;
uint16_t optDistances[3] = {9999, 9999, 9999};  // [0]=LEFT, [1]=FRONT, [2]=RIGHT

void initOPT() {
  opt.setAddress(0x58);  // standard
  opt.init();

  if (opt.getLastError()) {
    Serial.println("❌ OPT3101 init misslyckades!");
    return;
  }

  opt.setFrameTiming(128);  // snabbare uppdatering
  opt.setBrightness(OPT3101Brightness::High);

  Serial.println("✅ OPT3101 initierad");
}

OptStatus checkOptObstacles() {
  const uint16_t CRIT_FRONT  = 300;
  const uint16_t CRIT_LEFT   = 450;
  const uint16_t CRIT_RIGHT  = 450;

  const uint16_t MIN_AMPLITUDE = 40;
  const uint16_t MAX_DISTANCE  = 1000;

  OptStatus lastCritical = OPT_CLEAR;

  for (int ch = 0; ch < 3; ch++) {
    opt.setChannel(ch);
    opt.sample();  // blockande mätning

    uint16_t dist = opt.distanceMillimeters;
    uint16_t amp  = opt.amplitude;
    uint16_t amb  = opt.ambient;

    optDistances[ch] = dist;  // <-- Spara mätvärde till array

    Serial.print("OPT ch ");
    Serial.print(ch);
    Serial.print(": dist = ");
    Serial.print(dist);
    Serial.print(" mm | amp = ");
    Serial.print(amp);
    Serial.print(" | amb = ");
    Serial.println(amb);

    // Felskydd
    if (amp < MIN_AMPLITUDE || dist > MAX_DISTANCE || dist == 0) {
      Serial.println("⚠️ Ogiltig mätning – ignorerar");
      continue;
    }

    // Riktning
    if (ch == 1 && dist < CRIT_FRONT) {
      Serial.println("✅ OPT: FRONT kritisk");
      lastCritical = OPT_CRITICAL_FRONT;
    }
    if (ch == 0 && dist < CRIT_LEFT && lastCritical == OPT_CLEAR) {
      Serial.println("✅ OPT: VÄNSTER kritisk");
      lastCritical = OPT_CRITICAL_LEFT;
    }
    if (ch == 2 && dist < CRIT_RIGHT && lastCritical == OPT_CLEAR) {
      Serial.println("✅ OPT: HÖGER kritisk");
      lastCritical = OPT_CRITICAL_RIGHT;
    }
  }

  return lastCritical;
}
void readOPTSensors() {
  for (int ch = 0; ch < 3; ch++) {
    opt.setChannel(ch);
    opt.sample();  // blockande mätning

    uint16_t dist = opt.distanceMillimeters;
    uint16_t amp  = opt.amplitude;

    if (amp < 100 || dist == 0) {
      Serial.print("⚠️ OPT ch "); Serial.print(ch);
      Serial.println(": ogiltig mätning – ignoreras");
      optDistances[ch] = 9999;  // indikera ogiltigt
    } else {
      optDistances[ch] = dist;
    }

    Serial.print("OPT ch ");
    Serial.print(ch);
    Serial.print(": dist = ");
    Serial.print(optDistances[ch]);
    Serial.println(" mm");
  }
}

