#include <WiFi.h>
#include <ArduinoOTA.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <WebSerial.h>
#include "secrets.h"
#include "mux.h"
#include "opt.h"
#include "ota.h"

extern void setLeftSpeed(int pwmPercent);
extern void setRightSpeed(int pwmPercent);
extern void moveForward();
extern void moveBackward();
extern void turnLeft();
extern void turnRight();
extern void stopOrReverse();
extern void setManualSteeringBias(float bias);
extern bool isAutoMode; 

// WebServer och WebSocket
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    client->text("Välkommen till Panzer WebSocket!");
  } else if (type == WS_EVT_DATA) {
    String msg = "";
    for (size_t i = 0; i < len; i++) msg += (char)data[i];

    if (msg == "cmd:reset") {
      ws.textAll("🔄 Startar om systemet...");
      delay(200);
      ESP.restart();
    } else if (msg.startsWith("pwm_left:")) {
      setLeftSpeed(msg.substring(9).toInt());
    } else if (msg.startsWith("pwm_right:")) {
      setRightSpeed(msg.substring(10).toInt());
    } else if (msg.startsWith("move:")) {
      String dir = msg.substring(5);
      if (dir == "forward") moveForward();
      else if (dir == "backward") moveBackward();
      else if (dir == "left") turnLeft();
      else if (dir == "right") turnRight();
      else if (dir == "stop") stopOrReverse();
    } else if (msg.startsWith("bias:")) {
      float val = msg.substring(5).toFloat();
      setManualSteeringBias(val / 100.0);
    } else if (msg == "mode:auto") {
      isAutoMode = true;
      ws.textAll("AUTO MODE: ON");
    } else if (msg == "mode:manual") {
      isAutoMode = false;
      ws.textAll("AUTO MODE: OFF");
    } else if (msg == "ws:on") {
      isWebSocketDataEnabled = true;
      ws.textAll("📡 WebSocket-data: ON");
    } else if (msg == "ws:off") {
      isWebSocketDataEnabled = false;
      ws.textAll("📴 WebSocket-data: OFF");
    } else if (msg == "ws:status") {
      ws.textAll(isWebSocketDataEnabled ? "📡 WebSocket-data: ON" : "📴 WebSocket-data: OFF");
    }
  }
}
const char* getControlHTML() {
  return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>Panzer Kontroll</title>
  <style>
    body { background: #121212; color: #fff; font-family: sans-serif; text-align: center; }
    canvas { background: #000; margin: 1em auto; border: 1px solid #555; display: block; }
    input[type=range] { width: 60%; }
    button { padding: 1em; margin: 0.3em; border-radius: 10px; background: #333; color: #fff; border: none; }
    #log { width: 90%; height: 200px; background: #1e1e1e; overflow-y: scroll; border: 1px solid #444; margin: auto; padding: 1em; text-align: left; }
    #modeBtn { font-weight: bold; }
  
  .switch {
    position: relative;
    display: inline-block;
    width: 60px;
    height: 34px;
    margin-top: 10px;
  }

  .switch input {
    opacity: 0;
    width: 0;
    height: 0;
  }

  .slider {
    position: absolute;
    cursor: pointer;
    top: 0; left: 0;
    right: 0; bottom: 0;
    background-color: #ccc;
    transition: .4s;
    border-radius: 34px;
  }

  .slider:before {
    position: absolute;
    content: "";
    height: 26px; width: 26px;
    left: 4px; bottom: 4px;
    background-color: white;
    transition: .4s;
    border-radius: 50%;
  }

  input:checked + .slider {
    background-color: #4CAF50;
  }

  input:checked + .slider:before {
    transform: translateX(26px);
  }

  </style>
</head>
<body>
  <h1>🛡️ Panzer Kontroll</h1>

  <h3>Vänster PWM: <span id="pwmLeftVal">50</span>%</h3>
  <input type="range" min="-100" max="100" value="50" id="pwmLeft" oninput="throttledPWM('left')">
  <h3>Höger PWM: <span id="pwmRightVal">50</span>%</h3>
  <input type="range" min="-100" max="100" value="50" id="pwmRight" oninput="throttledPWM('right')">

  <h3>Steering Bias: <span id="biasVal">0</span></h3>
  <input type="range" min="-100" max="100" value="0" id="bias" oninput="updateBias()">

  <h3>Körtid (ms): <span id="durationVal">1000</span></h3>
  <input type="range" min="100" max="5000" value="1000" step="100" id="duration" oninput="updateDuration()">

  <canvas id="sensorCanvas" width="600" height="300"></canvas>

  <div>
    <button onclick="sendTimedCommand('move:left')">⬅️</button>
    <button onclick="sendTimedCommand('move:forward')">⬆️</button>
    <button onclick="sendTimedCommand('move:right')">➡️</button><br>
    <button onclick="sendTimedCommand('move:backward')">⬇️</button>
    <button onclick="sendCommand('move:stop')">⏹️</button>
    <button onclick="sendCommand('cmd:reset')">🔄 RESET</button>
  </div>

  <h3>Läge:</h3>
  <label class="switch">
    <input type="checkbox" id="modeToggle" checked onchange="toggleMode()">
    <span class="slider round"></span>
  </label>
  <span id="modeLabel">AUTO</span>


  <h3>Logg:</h3>
  <div id="log"></div>

  <h3>Data:</h3>
  <button onclick="sendCommand('ws:on')">📡 Data ON</button>
  <button onclick="sendCommand('ws:off')">📴 Data OFF</button>

  <script>
    let ws;
    const canvas = document.getElementById('sensorCanvas');
    const ctx = canvas.getContext('2d');
    const pwmTimeout = {};
    let driveDuration = 1000;
    let autoMode = true;

    function toggleMode() {
      const toggle = document.getElementById("modeToggle");
      const label = document.getElementById("modeLabel");

    autoMode = toggle.checked;
      if (autoMode) {
      sendCommand("mode:auto");
      label.innerText = "AUTO";
    } else {
      sendCommand("mode:manual");
      label.innerText = "MANUAL";
    }
}


function connectWebSocket() {
  ws = new WebSocket(`ws://${location.hostname}/ws`);

ws.onopen = () => {
  appendLog("✅ WebSocket ansluten");
  sendCommand("ws:status");

  // Håll anslutningen vid liv
  setInterval(() => {
    if (ws.readyState === WebSocket.OPEN) {
      ws.send("ping");
    }
  }, 5000); // var 5:e sekund
};


  ws.onclose = () => {
    appendLog("❌ WebSocket frånkopplad");
    setTimeout(connectWebSocket, 2000);
  };

  ws.onmessage = e => {
    appendLog("📩 " + e.data);
    if (e.data.startsWith("sensor:")) drawSensorData(e.data);
    else if (e.data.startsWith("pwm:")) drawPWMBars(e.data);
  };
}

function drawSensorData(data) {
  const parts = data.replace("sensor:", "").split(",").map(Number);
  if (parts.length < 9) return;

  // === Sensorvärden ===
  const vl_L = parts[0];
  const opt0 = parts[1];
  const opt1 = parts[2];
  const opt2 = parts[3];
  const vl_R = parts[4];
  const pwmL_raw = parts[5];
  const pwmR_raw = parts[6];
  const pwmL_pct = parts[7];
  const pwmR_pct = parts[8];

  // === Rita sensordata ===
  ctx.clearRect(0, 0, canvas.width, canvas.height);
  const h = canvas.height;
  const maxDist = 1000;
  const minDist = 100;
  const maxVisualSize = 200;

  const invScale = d => {
    const clamped = Math.max(minDist, Math.min(d, maxDist));
    return ((maxDist - clamped) / (maxDist - minDist)) * maxVisualSize;
  };

  const dangerColor = d => {
    const t = Math.max(0, Math.min(1, (maxDist - d) / maxDist));
    const r = Math.floor(255 * t);
    const g = Math.floor(255 * (1 - t));
    return `rgb(${r},${g},0)`;
  };

  ctx.fillStyle = dangerColor(vl_L);
  ctx.fillRect(40, h - invScale(vl_L), 20, invScale(vl_L));

  ctx.fillStyle = dangerColor(vl_R);
  ctx.fillRect(540, h - invScale(vl_R), 20, invScale(vl_R));

  const baseX = 300;
  const baseY = h - 20;
  drawCone(baseX, baseY, invScale(opt0), -140, dangerColor(opt0));
  drawCone(baseX, baseY, invScale(opt1),  -90, dangerColor(opt1));
  drawCone(baseX, baseY, invScale(opt2),  -40, dangerColor(opt2));

  // === PWM-visning i vänster/höger marginal ===
  ctx.clearRect(0, 0, 20, h);     // Vänster motorområde
  ctx.clearRect(580, 0, 20, h);   // Höger motorområde

  const drawBar = (x, pwm) => {
    const color = pwm >= 0 ? "green" : "red";
    const height = Math.abs(pwm) * 2;
    const y = pwm >= 0 ? h / 2 - height : h / 2;
    ctx.fillStyle = color;
    ctx.fillRect(x, y, 20, height);
  };

  drawBar(10, pwmL_pct);
  drawBar(570, pwmR_pct);
}

    function drawCone(cx, cy, length, angleDeg, color) {
      const angle = angleDeg * Math.PI / 180;
      const spread = 25 * Math.PI / 180;
      const x1 = cx + length * Math.cos(angle - spread);
      const y1 = cy + length * Math.sin(angle - spread);
      const x2 = cx + length * Math.cos(angle + spread);
      const y2 = cy + length * Math.sin(angle + spread);
      ctx.beginPath();
      ctx.moveTo(cx, cy);
      ctx.lineTo(x1, y1);
      ctx.lineTo(x2, y2);
      ctx.closePath();
      ctx.fillStyle = color;
      ctx.fill();
    }

    function sendCommand(cmd) {
      if (ws.readyState === WebSocket.OPEN) {
        ws.send(cmd);
        appendLog("📤 " + cmd);
      }
    }

    function sendTimedCommand(cmd) {
      sendCommand(cmd);
      setTimeout(() => sendCommand("move:stop"), driveDuration);
    }

    function updateBias() {
      const val = document.getElementById("bias").value;
      document.getElementById("biasVal").innerText = val;
      sendCommand(`bias:${val}`);
    }

    function updateDuration() {
      const val = document.getElementById("duration").value;
      document.getElementById("durationVal").innerText = val;
      driveDuration = parseInt(val);
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

void setupWiFiAndOTA() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("🔌 Ansluter till WiFi-nätverket");
  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("✅ Ansluten till WiFi! IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ Kunde inte ansluta till WiFi – fortsätter i AP-läge");
  }

  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.print("📡 Eget AP aktivt – IP: ");
  Serial.println(WiFi.softAPIP());

  ArduinoOTA.setHostname("Panzer");
  ArduinoOTA.setPassword(OTA_PASSWORD);
  ArduinoOTA.begin();

  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", getControlHTML());
  });

  WebSerial.begin(&server);
  WebSerial.println("📡 Panzer WebSerial aktiv");
  server.begin();
}