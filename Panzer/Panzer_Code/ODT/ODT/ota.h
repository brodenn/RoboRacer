#ifndef OTA_H
#define OTA_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

// Extern WebSocket och serverobjekt
extern AsyncWebServer server;
extern AsyncWebSocket ws;
extern bool isWebSocketDataEnabled;

// Funktion för att initiera WiFi, OTA och webserver
void setupWiFiAndOTA();

#endif // OTA_H
