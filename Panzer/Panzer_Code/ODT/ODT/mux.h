#ifndef MUX_H
#define MUX_H

#include <Arduino.h>

#define MUX_ADDR 0x70  // I2C-adress för multiplexer

// === Tröskelvärden för kritiskt avstånd (mm) ===
#define VL53_CRIT_LEFT  100
#define VL53_CRIT_RIGHT 100

// === Returnstatus för hinderkontroll ===
enum MuxStatus {
  MUX_CLEAR,
  MUX_CRITICAL_LEFT,
  MUX_CRITICAL_RIGHT
};

// === Globala mätvärden [0]=vänster, [1]=höger ===
extern uint16_t vl53Distances[2];

// === Funktioner ===
void selectMuxChannel(uint8_t channel);
void initVL53Sensors();
MuxStatus checkVL53Obstacles();

#endif
