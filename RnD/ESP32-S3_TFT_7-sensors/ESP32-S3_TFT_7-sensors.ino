#include <Wire.h>
#include <vl53l4cd_class.h>
#include <Adafruit_GFX.h>      // Core graphics library
#include <Adafruit_ST7789.h>   // ST7789 driver library
#include <SPI.h>

// TFT display configuration
#define TFT_CS        7
#define TFT_RST      40
#define TFT_DC       39
#define TFT_BACKLIGHT 45

// I2C multiplexer address
#define TCA9548A_ADDRESS 0x70
#define SENSOR_COUNT 7  // Number of VL53L4CD sensors

// VL53L4CD sensors array
VL53L4CD sensors[SENSOR_COUNT] = {
    VL53L4CD(&Wire, -1),
    VL53L4CD(&Wire, -1),
    VL53L4CD(&Wire, -1),
    VL53L4CD(&Wire, -1),
    VL53L4CD(&Wire, -1),
    VL53L4CD(&Wire, -1),
    VL53L4CD(&Wire, -1)
};

// Initialize TFT display
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// Function to select the active channel on the TCA9548A multiplexer
void selectChannel(uint8_t channel) {
    if (channel > 7) {
        Serial.println("⚠️ Invalid channel selected!");
        return;
    }
    Wire.beginTransmission(TCA9548A_ADDRESS);
    Wire.write(1 << channel);
    Wire.endTransmission();
    delay(10);
}

void setup() {
    // Serial and I2C initialization
    Serial.begin(115200);
    Wire.begin();
    Wire.setClock(400000);  // 400 kHz I2C speed
    delay(2000);

    // TFT display initialization
    tft.init(135, 240);  // Initialize TFT with width and height
    tft.setRotation(3);
    pinMode(TFT_BACKLIGHT, OUTPUT);
    digitalWrite(TFT_BACKLIGHT, HIGH);
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);

    tft.setCursor(10, 10);
    tft.println("Initializing Sensors...");
    Serial.println("Initializing Sensors...");

    // Initialize VL53L4CD sensors
    for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
        selectChannel(i);
        if (sensors[i].begin() != 0) {
            Serial.print("⚠️ Sensor ");
            Serial.print(i + 1);
            Serial.println(" not found!");
            tft.setCursor(10, 30 + (i * 20));
            tft.printf("Ch %d: Error", i + 1);
        } else {
            Serial.print("✅ Sensor ");
            Serial.print(i + 1);
            Serial.println(" initialized.");
            sensors[i].VL53L4CD_Off();
            sensors[i].InitSensor();
            sensors[i].VL53L4CD_SetRangeTiming(200, 0);
            sensors[i].VL53L4CD_StartRanging();
        }
    }

    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(10, 10);
    tft.println("Sensors Ready!");
    delay(2000);
    tft.fillScreen(ST77XX_BLACK);
}

void loop() {
    VL53L4CD_Result_t results;
    uint8_t NewDataReady = 0;
    unsigned long totalStartTime = millis();

    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(10, 10);
    tft.println("Sensor Readings:");

    for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
        selectChannel(i);
        sensors[i].VL53L4CD_CheckForDataReady(&NewDataReady);
        if (NewDataReady != 0) {
            sensors[i].VL53L4CD_GetResult(&results);
            sensors[i].VL53L4CD_ClearInterrupt();
            Serial.print("Sensor ");
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(results.distance_mm);
            Serial.println(" mm");

            tft.setCursor(10, 30 + (i * 20));
            tft.printf("Ch %d: %d mm", i + 1, results.distance_mm);
        } else {
            Serial.print("⚠️ No new data from sensor ");
            Serial.println(i + 1);

            tft.setCursor(10, 30 + (i * 20));
            tft.printf("Ch %d: No Data", i + 1);
        }
    }

    unsigned long totalEndTime = millis();
    unsigned long totalElapsedTime = totalEndTime - totalStartTime;

    Serial.print("Total time to read all sensors: ");
    Serial.print(totalElapsedTime);
    Serial.println(" ms");

    tft.setCursor(10, 180);
    tft.printf("Total time: %lu ms", totalElapsedTime);

    delay(500);  // Wait between measurement cycles
}
