#ifndef WIFI_SETUP_H
#define WIFI_SETUP_H

#include <WiFi.h>
#include <WiFiUdp.h>

extern const char* ssid;
extern const char* password;
extern const char* laptop_ip;
extern const int udp_port;

extern WiFiUDP udp;
extern SemaphoreHandle_t wifiSemaphore;

void setupWiFi();

#endif
