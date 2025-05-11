#include "opt.h"

OPT3101 opt;

void initOPT() {
  opt.setAddress(0x58);  // standard
  opt.init();

  if (opt.getLastError()) {
    Serial.println("❌ OPT3101 init misslyckades!");
    return;
  }

  opt.setFrameTiming(256);  // stabil uppdatering
  opt.setBrightness(OPT3101Brightness::Adaptive);

  Serial.println("✅ OPT3101 initierad");
}

OptStatus checkOptObstacles() {
  const uint16_t CRIT_FRONT  = 300;
  const uint16_t CRIT_LEFT   = 400;
  const uint16_t CRIT_RIGHT  = 400;

  const uint16_t MIN_AMPLITUDE = 50;
  const uint16_t MAX_DISTANCE  = 4000;

  OptStatus lastCritical = OPT_CLEAR;

  for (int ch = 0; ch < 3; ch++) {
    opt.setChannel(ch);
    opt.sample();  // använder samma som i testkod

    uint16_t dist = opt.distanceMillimeters;
    uint16_t amp  = opt.amplitude;
    uint16_t amb  = opt.ambient;

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

    // Riktning: ch 1 = FRONT, ch 2 = LEFT, ch 0 = RIGHT (verifierat från test)
    if (ch == 1 && dist < CRIT_FRONT) {
      Serial.println("✅ OPT: FRONT kritisk");
      lastCritical = OPT_CRITICAL_FRONT;
    }
    if (ch == 2 && dist < CRIT_LEFT && lastCritical == OPT_CLEAR) {
      Serial.println("✅ OPT: VÄNSTER kritisk");
      lastCritical = OPT_CRITICAL_LEFT;
    }
    if (ch == 0 && dist < CRIT_RIGHT && lastCritical == OPT_CLEAR) {
      Serial.println("✅ OPT: HÖGER kritisk");
      lastCritical = OPT_CRITICAL_RIGHT;
    }
  }

  return lastCritical;
}
