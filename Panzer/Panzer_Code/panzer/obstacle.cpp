#include "obstacle.h"
#include "motors.h"
#include "mux.h"
#include "opt.h"
#include <WebSerial.h>

#define CRITICAL_DISTANCE 300

extern uint16_t vl53Distances[2]; // [0] = left, [1] = right
extern uint16_t optDistances[3];  // [0,1,2] = front sensors

void handleObstacleAvoidance() {
  bool leftClose = vl53Distances[0] < CRITICAL_DISTANCE;
  bool rightClose = vl53Distances[1] < CRITICAL_DISTANCE;
  bool frontClose = (optDistances[0] < CRITICAL_DISTANCE ||
                     optDistances[1] < CRITICAL_DISTANCE ||
                     optDistances[2] < CRITICAL_DISTANCE);

  WebSerial.print("VL53 Left: "); WebSerial.println(vl53Distances[0]);
  WebSerial.print("VL53 Right: "); WebSerial.println(vl53Distances[1]);
  WebSerial.print("OPT Front: ");
  WebSerial.print(optDistances[0]); WebSerial.print(", ");
  WebSerial.print(optDistances[1]); WebSerial.print(", ");
  WebSerial.println(optDistances[2]);

  if (frontClose) {
    WebSerial.println("🟥 Hinder fram");

    if (!leftClose && rightClose) {
      WebSerial.println("🔄 Svänger vänster (höger blockerad)");
      turnLeft();
    } else if (leftClose && !rightClose) {
      WebSerial.println("🔄 Svänger höger (vänster blockerad)");
      turnRight();
    } else {
      WebSerial.println("🛑 Båda sidor blockerade – stoppar");
      stopOrReverse();
    }
  } else if (leftClose && !rightClose) {
    WebSerial.println("↪ Kurskorrigering höger");
    slightRight();
  } else if (rightClose && !leftClose) {
    WebSerial.println("↩ Kurskorrigering vänster");
    slightLeft();
  } else {
    WebSerial.println("✅ Fri väg – kör framåt");
    moveForward();
  }
}
