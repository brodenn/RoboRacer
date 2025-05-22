#ifndef OPT_H
#define OPT_H

#include <Wire.h>
#include "OPT3101.h"

enum OptStatus {
  OPT_CLEAR,
  OPT_CRITICAL_FRONT,
  OPT_CRITICAL_LEFT,
  OPT_CRITICAL_RIGHT
};

// Externa variabler
extern OPT3101 opt;
extern uint16_t optDistances[3];  // [0]=LEFT, [1]=FRONT, [2]=RIGHT

// Funktioner
void initOPT();
void readOPTSensors();
OptStatus checkOptObstacles();

#endif
