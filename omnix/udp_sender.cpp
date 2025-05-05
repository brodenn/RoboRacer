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
    msg["f"]  = distances[0];
    msg["fl"] = distances[1];
    msg["l"]  = distances[2];
    msg["bl"] = distances[3];
    msg["b"]  = distances[4];
    msg["br"] = distances[5];
    msg["r"]  = distances[6];
    msg["fr"] = distances[7];

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
        Serial.println("⚡ INA auto-read enabled — reading INA219");

        float current_ina60 = ina60.getCurrent_mA();
        float current_ina61 = ina61.getCurrent_mA();

        // Debug validation
        if (isnan(current_ina60) || isnan(current_ina61)) {
            Serial.println("❌ INA219 read failed: got NaN values");
        } else {
            Serial.printf("📊 INA219: INA60 = %.2f mA | INA61 = %.2f mA\n", current_ina60, current_ina61);
        }

        delayMicroseconds(300);  // Avoid back-to-back UDP issues

        StaticJsonDocument<256> reply;
        reply["ack"] = "ina_data";
        reply["ina60"] = current_ina60;
        reply["ina61"] = current_ina61;

        char inaJson[256];
        serializeJson(reply, inaJson);

        if (xSemaphoreTake(wifiSemaphore, pdMS_TO_TICKS(50))) {
            udp.beginPacket(laptop_ip, udp_port);
            udp.write((const uint8_t*)inaJson, strlen(inaJson));
            udp.endPacket();
            xSemaphoreGive(wifiSemaphore);
        }
    }
}
