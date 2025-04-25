#include "tft.h"
#include "mux.h"
#include "opt.h"

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

void initTFT() {
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  tft.init(135, 240);
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(10, 10);
  tft.println("TFT initierad!");

  // FOV-grafik och etiketter
  tft.setCursor(20, 0);  tft.setTextColor(ST77XX_RED);   tft.print("<-");
  tft.setCursor(110, 0); tft.setTextColor(ST77XX_GREEN); tft.print("^");
  tft.setCursor(200, 0); tft.setTextColor(ST77XX_BLUE);  tft.print("->");

  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(90, 20); tft.print("mm:");
  tft.setCursor(90, 60); tft.print("Amp:");

  drawFOVSketch();
}

void drawFOVSketch() {
  tft.drawLine(120, 120, 40, 200, ST77XX_RED);
  tft.drawLine(120, 120, 120, 200, ST77XX_GREEN);
  tft.drawLine(120, 120, 200, 200, ST77XX_BLUE);

  tft.setCursor(15, 205);  tft.setTextColor(ST77XX_RED);   tft.print("LEFT F");
  tft.setCursor(105, 205); tft.setTextColor(ST77XX_GREEN); tft.print("FRONT");
  tft.setCursor(185, 205); tft.setTextColor(ST77XX_BLUE);  tft.print("RIGHT F");
}

void drawSensorData(uint16_t leftVL53, uint16_t frontOPT, uint16_t rightVL53,
                    uint16_t leftF_OPT, uint16_t frontF_OPT, uint16_t rightF_OPT) {
  // Rensa gamla värden
  tft.fillRect(20, 40, 60, 20, ST77XX_BLACK);
  tft.fillRect(100, 40, 60, 20, ST77XX_BLACK);
  tft.fillRect(190, 40, 60, 20, ST77XX_BLACK);

  tft.fillRect(20, 80, 60, 20, ST77XX_BLACK);
  tft.fillRect(100, 80, 60, 20, ST77XX_BLACK);
  tft.fillRect(190, 80, 60, 20, ST77XX_BLACK);

  tft.fillRect(20, 120, 60, 20, ST77XX_BLACK);
  tft.fillRect(190, 120, 60, 20, ST77XX_BLACK);

  // mm
  tft.setCursor(20, 40);  tft.setTextColor(ST77XX_RED);   tft.print(leftF_OPT);
  tft.setCursor(100, 40); tft.setTextColor(ST77XX_GREEN); tft.print(frontOPT);
  tft.setCursor(190, 40); tft.setTextColor(ST77XX_BLUE);  tft.print(rightF_OPT);

  // amplitud
  tft.setCursor(20, 80);  tft.setTextColor(ST77XX_RED);   tft.print(optAmplitudes[0]);
  tft.setCursor(100, 80); tft.setTextColor(ST77XX_GREEN); tft.print(optAmplitudes[1]);
  tft.setCursor(190, 80); tft.setTextColor(ST77XX_BLUE);  tft.print(optAmplitudes[2]);

  // VL53-värden
  tft.setCursor(20, 120);  tft.setTextColor(ST77XX_WHITE); tft.print(leftVL53);
  tft.setCursor(190, 120); tft.setTextColor(ST77XX_WHITE); tft.print(rightVL53);
}

void updateTFT() {
  drawSensorData(
    vl53Distances[0],          // VL53 Left
    optDistances[1],           // OPT Front
    vl53Distances[1],          // VL53 Right
    optDistances[0],           // OPT Left
    optDistances[1],           // OPT Front again (distance)
    optDistances[2]            // OPT Right
  );
}
