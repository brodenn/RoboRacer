#include "opt.h"

OPT3101 opt;
#include "opt.h"

uint16_t optDistances[3] = {0};
uint16_t optAmplitudes[3] = {0};


static int currentChannel = 0;
unsigned long lastSampleTime = 0;
const unsigned long sampleInterval = 1000 / 60; // 60 Hz

void initOPT() {
  opt.setAddress(0x58);
  opt.resetAndWait();
  opt.init();
  opt.configureDefault();
  opt.setBrightness(OPT3101Brightness::Adaptive);
  opt.setMonoshotMode();
  opt.setFrameTiming(5);
  Serial.println("✅ OPT3101 initierad");
}

void readOPTSensors() {
  if (millis() - lastSampleTime >= sampleInterval) {
    lastSampleTime = millis();

    opt.setChannel(currentChannel);
    opt.startSample();
    while (!opt.isSampleDone()) delay(1);
    opt.readOutputRegs();

    optDistances[currentChannel] = opt.distanceMillimeters;
    optAmplitudes[currentChannel] = opt.amplitude;

    currentChannel = (currentChannel + 1) % 3;
  }
}
