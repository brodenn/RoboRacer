from datetime import datetime
import time
import os
import csv
from config import params, latest_data, log_options  # ✅ log_options must be defined in config

log_enabled = False
log_file = None
csv_writer = None

def create_new_logfile():
    global log_file, csv_writer
    os.makedirs("logs", exist_ok=True)
    timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    filepath = os.path.join("logs", f"robot_log_{timestamp}.csv")

    try:
        log_file = open(filepath, mode="w", newline="")
        csv_writer = csv.writer(log_file)

        # === HEADER GENERATION ===
        header = ["Timestamp", "UnixTime"]

        # Sensor distances
        if log_options.get("sensor", True):
            header += ["F", "FL", "FR", "L", "R", "BL", "BR", "B"]

        # Motor telemetry
        if log_options.get("motor", True):
            for i in range(1, 5):
                header += [f"M{i}_Speed", f"M{i}_Dir"]

        # INA current sensors
        if log_options.get("current", True):
            header += ["INA60 (mA)", "INA61 (mA)"]

        # IMU data
        if log_options.get("temp", False):
            header.append("IMU_Temp (°C)")
        if log_options.get("imu_ax", False):
            header.append("IMU_ax")
        if log_options.get("imu_ay", False):
            header.append("IMU_ay")
        if log_options.get("imu_gz", False):
            header.append("IMU_gz")

        # Selected parameters
        if "params" in log_options:
            header += list(log_options["params"].keys())

        csv_writer.writerow(header)
        print(f"📝 Logging to {filepath}")
    except Exception as e:
        print(f"❌ Failed to create log file: {e}")
        log_file = None
        csv_writer = None

def toggle_logging():
    global log_enabled
    log_enabled = not log_enabled

    if log_enabled:
        create_new_logfile()
    else:
        print("⏹️ Logging disabled")

def log_data(msg):
    if not log_enabled or not csv_writer:
        return

    now = datetime.now()
    unix = time.time()
    row = [now.strftime("%Y-%m-%d %H:%M:%S"), unix]

    # === MATCH HEADER STRUCTURE ===
    if log_options.get("sensor", True):
        row += [
            msg.get("f", 2000), msg.get("fl", 2000), msg.get("fr", 2000),
            msg.get("l", 2000), msg.get("r", 2000),
            msg.get("bl", 2000), msg.get("br", 2000), msg.get("b", 2000)
        ]

    if log_options.get("motor", True):
        for motor in ["m1", "m2", "m3", "m4"]:
            speed = msg.get(motor, 0)
            row.append(abs(speed))
            row.append(1 if speed >= 0 else -1)

    if log_options.get("current", True):
        row.append(msg.get("ina60", latest_data.get("ina60", 0.0)))
        row.append(msg.get("ina61", latest_data.get("ina61", 0.0)))

    if log_options.get("temp", False):
        row.append(msg.get("imu_temp", latest_data.get("imu_temp", 0.0)))
    if log_options.get("imu_ax", False):
        row.append(msg.get("imu_ax", latest_data.get("imu_ax", 0.0)))
    if log_options.get("imu_ay", False):
        row.append(msg.get("imu_ay", latest_data.get("imu_ay", 0.0)))
    if log_options.get("imu_gz", False):
        row.append(msg.get("imu_gz", latest_data.get("imu_gz", 0.0)))

    if "params" in log_options:
        for param_name in log_options["params"]:
            row.append(params.get(param_name, 0))

    try:
        csv_writer.writerow(row)
        log_file.flush()
    except Exception as e:
        print(f"⚠️ Logging error: {e}")
