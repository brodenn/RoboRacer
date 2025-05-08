#ifndef MUX_H
#define MUX_H

#include <Arduino.h>

// I2C-adressen till din multiplexer (kanal 0x70 typiskt för Adafruit-modellen)
#define MUX_ADDR 0x70

// Funktion för att välja en kanal på MUX
void selectMuxChannel(uint8_t channel);

#endif
