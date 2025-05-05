#ifndef TFT_H
#define TFT_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// TFT-pinnar
#define TFT_CS    7
#define TFT_RST   40
#define TFT_DC    39
#define TFT_BL    45

extern Adafruit_ST7789 tft;

// Initierar skärmen och ritar FOV-etiketter
void initTFT();

// Ritar själva FOV-layouten (linjer + etiketter)
void drawFOVSketch();

// Skriver ut sensordata på skärmen
void drawSensorData(
  uint16_t leftVL53, uint16_t frontOPT, uint16_t rightVL53,
  uint16_t leftF_OPT, uint16_t frontF_OPT, uint16_t rightF_OPT
);

// Uppdaterar hela displayen
void updateTFT();

#endif
