import socket
import json
from config import UDP_PORT, LOCAL_IP, ESP32_IP, params, latest_data
from logger import log_data

# Callbacks injected from ui.py
mode_callback = None
ina_status_callback = None

def register_mode_callback(callback):
    global mode_callback
    mode_callback = callback

def register_ina_status_callback(callback):
    global ina_status_callback
    ina_status_callback = callback

def start_udp_listener():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((LOCAL_IP, UDP_PORT))

    while True:
        try:
            data, _ = sock.recvfrom(1024)
            msg = json.loads(data.decode())

            print("📦 RAW MSG:", msg)

            # === INA219 Data ===
            if msg.get("ack") == "ina_data":
                ina60_val = msg.get("ina60", 0)
                ina61_val = msg.get("ina61", 0)
                print(f"✅ INA Data received: INA60 = {ina60_val:.2f} mA | INA61 = {ina61_val:.2f} mA")
                latest_data["ina60"] = ina60_val
                latest_data["ina61"] = ina61_val
                log_data(msg)
                continue

            # === Auto INA toggle ack ===
            if msg.get("ack") == "ina_auto_read":
                state = msg.get("state", False)
                print(f"✅ ESP32 INA Auto Read ACK → {'ENABLED' if state else 'DISABLED'}")
                latest_data["ina_auto_read"] = state
                if ina_status_callback:
                    ina_status_callback(state)
                continue

            # === Mode ACK handling ===
            if msg.get("ack") == "mode" and "mode" in msg:
                mode_str = msg["mode"]
                print(f"✅ Mode updated to: {mode_str.upper()}")
                latest_data["mode"] = mode_str
                if mode_callback:
                    mode_callback(mode_str.upper())
                continue

            # === General ACKs ===
            if msg.get("ack"):
                print(f"✅ ESP32 ACK: {msg['ack']}")
                continue

            # === IMU Telemetry ===
            imu_keys = [
                "imu_ax", "imu_ay", "imu_az",
                "imu_gx", "imu_gy", "imu_gz",
                "imu_temp"
            ]
            imu_data = {k: msg.get(k) for k in imu_keys if k in msg}
            if imu_data:
                print("📈 IMU:", imu_data)
                latest_data.update(imu_data)

            # === Telemetry data with mode ===
            if "mode" in msg:
                latest_data["mode"] = msg["mode"]
                print(f"🚦 Mode switched to: {msg['mode'].upper()}")
                if mode_callback:
                    mode_callback(msg["mode"].upper())

            # === Full telemetry update ===
            latest_data.update(msg)
            log_data(msg)

        except Exception as e:
            print("❌ UDP error:", e)

def send_params():
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.sendto(json.dumps(params).encode(), (ESP32_IP, UDP_PORT))
        print("📤 Sent params to ESP32!")
    except Exception as e:
        print("❌ Send error:", e)

def send_mode(mode):
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        msg = json.dumps({"command": "mode", "mode": mode})
        sock.sendto(msg.encode(), (ESP32_IP, UDP_PORT))
        print(f"📤 Sent mode change: {mode}")
    except Exception as e:
        print("❌ Send error:", e)
