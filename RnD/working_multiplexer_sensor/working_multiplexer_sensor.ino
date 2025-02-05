#include <Wire.h>
#include <vl53l4cd_class.h>
#include <Adafruit_GFX.h>      // Core graphics library
#include <Adafruit_ST7789.h>   // ST7789 driver library
#include <SPI.h>

// Define TFT control pins for Adafruit ESP32-S3 TFT Feather
#define TFT_CS        7
#define TFT_RST      40
#define TFT_DC       39
#define TFT_BACKLIGHT 45

// I2C multiplexer address
#define TCA9548A_ADDRESS 0x70

// Initialize VL53L4CD sensor instance
VL53L4CD sensor(&Wire, -1);

// Initialize the ST7789 display
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// Function to select a channel on the I2C multiplexer
void tcaSelect(uint8_t channel) {
  if (channel > 7) return;
  Wire.beginTransmission(TCA9548A_ADDRESS);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

// Setup function
void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000);  // 400 kHz I2C speed

  // Initialize the TFT display
  tft.init(135, 240);  // Width and height of the display
  tft.setRotation(3);  // Set display orientation
  pinMode(TFT_BACKLIGHT, OUTPUT);
  digitalWrite(TFT_BACKLIGHT, HIGH);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);

  // Initialize sensors on all 7 channels
  for (uint8_t i = 0; i < 7; i++) {
    tcaSelect(i);
    delay(100);
    if (sensor.begin() != 0) {
      Serial.print("⚠️ Sensor on channel ");
      Serial.print(i);
      Serial.println(" not found!");
    } else {
      Serial.print("✅ Sensor on channel ");
      Serial.print(i);
      Serial.println(" initialized.");
      sensor.VL53L4CD_SetRangeTiming(200, 0);  // 200 ms timing budget
      sensor.VL53L4CD_StartRanging();
    }
  }

  // Display initialization message
  tft.setCursor(10, 10);
  tft.println("Sensors Ready!");
  delay(2000);
  tft.fillScreen(ST77XX_BLACK);
}

// Loop function
void loop() {
  VL53L4CD_Result_t results;
  uint8_t yOffset = 10;  // Y offset for displaying text

  tft.fillScreen(ST77XX_BLACK);  // Clear screen
  tft.setCursor(10, yOffset);
  tft.println("Sensor Readings:");

  unsigned long startTime = millis();  // Start time for timing

  for (uint8_t i = 0; i < 7; i++) {
    tcaSelect(i);  // Select multiplexer channel
    delay(50);     // Allow time for channel switching

    if (sensor.VL53L4CD_GetResult(&results) == 0) {
      Serial.print("Sensor on channel ");
      Serial.print(i);
      Serial.print(": ");
      Serial.print(results.distance_mm);
      Serial.println(" mm");

      // Display sensor data on TFT
      tft.setCursor(10, yOffset + (i + 1) * 20);
      tft.printf("Ch %d: %d mm", i, results.distance_mm);
    } else {
      Serial.print("⚠️ Error reading sensor on channel ");
      Serial.println(i);

      // Display error message on TFT
      tft.setCursor(10, yOffset + (i + 1) * 20);
      tft.printf("Ch %d: Error", i);
    }
  }

  unsigned long endTime = millis();  // End time for timing
  Serial.print("Total time for reading all sensors: ");
  Serial.print(endTime - startTime);
  Serial.println(" ms");

  // Display total read time on TFT
  tft.setCursor(10, yOffset + 160);
  tft.printf("Total time: %lu ms", endTime - startTime);

  delay(500);  // Wait before the next loop
}
