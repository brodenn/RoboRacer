#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include <ArduinoOTA.h>
#include <WebSerial.h>

#include "ota.h"
#include "mux.h"
#include "opt.h"
#include "tft.h"
#include "evade.h"
#include "steering.h"

// === Motorer ===
Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x60);
Adafruit_DCMotor* motorLeft;
Adafruit_DCMotor* motorRight;

unsigned long avoidUntil = 0;
bool isAutoMode = true;
bool isWebSocketDataEnabled = true;

void setup() {
  Serial.begin(115200);
  delay(1000);

  setupWiFiAndOTA();       // WiFi och OTA
  Wire.begin();            // SDA = GPIO5, SCL = GPIO6

  // Init motor shield
  if (!AFMS.begin()) {
    Serial.println("❌ Kunde inte starta Motor FeatherWing!");
    while (1);
  }
  Serial.println("✅ Motor FeatherWing OK");

  motorLeft  = AFMS.getMotor(3);
  motorRight = AFMS.getMotor(4);

  initOPT();
  initVL53Sensors();
  initTFT();
}

void loop() {
  ArduinoOTA.handle();

  // === Sensoravläsning ===
  readOPTSensors();
  checkVL53Obstacles();
  updateTFT();

  // === WebSocket & WebSerial-uppdatering ===
  static unsigned long lastSend = 0;
  static unsigned long lastCleanup = 0;
  const unsigned long sendInterval = 200;  // Ändra till 100 om du vill testa igen
  const unsigned long cleanupInterval = 1000;

  if (millis() - lastSend >= sendInterval) {
    lastSend = millis();

    if (isWebSocketDataEnabled) {
      int pwmL_pct = map(currentPWM_L, 0, 255, 0, 100);
      int pwmR_pct = map(currentPWM_R, 0, 255, 0, 100);
      if (dirLeftForward) pwmL_pct *= -1;
      if (!dirRightForward) pwmR_pct *= -1;

      String msg = "sensor:" +
                   String(vl53Distances[0]) + "," +
                   String(optDistances[0]) + "," +
                   String(optDistances[1]) + "," +
                   String(optDistances[2]) + "," +
                   String(vl53Distances[1]) + "," +
                   String(currentPWM_L) + "," +
                   String(currentPWM_R) + "," +
                   String(pwmL_pct) + "," +
                   String(pwmR_pct);

      ws.textAll(msg);
      WebSerial.println(msg);

      Serial.print("PWM L: ");
      Serial.print(pwmL_pct);
      Serial.print("%  |  PWM R: ");
      Serial.print(pwmR_pct);
      Serial.println("%");
    }
  }

  // === WebSocket-klient-rensning ===
  if (millis() - lastCleanup >= cleanupInterval) {
    lastCleanup = millis();
    ws.cleanupClients();
  }

  // === AUTO-läge ===
  if (isAutoMode) {
    bool frontBlocked = optDistances[1] < 300;
    bool sideBlockedL = vl53Distances[0] > 0 && vl53Distances[0] < 100;
    bool sideBlockedR = vl53Distances[1] > 0 && vl53Distances[1] < 100;

    Serial.println("🔁 AUTO-läge körs");

    if (frontBlocked || sideBlockedL || sideBlockedR) {
      performAvoidance(
        vl53Distances[0], optDistances[0], optDistances[2], vl53Distances[1], optDistances[1]
      );
    } else {
      correctCourse();
    }
  }
}
