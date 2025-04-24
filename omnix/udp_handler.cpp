#include "udp_comm.h"
#include "globals.h"
#include "params.h"
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include "mux.h"
#include "controller.h"
#include <Adafruit_INA219.h>

extern Adafruit_INA219 ina60;
extern Adafruit_INA219 ina61;

bool inaContinuous = false;
bool modeChanged = false;

void handleUdpParams() {
    int packetSize = udp.parsePacket();
    if (packetSize <= 0) return;

    char buffer[512];
    int len = udp.read(buffer, sizeof(buffer) - 1);
    if (len > 0) buffer[len] = 0;

    Serial.printf("📩 [%lu] Received UDP (%d bytes): %s\n", millis(), len, buffer);

    StaticJsonDocument<1024> doc;
    DeserializationError err = deserializeJson(doc, buffer);
    if (err) {
        Serial.printf("❌ JSON Parse Error: %s\n", err.c_str());
        return;
    }

    // Load parameters
    params.loadFromJson(doc);
    modeChanged = false;

    if (doc.containsKey("command")) {
        const char* cmd = doc["command"];

        if (strcmp(cmd, "start") == 0) {
            reversing = false;
            motorsEnabled = true;
            Serial.println("🟢 Start command received");

        } else if (strcmp(cmd, "stop") == 0) {
            motorsEnabled = false;
            target_m1 = target_m2 = target_m3 = target_m4 = 0;
            current_m1 = current_m2 = current_m3 = current_m4 = 0;
            motor1->run(RELEASE);
            motor2->run(RELEASE);
            motor3->run(RELEASE);
            motor4->run(RELEASE);
            Serial.println("🔴 Stop command received");

        } else if (strcmp(cmd, "mode") == 0 && doc.containsKey("mode")) {
            const char* requestedMode = doc["mode"];
            if (strcmp(requestedMode, "controller") == 0) {
                controlMode = MODE_CONTROLLER;
                modeChanged = true;
                Serial.println("🎮 Switched to CONTROLLER mode");
            } else if (strcmp(requestedMode, "autonomous") == 0) {
                controlMode = MODE_AUTONOMOUS;
                modeChanged = true;
                Serial.println("🤖 Switched to AUTONOMOUS mode");
            } else {
                Serial.printf("⚠️ Unknown mode: %s\n", requestedMode);
            }

        } else if (strcmp(cmd, "ina_auto_read") == 0 && doc.containsKey("state")) {
            inaContinuous = doc["state"];
            Serial.printf("🔄 Auto INA Read: %s\n", inaContinuous ? "ENABLED" : "DISABLED");

            StaticJsonDocument<128> reply;
            reply["ack"] = "ina_auto_read";
            reply["state"] = inaContinuous;

            char json[128];
            serializeJson(reply, json);
            udp.beginPacket(laptop_ip, udp_port);
            udp.write((const uint8_t*)json, strlen(json));
            udp.endPacket();

        } else if (strcmp(cmd, "ina_read") == 0) {
            float current_ina60 = 0.0f;
            float current_ina61 = 0.0f;

            if (xSemaphoreTake(i2cBusyWire0, pdMS_TO_TICKS(20)) == pdTRUE) {
                deselectAllMux();
                selectMuxINA60();
                //delayMicroseconds(300);  // Can be reduced or replaced if async
                current_ina60 = ina60.getCurrent_mA();

                deselectAllMux();
                selectMuxINA61();
                //delayMicroseconds(300);
                current_ina61 = ina61.getCurrent_mA();

                xSemaphoreGive(i2cBusyWire0);
            } else {
                Serial.println("⚠️ I2C busy: Could not read INA219");
            }

            StaticJsonDocument<256> reply;
            reply["ack"] = "ina_data";
            reply["ina60"] = current_ina60;
            reply["ina61"] = current_ina61;

            char json[256];
            serializeJson(reply, json);
            udp.beginPacket(laptop_ip, udp_port);
            udp.write((const uint8_t*)json, strlen(json));
            udp.endPacket();

            Serial.printf("✅ Manual INA: INA60 = %.2f mA | INA61 = %.2f mA\n", current_ina60, current_ina61);

        } else {
            Serial.printf("⚠️ Unknown command: %s\n", cmd);
        }
    }

    // ACK param update
    StaticJsonDocument<128> ack;
    ack["ack"] = "params_updated";
    char jsonAck[128];
    serializeJson(ack, jsonAck);
    udp.beginPacket(laptop_ip, udp_port);
    udp.write((const uint8_t*)jsonAck, strlen(jsonAck));
    udp.endPacket();

    // ACK mode change
    if (modeChanged) {
        StaticJsonDocument<128> reply;
        reply["ack"] = "mode";
        reply["mode"] = (controlMode == MODE_AUTONOMOUS) ? "autonomous" : "controller";
        char modeJson[128];
        serializeJson(reply, modeJson);
        udp.beginPacket(laptop_ip, udp_port);
        udp.write((const uint8_t*)modeJson, strlen(modeJson));
        udp.endPacket();

        Serial.printf("📤 Sent MODE ACK: %s\n", reply["mode"].as<const char*>());
    }
}

