# ====== CONFIG ======
UDP_PORT = 12345
LOCAL_IP = "0.0.0.0"
ESP32_IP = "192.168.4.1"

robot_ip = ESP32_IP
udp_port = UDP_PORT

# ==== Default Parameters ====
params = {
    "backAvoidStrength": 0.8,
    "backAvoidThreshold": 300,
    "alignCorrectionStrength": 0.5,
    "speedMultiplier": 2.0,
    "baseSpeedFactor": 100,
    "slowSpeedFactor": 60,
    "steeringSensitivity": 65,
    "MAX_RELIABLE_SIGMA": 20,
    "MIN_SIGNAL_THRESHOLD": 600,
    "MAX_VALID_DISTANCE": 1300,
    "INVALID_DISTANCE": 2000,
    "sensor_threshold_0": 300,
    "sensor_threshold_1": 600,
    "sensor_threshold_2": 500,
    "sensor_threshold_3": 500,
    "sensor_threshold_4": 250,
    "sensor_threshold_5": 300,
    "sensor_threshold_6": 250,
    "sensor_threshold_7": 250,
    "window": 3
}

# ==== Parameter Presets ====
presets = {
    "Balanced": params.copy(),
    "Aggressive": {
        **params,
        "backAvoidStrength": 1.2,
        "alignCorrectionStrength": 0.9,
        "backAvoidThreshold": 250,
        "speedMultiplier": 2.5,
        "baseSpeedFactor": 110,
        "slowSpeedFactor": 70,
        "steeringSensitivity": 85,
        "MAX_RELIABLE_SIGMA": 18,
        "MIN_SIGNAL_THRESHOLD": 500,
        "MAX_VALID_DISTANCE": 1400
    },
    "Gentle": {
        **params,
        "backAvoidStrength": 0.4,
        "alignCorrectionStrength": 0.3,
        "backAvoidThreshold": 400,
        "speedMultiplier": 1.5,
        "baseSpeedFactor": 80,
        "slowSpeedFactor": 40,
        "steeringSensitivity": 45,
        "MAX_RELIABLE_SIGMA": 25,
        "MIN_SIGNAL_THRESHOLD": 700,
        "MAX_VALID_DISTANCE": 1200
    },
    "FastButCareful": {
        **params,
        "backAvoidStrength": 1.0,
        "alignCorrectionStrength": 0.7,
        "backAvoidThreshold": 300,
        "speedMultiplier": 2.2,
        "baseSpeedFactor": 105,
        "slowSpeedFactor": 70,
        "steeringSensitivity": 70,
        "MAX_RELIABLE_SIGMA": 20,
        "MIN_SIGNAL_THRESHOLD": 600,
        "MAX_VALID_DISTANCE": 1350
    },
    "StableMid": {
        **params,
        "backAvoidStrength": 0.6,
        "alignCorrectionStrength": 0.45,
        "backAvoidThreshold": 320,
        "speedMultiplier": 1.8,
        "baseSpeedFactor": 95,
        "slowSpeedFactor": 55,
        "steeringSensitivity": 60,
        "MAX_RELIABLE_SIGMA": 22,
        "MIN_SIGNAL_THRESHOLD": 650,
        "MAX_VALID_DISTANCE": 1250
    }
}

# ==== Shared Global State ====
latest_data = {
    "f": 2000, "fl": 2000, "fr": 2000, "l": 2000, "r": 2000, "bl": 2000, "br": 2000, "b": 2000,
    "m1": 0, "m2": 0, "m3": 0, "m4": 0,
    "ina60": 0.0,
    "ina61": 0.0,
    "mode": "unknown",
    "imu_ax": 0.0, "imu_ay": 0.0, "imu_gz": 0.0, "imu_temp": 0.0
}

# ==== Logging Configuration ====
log_enabled = False
log_file = None
csv_writer = None

log_options = {
    "sensor": True,
    "motor": True,
    "current": True,
    "temp": True,
    "params": {
        "backAvoidStrength": True,
        "alignCorrectionStrength": True,
        "steeringSensitivity": True,
        "speedMultiplier": True
    }
}
