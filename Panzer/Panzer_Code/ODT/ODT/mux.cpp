#include <Wire.h>
#include "mux.h"
#include "vl53l4cd_class.h"

VL53L4CD vl53_left(&Wire, -1);
VL53L4CD vl53_right(&Wire, -1);
uint16_t vl53Distances[2] = {0, 0};

void selectMuxChannel(uint8_t channel) {
  Wire.beginTransmission(MUX_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

void initVL53Sensors() {
  selectMuxChannel(0);
  vl53_left.VL53L4CD_Off();
  vl53_left.VL53L4CD_On();
  if (vl53_left.InitSensor() == 0) {
    vl53_left.VL53L4CD_SetRangeTiming(15, 0);
    vl53_left.VL53L4CD_StartRanging();
    Serial.println("✅ VL53 vänster initierad");
  }

  selectMuxChannel(3);
  vl53_right.VL53L4CD_Off();
  vl53_right.VL53L4CD_On();
  if (vl53_right.InitSensor() == 0) {
    vl53_right.VL53L4CD_SetRangeTiming(15, 0);
    vl53_right.VL53L4CD_StartRanging();
    Serial.println("✅ VL53 höger initierad");
  }
}

MuxStatus checkVL53Obstacles() {
  uint8_t ready = 0;
  VL53L4CD_Result_t result;

  // Vänster
  selectMuxChannel(0);
  vl53_left.VL53L4CD_CheckForDataReady(&ready);
  if (ready) {
    vl53_left.VL53L4CD_GetResult(&result);
    vl53_left.VL53L4CD_ClearInterrupt();
    vl53Distances[0] = result.distance_mm;
    Serial.print("📏 VL53 V: "); Serial.print(vl53Distances[0]); Serial.println(" mm");
    if (vl53Distances[0] < VL53_CRIT_LEFT && vl53Distances[0] > 0) {
      return MUX_CRITICAL_LEFT;
    }
  }

  // Höger
  ready = 0;
  selectMuxChannel(3);
  vl53_right.VL53L4CD_CheckForDataReady(&ready);
  if (ready) {
    vl53_right.VL53L4CD_GetResult(&result);
    vl53_right.VL53L4CD_ClearInterrupt();
    vl53Distances[1] = result.distance_mm;
    Serial.print("📏 VL53 H: "); Serial.print(vl53Distances[1]); Serial.println(" mm");
    if (vl53Distances[1] < VL53_CRIT_RIGHT && vl53Distances[1] > 0) {
      return MUX_CRITICAL_RIGHT;
    }
  }

  return MUX_CLEAR;
}
