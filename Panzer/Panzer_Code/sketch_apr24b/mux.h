// mux.h
#ifndef MUX_H
#define MUX_H

#include <Wire.h>
#include "vl53l4cd_class.h"

#define MUX_ADDR 0x71
#define VL53_CHANNEL_LEFT 0
#define VL53_CHANNEL_RIGHT 7

extern TwoWire I2C_STEMMA;
extern VL53L4CD vl53_left;
extern VL53L4CD vl53_right;
extern uint16_t vl53Distances[2];

void initMuxAndSensors();
void readVL53Sensors();
void selectMuxChannel(uint8_t channel);

#endif
