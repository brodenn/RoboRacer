#include "mux.h"
#include <WebSerial.h>

// VL53L4CD-sensorer via MUX på standard I2C (Wire)
VL53L4CD vl53_left(&Wire, -1);
VL53L4CD vl53_right(&Wire, -1);
uint16_t vl53Distances[2] = {0};

// Initiera MUX och VL53L4CD-sensorerna
void initMuxAndSensors() {
  selectMuxChannel(0);
  vl53_left.VL53L4CD_Off(); vl53_left.VL53L4CD_On();
  int leftStatus = vl53_left.InitSensor();
  Serial.print("VL53 Left InitSensor() return: "); Serial.println(leftStatus);
  if (leftStatus == 0) {
    Serial.println("✅ VL53 Left init OK");
    vl53_left.VL53L4CD_SetDetectionThresholds(0, 300, 2);  // Trigger om objekt är utanför [0, 300] → d.v.s. <300 mm
  } else {
    Serial.println("❌ VL53 Left init FAIL");
  }

  selectMuxChannel(7);
  vl53_right.VL53L4CD_Off(); vl53_right.VL53L4CD_On();
  int rightStatus = vl53_right.InitSensor();
  Serial.print("VL53 Right InitSensor() return: "); Serial.println(rightStatus);
  if (rightStatus == 0) {
    Serial.println("✅ VL53 Right init OK");
    vl53_right.VL53L4CD_SetDetectionThresholds(0, 300, 2);
  } else {
    Serial.println("❌ VL53 Right init FAIL");
  }
}

// Välj MUX-kanal
void selectMuxChannel(uint8_t channel) {
  Wire.beginTransmission(MUX_ADDR);
  Wire.write(1 << channel);
  byte err = Wire.endTransmission();
  Serial.print("→ Väljer mux kanal "); Serial.print(channel);
  if (err == 0) Serial.println(" ✅");
  else Serial.println(" ❌ (Transmission error)");
}

// Läs båda VL53-sensorerna via mux
void readVL53Sensors() {
  // Vänster sensor
  selectMuxChannel(0);
  vl53_left.VL53L4CD_StartRanging();
  uint8_t ready = 0;
  while (!ready) { vl53_left.VL53L4CD_CheckForDataReady(&ready); delay(1); }
  VL53L4CD_Result_t resL;
  vl53_left.VL53L4CD_GetResult(&resL);
  vl53_left.VL53L4CD_ClearInterrupt();
  vl53_left.VL53L4CD_StopRanging();
  vl53Distances[0] = resL.distance_mm;

  // Höger sensor
  selectMuxChannel(7);
  vl53_right.VL53L4CD_StartRanging();
  ready = 0;
  while (!ready) { vl53_right.VL53L4CD_CheckForDataReady(&ready); delay(1); }
  VL53L4CD_Result_t resR;
  vl53_right.VL53L4CD_GetResult(&resR);
  vl53_right.VL53L4CD_ClearInterrupt();
  vl53_right.VL53L4CD_StopRanging();
  vl53Distances[1] = resR.distance_mm;
}

// Läs en specifik VL53-sensor via MUX (för interrupt-hantering)
void readVL53Sensor(uint8_t muxChannel) {
  selectMuxChannel(muxChannel);
  VL53L4CD &sensor = (muxChannel == 0) ? vl53_left : vl53_right;

  sensor.VL53L4CD_StartRanging();
  uint8_t ready = 0;
  while (!ready) {
    sensor.VL53L4CD_CheckForDataReady(&ready);
    delay(1);
  }

  VL53L4CD_Result_t result;
  sensor.VL53L4CD_GetResult(&result);
  sensor.VL53L4CD_ClearInterrupt();
  sensor.VL53L4CD_StopRanging();

  if (muxChannel == 0) vl53Distances[0] = result.distance_mm;
  else                 vl53Distances[1] = result.distance_mm;
}
