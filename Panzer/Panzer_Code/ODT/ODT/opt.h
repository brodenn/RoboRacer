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

extern OPT3101 opt;

void initOPT();
OptStatus checkOptObstacles();

#endif
