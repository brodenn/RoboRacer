#include "opt.h"

OPT3101 opt;
uint16_t optDistances[3] = {0};
uint16_t optAmplitudes[3] = {0};

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
  opt.setFrameTiming(5);  // ca 200 Hz internt
  Serial.println("✅ OPT3101 init OK");
}

void readOPTSensors() {
  for (int ch = 0; ch < 3; ch++) {
    opt.setChannel(ch);
    opt.startSample();
    while (!opt.isSampleDone()) delay(1);  // typiskt 5–10 ms
    opt.readOutputRegs();

    optDistances[ch] = opt.distanceMillimeters;
    optAmplitudes[ch] = opt.amplitude;
  }
}
