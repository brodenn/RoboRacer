#include "xshut.h"

// --- Actual XSHUT pin mapping (defined once here) ---
const uint8_t XSHUT_GPIO[NUM_SENSORS] = {
    25,  // SENSOR_FRONT        (A1 / GPIO25)
     4,  // SENSOR_FRONT_LEFT   (A5 / GPIO4)
    32,  // SENSOR_LEFT         (D32 / GPIO32)
    19,  // SENSOR_BACK_LEFT    (MOSI / GPIO19)
     5,  // SENSOR_BACK         (SCK / GPIO5)
    26,  // SENSOR_BACK_RIGHT   (A0 / GPIO26)
    33,  // SENSOR_RIGHT        (D33 / GPIO33)
    27   // SENSOR_FRONT_RIGHT  (D27 / GPIO27)
};


// --- Control Functions ---
void setupXshut() {
    for (uint8_t i = 0; i < NUM_SENSORS; i++) {
        pinMode(XSHUT_GPIO[i], OUTPUT);
        digitalWrite(XSHUT_GPIO[i], HIGH); // Default ON
    }
}

void sensorPowerOff(uint8_t sensorIndex) {
    if (sensorIndex < NUM_SENSORS) {
        digitalWrite(XSHUT_GPIO[sensorIndex], LOW);
    }
}

void sensorPowerOn(uint8_t sensorIndex) {
    if (sensorIndex < NUM_SENSORS) {
        digitalWrite(XSHUT_GPIO[sensorIndex], HIGH);
    }
}

void xshutPulse(uint8_t sensorIndex) {
    if (sensorIndex < NUM_SENSORS) {
        sensorPowerOff(sensorIndex);
        delay(5);
        sensorPowerOn(sensorIndex);
        delay(20);
    }
}
