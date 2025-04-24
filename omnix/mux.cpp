#include "mux.h"
#include "globals.h"

// --- Updated MUX Lookup Table ---
// Sensor order: L, F, FR, FL, BL, R, B, BR
MuxChannel muxMap[NUM_SENSORS] = {
    {MUX1_ADDR, 0}, // Sensor 0: Left
    {MUX1_ADDR, 1}, // Sensor 1: Front
    {MUX1_ADDR, 2}, // Sensor 2: Front-Right
    {MUX1_ADDR, 3}, // Sensor 3: Front-Left
    {MUX1_ADDR, 4}, // Sensor 4: Back-Left
    {MUX1_ADDR, 5}, // Sensor 5: Right
    {MUX1_ADDR, 6}, // Sensor 6: Back
    {MUX1_ADDR, 7}  // Sensor 7: Back-Right
};

void setupMux() {
    Wire.begin();
    Wire.setClock(400000);
    Serial.println("✅ MUX initialized");
}

bool waitForMuxReady(uint8_t addr) {
    for (int attempts = 0; attempts < 5; attempts++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) return true;
        delayMicroseconds(50);
    }
    Serial.printf("⚠️ MUX 0x%02X not responding\n", addr);
    return false;
}

// ⚡ Select MUX channel (no I2C locking inside!)
void selectMuxChannel(uint8_t sensorIndex) {
    if (sensorIndex >= NUM_SENSORS) return;

    const MuxChannel& map = muxMap[sensorIndex];

    Wire.beginTransmission(map.muxAddr);
    Wire.write(1 << map.channel);
    Wire.endTransmission();
    waitForMuxReady(map.muxAddr);

    delayMicroseconds(500);  // Allow signal to settle
}

// ⚡ Deselect all channels (no I2C lock inside!)
void deselectAllMux() {
    Wire.beginTransmission(MUX1_ADDR);
    Wire.write(0x00);  // Disable all channels
    Wire.endTransmission();
}

void selectMuxINA60() {
    selectMuxChannelDirect(MUX1_ADDR, INA60_MUX_CHANNEL);
}

void selectMuxINA61() {
    selectMuxChannelDirect(MUX1_ADDR, INA61_MUX_CHANNEL);
}

// ⚡ Direct channel select (also without locking)
void selectMuxChannelDirect(uint8_t muxAddr, uint8_t channel) {
    Wire.beginTransmission(muxAddr);
    Wire.write(1 << channel);
    Wire.endTransmission();
    waitForMuxReady(muxAddr);
    delayMicroseconds(300);
}
