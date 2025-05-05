#include "opt.h"
#include <WebSerial.h>

OPT3101 opt;

uint16_t optDistances[3] = {0};
uint16_t optAmplitudes[3] = {0};

void initOPT() {
  opt.setAddress(0x58);               // Standard I2C-adress för OPT3101
  opt.resetAndWait();
  opt.init();
  opt.configureDefault();

  opt.setBrightness(OPT3101Brightness::Adaptive);
  opt.setMonoshotMode();             // Mätning sker per kommando
  opt.setFrameTiming(5);             // Justerbar integrationstid

  Serial.println("✅ OPT3101 initierad");
}

void readOPTSensors() {
  for (int ch = 0; ch < 3; ch++) {
    opt.setChannel(ch);
    opt.startSample();
    while (!opt.isSampleDone()) delay(1);
    opt.readOutputRegs();

    optDistances[ch] = opt.distanceMillimeters;
    optAmplitudes[ch] = opt.amplitude;
  }
}

// Alias om du hellre anropar detta i t.ex. obstacle.cpp
void readOPTFront() {
  readOPTSensors();
}
