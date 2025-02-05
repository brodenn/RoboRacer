#include <Adafruit_GFX.h>      // Core graphics library
#include <Adafruit_ST7789.h>   // ST7789 driver library
#include <SPI.h>

// Define TFT control pins for Adafruit ESP32-S3 TFT Feather
#define TFT_CS        7
#define TFT_RST      40
#define TFT_DC       39
#define TFT_BACKLIGHT 45

// Initialize the ST7789 display
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);
  Serial.println("TFT Display Test");

  // Initialize the TFT display
  tft.init(135, 240);  // Width and height of the display
  tft.setRotation(3);  // Set display orientation (try 0, 1, 2, or 3)

  // Turn on the backlight
  pinMode(TFT_BACKLIGHT, OUTPUT);
  digitalWrite(TFT_BACKLIGHT, HIGH);

  // Clear the screen and fill with black
  tft.fillScreen(ST77XX_BLACK);

  // Set text properties
  tft.setTextColor(ST77XX_WHITE);  // White text
  tft.setTextSize(2);              // Double size text
  tft.setCursor(10, 10);           // Set starting point

  // Display text
  tft.println("Hello, World!");
  tft.println("ESP32-S3 TFT");
}

void loop() {
  // Invert display colors for a simple animation effect
  tft.invertDisplay(true);
  delay(500);
  tft.invertDisplay(false);
  delay(500);
}
