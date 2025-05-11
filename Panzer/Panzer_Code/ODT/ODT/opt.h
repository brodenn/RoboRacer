#ifndef OPT_H
#define OPT_H

#include <Wire.h>
#include "OPT3101.h"  // Pololu-klassen

// === Kritisk riktning för Panzer-logik ===
// CH 0 = HÖGER
// CH 1 = FRAM
// CH 2 = VÄNSTER

enum OptStatus {
  OPT_CLEAR,
  OPT_CRITICAL_FRONT,
  OPT_CRITICAL_LEFT,
  OPT_CRITICAL_RIGHT
};

// Globala instanser och funktioner
extern OPT3101 opt;

void initOPT();              // Initierar OPT3101 med testade värden
OptStatus checkOptObstacles();  // Kör sample() för varje kanal och returnerar status

#endif
