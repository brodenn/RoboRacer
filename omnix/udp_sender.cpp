#include "udp_comm.h"
#include "globals.h"
#include <ArduinoJson.h>
#include "mux.h"
#include <Adafruit_INA219.h>

extern Adafruit_INA219 ina60;
extern Adafruit_INA219 ina61;
extern bool inaContinuous;

void sendData() {
    // Only send data every 250 ms
    if (millis() - lastDataSend < sendInterval) return;
    lastDataSend = millis();

    StaticJsonDocument<512> msg;

    // === Sensor Data ===
    msg["l"]  = distances[0];
    msg["f"]  = distances[1];
    msg["fr"] = distances[2];
    msg["fl"] = distances[3];
    msg["bl"] = distances[4];
    msg["r"]  = distances[5];
    msg["b"]  = distances[6];
    msg["br"] = distances[7];

    // === Motor Speeds ===
    msg["m1"] = current_m1;
    msg["m2"] = current_m2;
    msg["m3"] = current_m3;
    msg["m4"] = current_m4;

    // === IMU Telemetry ===
    msg["imu_ax"] = imu_ax;
    msg["imu_ay"] = imu_ay;
    msg["imu_az"] = imu_az;
    msg["imu_gx"] = imu_gx;
    msg["imu_gy"] = imu_gy;
    msg["imu_gz"] = imu_gz;
    msg["imu_temp"] = imu_temp;

    // === Mode Info & Timestamp ===
    msg["mode"] = (controlMode == MODE_AUTONOMOUS) ? "autonomous" : "controller";
    msg["ts"] = millis();

    char json[512];
    serializeJson(msg, json);

    if (xSemaphoreTake(wifiSemaphore, pdMS_TO_TICKS(50))) {
        udp.beginPacket(laptop_ip, udp_port);
        udp.write((const uint8_t*)json, strlen(json));
        udp.endPacket();
        xSemaphoreGive(wifiSemaphore);
    }

    // === INA219 Auto Read (optional) ===
    if (inaContinuous) {
        float current_ina60 = 0.0f;
        float current_ina61 = 0.0f;

        deselectAllMux();
        selectMuxINA60();
        current_ina60 = ina60.getCurrent_mA();

        deselectAllMux();
        selectMuxINA61();
        current_ina61 = ina61.getCurrent_mA();

        StaticJsonDocument<256> reply;
        reply["ack"] = "ina_data";
        reply["ina60"] = current_ina60;
        reply["ina61"] = current_ina61;

        char inaJson[256];
        serializeJson(reply, inaJson);
        udp.beginPacket(laptop_ip, udp_port);
        udp.write((const uint8_t*)inaJson, strlen(inaJson));
        udp.endPacket();
    }
}
