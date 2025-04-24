#include "wifi_setup.h"
#include <Arduino.h>

void setupWiFi() {
    WiFi.softAP(ssid, password);
    udp.begin(udp_port);
    wifiSemaphore = xSemaphoreCreateMutex();
    Serial.println("✅ WiFi Setup Complete!");
}
