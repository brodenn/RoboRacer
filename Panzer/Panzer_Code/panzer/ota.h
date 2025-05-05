// ota.h
#ifndef OTA_H
#define OTA_H

#include <ESPAsyncWebServer.h>

extern AsyncWebServer server;

void setupWiFiAndOTA();
const char* getControlHTML();

#endif
