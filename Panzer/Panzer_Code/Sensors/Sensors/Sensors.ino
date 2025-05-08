#include <Wire.h>
#include "mux.h"
#include "vl53l4cd_class.h"
#include "opt.h"

// === Sensorinstanser ===
VL53L4CD vl53_left(&Wire, -1);

// === Mätdata ===
uint16_t vlL = 9999;
uint32_t ts_vlL = 0;

uint16_t optVals[3] = {9999, 9999, 9999};
uint32_t ts_optVals[3] = {0, 0, 0};

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000);  // snabb I2C
  delay(300);

  Serial.println("🟢 Initierar VL53 vänster...");

  // === Init VL53 vänster (MUX 0) ===
  selectMuxChannel(0);
  vl53_left.VL53L4CD_Off(); vl53_left.VL53L4CD_On();
  if (vl53_left.InitSensor() == 0) {
    vl53_left.VL53L4CD_SetRangeTiming(15, 0);
    vl53_left.VL53L4CD_StartRanging();  // kontinuerligt läge
    Serial.println("✅ VL53 vänster klar");
  } else {
    Serial.println("❌ VL53 vänster FEL");
  }

  initOPT();
  Serial.println("✅ OPT3101 klar");
  opt.configureDefault();
opt.setBrightness(OPT3101Brightness::Adaptive);
opt.setMonoshotMode();  // redan OK
opt.setFrameTiming(1);  // ← detta SÄTTER ~5ms samplingstid

}

void loop() {
  // === VL53 vänster (kontinuerlig) ===
  selectMuxChannel(0);
  VL53L4CD_Result_t result;
  if (vl53_left.VL53L4CD_GetResult(&result) == 0) {
    vlL = result.distance_mm;
    vl53_left.VL53L4CD_ClearInterrupt();
    ts_vlL = millis();
  }

  // === OPT alla kanaler (0–2) varje loop ===
  for (int ch = 0; ch < 3; ch++) {
    opt.setChannel(ch);
    opt.startSample();
    while (!opt.isSampleDone()) delay(1);
    opt.readOutputRegs();
    optVals[ch] = opt.distanceMillimeters;
    ts_optVals[ch] = millis();
  }

  // === Serial output ===
  Serial.print("VL L: ");
  Serial.print(vlL); Serial.print(" @ "); Serial.print(ts_vlL); Serial.print(" ms");

  Serial.print(" || OPT L: ");
  Serial.print(optVals[0]); Serial.print(" @ "); Serial.print(ts_optVals[0]);

  Serial.print(" | F: ");
  Serial.print(optVals[1]); Serial.print(" @ "); Serial.print(ts_optVals[1]);

  Serial.print(" | R: ");
  Serial.print(optVals[2]); Serial.print(" @ "); Serial.print(ts_optVals[2]);

  Serial.println(" ms");
}
