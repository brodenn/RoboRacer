// ota.cpp
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <WebSerial.h>

#include "motors.h"
#include "mux.h"
#include "ota.h"

// 📡 Server & WebSocket-instans
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// 🔁 WebSocket-hanterare
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.println("🔌 WebSocket: klient ansluten");
    client->text("Välkommen till Panzer WebSocket!");
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.println("🔌 WebSocket: klient frånkopplad");
  } else if (type == WS_EVT_DATA) {
    String msg = "";
    for (size_t i = 0; i < len; i++) msg += (char)data[i];

    Serial.println("📩 WS mottaget: " + msg);

    if (msg == "cmd:reset") {
      ws.textAll("🔄 Startar om systemet...");
      delay(200);
      ESP.restart();
    } else if (msg.startsWith("pwm_left:")) {
      int val = msg.substring(9).toInt();
      setLeftSpeed(val);
    } else if (msg.startsWith("pwm_right:")) {
      int val = msg.substring(10).toInt();
      setRightSpeed(val);
    } else if (msg.startsWith("move:")) {
      String dir = msg.substring(5);
      if (dir == "forward") moveForward();
      else if (dir == "backward") moveBackward();
      else if (dir == "left") turnLeft();
      else if (dir == "right") turnRight();
      else if (dir == "stop") stopOrReverse();
    }
  }
}

// 🌐 Returnerar HTML-sida
const char* getControlHTML() {
  return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>Panzer Kontroll</title>
  <style>
    body { font-family: sans-serif; background: #121212; color: #fff; text-align: center; padding: 2em; }
    h1 { margin-bottom: 0.5em; }
    input[type=range] { width: 60%; }
    button { margin: 0.3em; padding: 1em; font-size: 1em; border: none; border-radius: 8px; background: #333; color: #fff; cursor: pointer; }
    button:hover { background: #555; }
    #log { width: 90%; height: 200px; background: #1e1e1e; border: 1px solid #444; padding: 1em; overflow-y: scroll; margin: auto; text-align: left; }
  </style>
</head>
<body>
  <h1>🛡️ Panzer Kontroll</h1>
  <h3>Vänster Motor PWM: <span id="pwmLeftVal">50</span>%</h3>
  <input type="range" min="0" max="100" value="50" id="pwmLeft" oninput="throttledPWM('left')">
  <h3>Höger Motor PWM: <span id="pwmRightVal">50</span>%</h3>
  <input type="range" min="0" max="100" value="50" id="pwmRight" oninput="throttledPWM('right')">
  <div style="margin-top: 2em;">
    <button onclick="sendCommand('move:left')">⬅️ Vänster</button>
    <button onclick="sendCommand('move:forward')">⬆️ Framåt</button>
    <button onclick="sendCommand('move:right')">➡️ Höger</button><br>
    <button onclick="sendCommand('move:backward')">⬇️ Bakåt</button>
    <button onclick="sendCommand('move:stop')">🟥 Stanna</button>
    <button onclick="sendCommand('cmd:reset')">🔄 RESET</button>
  </div>
  <div style="margin-top: 2em;">
    <input type="text" id="cmdInput" placeholder="Eget kommando...">
    <button onclick="sendCustom()">📤 Skicka</button>
  </div>
  <h3>Logg:</h3>
  <div id="log"></div>
  <script>
    let ws;
    let pwmTimeout = {};
    function connectWebSocket() {
      ws = new WebSocket(`ws://${location.hostname}/ws`);
      ws.onopen = () => appendLog("✅ WebSocket ansluten");
      ws.onclose = () => {
        appendLog("❌ WebSocket frånkopplad");
        setTimeout(connectWebSocket, 2000);
      };
      ws.onmessage = (e) => appendLog("📩 " + e.data);
    }
    function sendCommand(cmd) {
      if (ws.readyState === WebSocket.OPEN) {
        ws.send(cmd);
        appendLog("📤 " + cmd);
      }
    }
    function throttledPWM(side) {
      clearTimeout(pwmTimeout[side]);
      pwmTimeout[side] = setTimeout(() => {
        const slider = document.getElementById("pwm" + capitalize(side));
        const val = slider.value;
        document.getElementById("pwm" + capitalize(side) + "Val").innerText = val;
        sendCommand(`pwm_${side}:${val}`);
      }, 100);
    }
    function sendCustom() {
      const msg = document.getElementById("cmdInput").value.trim();
      if (msg) {
        sendCommand(msg);
        document.getElementById("cmdInput").value = "";
      }
    }
    function appendLog(msg) {
      const log = document.getElementById("log");
      log.innerHTML += msg + "<br>";
      log.scrollTop = log.scrollHeight;
    }
    function capitalize(str) {
      return str.charAt(0).toUpperCase() + str.slice(1);
    }
    window.onload = connectWebSocket;
  </script>
</body>
</html>
  )rawliteral";
}

// 🚀 Setup-funktion för WiFi, OTA, WebSerial & WebSocket
void setupWiFiAndOTA() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP("Panzer_AP", "update123");
  IPAddress ip = WiFi.softAPIP();

  Serial.println("📡 ESP startade som Access Point");
  Serial.print("🔐 SSID: Panzer_AP\n🔑 Lösenord: update123\n📍 IP-adress: ");
  Serial.println(ip);

  ArduinoOTA.setHostname("Panzer");
  ArduinoOTA.setPassword("update123");
  ArduinoOTA.begin();
  Serial.println("✅ OTA redo via AP-läge!");

  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", getControlHTML());
  });

  WebSerial.onMessage([](uint8_t *data, size_t len) {
    String msg = String((char*)data);
    msg.trim();

    WebSerial.println("⏎ WebSerial: " + msg);

    if (msg == "cmd:reset") {
      WebSerial.println("🔄 Startar om systemet...");
      delay(200);
      ESP.restart();
    } else if (msg.startsWith("pwm_left:")) {
      int val = msg.substring(9).toInt();
      setLeftSpeed(val);
    } else if (msg.startsWith("pwm_right:")) {
      int val = msg.substring(10).toInt();
      setRightSpeed(val);
    } else if (msg.startsWith("move:")) {
      String dir = msg.substring(5);
      if (dir == "forward") moveForward();
      else if (dir == "backward") moveBackward();
      else if (dir == "left") turnLeft();
      else if (dir == "right") turnRight();
      else if (dir == "stop") stopOrReverse();
    }
  });

  server.begin();
  Serial.println("🌐 WebServer igång på: http://" + ip.toString());

  WebSerial.print("Panzer säger hej via Wi-Fi! Avstånd: ");
  WebSerial.println(vl53Distances[0]);
}
