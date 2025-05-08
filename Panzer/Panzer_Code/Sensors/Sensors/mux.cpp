#include <Wire.h>
#include "mux.h"

// Välj aktiv kanal på MUX: kanal 0–7 (1-hot encoded)
void selectMuxChannel(uint8_t channel) {
  Wire.beginTransmission(MUX_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
}
