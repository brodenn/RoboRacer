#include "loop_logic.h"
#include "opt.h"
#include "mux.h"
#include "obstacle.h"
#include "tft.h"
#include <WebSerial.h>


void loopLogic() {
  static unsigned long lastLogicTime = 0;
  static unsigned long lastDisplayTime = 0;
  unsigned long now = millis();

  // === Sensoravläsning + hinderlogik ===
  if (now - lastLogicTime >= 50) {  // 20 Hz
    lastLogicTime = now;
    readOPTSensors();
    readVL53Sensors();
    handleObstacleAvoidance();
  }

  // === TFT och Serial-monitor ===
  if (now - lastDisplayTime >= 500) {  // 2 Hz
    lastDisplayTime = now;
    updateTFT();

    Serial.print("VL53 Left: ");
    Serial.print(vl53Distances[0]);
    Serial.print(" mm   Right: ");
    Serial.print(vl53Distances[1]);
    Serial.print(" mm   OPT: ");
    Serial.print(optDistances[0]); Serial.print(", ");
    Serial.print(optDistances[1]); Serial.print(", ");
    Serial.println(optDistances[2]);
  }
}
