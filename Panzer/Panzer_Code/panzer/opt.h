// opt.h
#ifndef OPT_H
#define OPT_H

#include <Arduino.h>
#include "OPT3101.h"

extern OPT3101 opt;
extern uint16_t optDistances[3];  // 0 = center, 1 = left, 2 = right
extern uint16_t optAmplitudes[3];

void initOPT();
void readOPTSensors();
void readOPTFront();

#endif
