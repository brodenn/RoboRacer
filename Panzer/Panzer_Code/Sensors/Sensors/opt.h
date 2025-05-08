#ifndef OPT_H
#define OPT_H

#include <Wire.h>
#include "OPT3101.h"

// Endast deklarationer (extern)
extern OPT3101 opt;
extern uint16_t optDistances[3];
extern uint16_t optAmplitudes[3];

// Funktioner
void initOPT();
void readOPTSensors();

#endif
