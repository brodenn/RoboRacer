#ifndef MUX_H
#define MUX_H

#include <Arduino.h>
#include <Wire.h>

// --- MUX Addresses ---
#define MUX1_ADDR 0x71   // First multiplexer (sensor hub)
#define MUX2_ADDR 0x70   // Second multiplexer (extended sensors)

// --- Number of Sensors ---
#define NUM_SENSORS 8

// --- INA219 Channels ---
#define INA60_MUX_CHANNEL 5   // INA60 is on channel 5 of MUX1
#define INA61_MUX_CHANNEL 6   // INA61 is on channel 6 of MUX1

// --- MUX Channel Mapping ---
struct MuxChannel {
    uint8_t muxAddr;
    uint8_t channel;
};

// --- Functions ---
void setupMux();

// --- Sensor Selection ---
void selectMuxChannel(uint8_t sensorIndex);  // Select sensor based on index 0..7

// --- INA219 Selection ---
void selectMuxINA60();   // Select INA60
void selectMuxINA61();   // Select INA61

// --- Utility ---
void deselectAllMux();
void selectMuxChannelDirect(uint8_t muxAddr, uint8_t channel);  // Optional low-level selector

#endif
