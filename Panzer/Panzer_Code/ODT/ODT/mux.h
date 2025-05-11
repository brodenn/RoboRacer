#ifndef MUX_H
#define MUX_H

#include <Arduino.h>
#define MUX_ADDR 0x70

enum MuxStatus {
  MUX_CLEAR,
  MUX_CRITICAL_LEFT,
  MUX_CRITICAL_RIGHT
};

void selectMuxChannel(uint8_t channel);
void initVL53Sensors();              // ⬅️ ny init-funktion
extern uint16_t vl53Distances[2];  // [0] = left, [1] = right

MuxStatus checkVL53Obstacles();

#endif
