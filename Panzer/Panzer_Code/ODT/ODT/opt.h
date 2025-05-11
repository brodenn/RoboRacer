#ifndef OPT_H
#define OPT_H

#include <Arduino.h>
#include "OPT3101.h"

enum OptStatus {
  OPT_CLEAR,
  OPT_CRITICAL_FRONT,
  OPT_CRITICAL_LEFT,
  OPT_CRITICAL_RIGHT
};

void initOPT();
OptStatus checkOptObstacles();

// 👇 Lägg till detta för global åtkomst i .ino
extern OPT3101 opt;
extern uint16_t optDistances[3];

#endif
