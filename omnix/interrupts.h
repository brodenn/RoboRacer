#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <Arduino.h>
#include "globals.h" 
extern volatile bool sensorTriggered[NUM_SENSORS];  // ✅ extern means "defined elsewhere"

void setupSensorInterrupts();

#endif
