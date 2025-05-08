#include "opt.h"

OPT3101 opt;

void initOPT() {
  opt.setAddress(0x58);
  opt.resetAndWait();
  opt.init();

  if (opt.getLastError()) {
    Serial.println("❌ OPT3101 init misslyckades!");
    return;
  }

  opt.configureDefault();
  opt.setBrightness(OPT3101Brightness::Adaptive);
  opt.setMonoshotMode();
  opt.setFrameTiming(1);

  Serial.println("✅ OPT3101 initierad");
}

OptStatus checkOptObstacles() {
  // Realistiska tröskelvärden i mm
  const uint16_t CRIT_FRONT  = 120;
  const uint16_t CRIT_LEFT   = 100;
  const uint16_t CRIT_RIGHT  = 100;

  for (int ch = 0; ch < 3; ch++) {
    opt.setChannel(ch);
    opt.startSample();
    while (!opt.isSampleDone()) delay(1);
    opt.readOutputRegs();

    // ✅ OPT3101: amplitude = faktiskt avstånd
    uint16_t dist = opt.amplitude;
    uint16_t amp  = opt.distanceMillimeters;

    // Debuglogg (kan kommenteras bort senare)
    Serial.print("OPT ch ");
    Serial.print(ch);
    Serial.print(": dist = ");
    Serial.print(dist);
    Serial.print(" mm | amp = ");
    Serial.println(amp);

    if (dist > 3000 || dist == 0) continue;  // ogiltigt eller för långt

    if (ch == 0 && dist < CRIT_FRONT) {
      Serial.println("✅ OPT: FRONT kritisk");
      return OPT_CRITICAL_FRONT;
    }

    if (ch == 2 && dist < CRIT_LEFT) {
      Serial.println("✅ OPT: VÄNSTER kritisk");
      return OPT_CRITICAL_LEFT;
    }

    if (ch == 1 && dist < CRIT_RIGHT) {
      Serial.println("✅ OPT: HÖGER kritisk");
      return OPT_CRITICAL_RIGHT;
    }
  }

  return OPT_CLEAR;
}
