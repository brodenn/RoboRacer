import tkinter as tk
import socket
import json
from config import params, presets, ESP32_IP, UDP_PORT, latest_data, log_options
import logger
from udp_comm import send_mode, register_ina_status_callback
from visualizer import set_auto_read

sliders = {}

def start_tk():
    root = tk.Tk()
    root.title("Robot Config Panel")
    root.geometry("1000x800")
    root.minsize(800, 600)

    canvas = tk.Canvas(root)
    scrollbar = tk.Scrollbar(root, orient="vertical", command=canvas.yview)
    scroll_frame = tk.Frame(canvas)
    scroll_frame.bind("<Configure>", lambda e: canvas.configure(scrollregion=canvas.bbox("all")))
    canvas.create_window((0, 0), window=scroll_frame, anchor="nw")
    canvas.configure(yscrollcommand=scrollbar.set)
    canvas.pack(side="left", fill="both", expand=True)
    scrollbar.pack(side="right", fill="y")

    # === Slider Definitions ===
    slider_defs = {
        "alignCorrectionStrength": (0.0, 2.0, 0.05),
        "backAvoidStrength": (0.0, 2.0, 0.05),
        "backAvoidThreshold": (50, 600, 10),
        "speedMultiplier": (0.5, 3.0, 0.05),
        "baseSpeedFactor": (30, 150, 5),
        "slowSpeedFactor": (20, 100, 5),
        "steeringSensitivity": (0, 100, 1),
        "MAX_RELIABLE_SIGMA": (10, 50, 1),
        "MIN_SIGNAL_THRESHOLD": (100, 1000, 10),
        "MAX_VALID_DISTANCE": (500, 2000, 10),
        "INVALID_DISTANCE": (1000, 2500, 10)
    }

    def add_slider(frame, name, frm, to, step):
        row = tk.Frame(frame)
        row.pack(fill="x", padx=10, pady=2)
        tk.Label(row, text=name, width=24).pack(side="left")
        var = tk.DoubleVar(value=params.get(name, 0))
        tk.Scale(row, variable=var, from_=frm, to=to, resolution=step,
                 orient="horizontal", length=250).pack(side="right")
        sliders[name] = var

    def send_selected(keys):
        msg = {key: sliders[key].get() for key in keys}
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            sock.sendto(json.dumps(msg).encode(), (ESP32_IP, UDP_PORT))
            print("📤 Sent:", msg)
        except Exception as e:
            print("Send error:", e)

    # === Slider Groups ===
    tuning_keys = ["alignCorrectionStrength", "backAvoidStrength", "backAvoidThreshold"]
    speed_keys = ["speedMultiplier", "baseSpeedFactor", "slowSpeedFactor", "steeringSensitivity"]
    sensor_keys = ["MAX_RELIABLE_SIGMA", "MIN_SIGNAL_THRESHOLD", "MAX_VALID_DISTANCE", "INVALID_DISTANCE"]

    tk.Label(scroll_frame, text="🔧 AI Steering Tuning", font=("Arial", 14, "bold")).pack(pady=(10, 2))
    steering_frame = tk.LabelFrame(scroll_frame, text="Steering Tuning", padx=10, pady=5)
    steering_frame.pack(padx=10, pady=5, fill="x")
    for key in tuning_keys:
        add_slider(steering_frame, key, *slider_defs[key])
    tk.Button(steering_frame, text="Send Steering Tuning", command=lambda: send_selected(tuning_keys)).pack(pady=5)

    speed_frame = tk.LabelFrame(scroll_frame, text="Speed Settings", padx=10, pady=5)
    speed_frame.pack(padx=10, pady=5, fill="x")
    for key in speed_keys:
        add_slider(speed_frame, key, *slider_defs[key])
    tk.Button(speed_frame, text="Send Speed Settings", command=lambda: send_selected(speed_keys)).pack(pady=5)

    sensor_frame = tk.LabelFrame(scroll_frame, text="Sensor Filter Settings", padx=10, pady=5)
    sensor_frame.pack(padx=10, pady=5, fill="x")
    for key in sensor_keys:
        add_slider(sensor_frame, key, *slider_defs[key])
    tk.Button(sensor_frame, text="Send Sensor Filters", command=lambda: send_selected(sensor_keys)).pack(pady=5)

    # === Detection Thresholds ===
    detection_frame = tk.LabelFrame(scroll_frame, text="VL53L4CD Detection Thresholds", padx=10, pady=5)
    detection_frame.pack(padx=10, pady=5, fill="x")

    sensor_names = {
        0: "Left",
        1: "Front",
        2: "Front-Right",
        3: "Front-Left",
        4: "Back-Left",
        5: "Right",
        6: "Back",
        7: "Back-Right"
    }

    detection_sliders = {}
    for i in range(8):
        row = tk.Frame(detection_frame)
        row.pack(fill="x", padx=10, pady=2)
        label = tk.Label(row, text=f"{sensor_names[i]} Threshold", width=24)
        label.pack(side="left")
        var = tk.IntVar(value=params.get(f"sensor_threshold_{i}", 400))
        tk.Scale(row, variable=var, from_=100, to=1200, resolution=10,
                 orient="horizontal", length=250).pack(side="right")
        detection_sliders[i] = var

    window_mode_var = tk.IntVar(value=params.get("window", 3))
    tk.Label(detection_frame, text="Interrupt Window Mode (0–3):").pack(pady=(10, 0))
    tk.Spinbox(detection_frame, from_=0, to=3, textvariable=window_mode_var, width=5).pack()

    def send_thresholds():
        thresholds = {str(i): detection_sliders[i].get() for i in detection_sliders}
        payload = {
            "thresholds": thresholds,
            "window": window_mode_var.get()
        }
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            sock.sendto(json.dumps(payload).encode(), (ESP32_IP, UDP_PORT))
            print("📤 Sent Detection Thresholds:", payload)
        except Exception as e:
            print("Send error:", e)

    tk.Button(detection_frame, text="Send Detection Thresholds", command=send_thresholds).pack(pady=10)

    # === Presets ===
    preset_frame = tk.LabelFrame(scroll_frame, text="Presets", padx=10, pady=5)
    preset_frame.pack(pady=10)
    for name in presets:
        tk.Button(preset_frame, text=name, command=lambda n=name: [
            sliders[k].set(v) for k, v in presets[n].items() if k in sliders
        ]).pack(side="left", padx=5)

    # === Control Buttons ===
    control_frame = tk.Frame(scroll_frame)
    control_frame.pack(pady=10)

    def send_start():
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            sock.sendto(json.dumps({"command": "start"}).encode(), (ESP32_IP, UDP_PORT))
            print("📤 Sent START command")
        except Exception as e:
            print("Send error:", e)

    def send_stop():
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            sock.sendto(json.dumps({"command": "stop"}).encode(), (ESP32_IP, UDP_PORT))
            print("📤 Sent STOP command")
        except Exception as e:
            print("Send error:", e)

    tk.Button(control_frame, text="▶️ Start", bg="lightgreen", width=12, command=send_start).pack(side="left", padx=10)
    tk.Button(control_frame, text="⛔ Stop", bg="salmon", width=12, command=send_stop).pack(side="left", padx=10)

    # === Logging Options ===
    logging_frame = tk.LabelFrame(scroll_frame, text="Logging Options", padx=10, pady=5)
    logging_frame.pack(padx=10, pady=5, fill="x")

    log_sensor = tk.BooleanVar(value=log_options.get("sensor", True))
    log_motor = tk.BooleanVar(value=log_options.get("motor", True))
    log_current = tk.BooleanVar(value=log_options.get("current", True))
    log_temp = tk.BooleanVar(value=log_options.get("temp", True))
    log_imu_ax = tk.BooleanVar(value=log_options.get("imu_ax", True))
    log_imu_ay = tk.BooleanVar(value=log_options.get("imu_ay", True))
    log_imu_gz = tk.BooleanVar(value=log_options.get("imu_gz", True))

    tk.Checkbutton(logging_frame, text="Sensor Data", variable=log_sensor).pack(anchor="w")
    tk.Checkbutton(logging_frame, text="Motor Data", variable=log_motor).pack(anchor="w")
    tk.Checkbutton(logging_frame, text="Current Data (INA)", variable=log_current).pack(anchor="w")
    tk.Checkbutton(logging_frame, text="IMU Temperature", variable=log_temp).pack(anchor="w")
    tk.Checkbutton(logging_frame, text="IMU ax", variable=log_imu_ax).pack(anchor="w")
    tk.Checkbutton(logging_frame, text="IMU ay", variable=log_imu_ay).pack(anchor="w")
    tk.Checkbutton(logging_frame, text="IMU gz", variable=log_imu_gz).pack(anchor="w")

    param_frame = tk.LabelFrame(logging_frame, text="Log Parameters", padx=10, pady=5)
    param_frame.pack(fill="x", pady=5)
    param_vars = {}
    for key in params:
        var = tk.BooleanVar(value=key in log_options.get("params", {}))
        tk.Checkbutton(param_frame, text=key, variable=var).pack(anchor="w")
        param_vars[key] = var

    def toggle_log_button():
        log_options["sensor"] = log_sensor.get()
        log_options["motor"] = log_motor.get()
        log_options["current"] = log_current.get()
        log_options["temp"] = log_temp.get()
        log_options["imu_ax"] = log_imu_ax.get()
        log_options["imu_ay"] = log_imu_ay.get()
        log_options["imu_gz"] = log_imu_gz.get()
        log_options["params"] = {k: True for k, v in param_vars.items() if v.get()}

        logger.toggle_logging()

        log_button.config(
            text="✅ Logging Enabled" if logger.log_enabled else "⏹️ Logging Disabled",
            bg="lightgreen" if logger.log_enabled else "lightgray"
        )


    initial_text = "✅ Logging Enabled" if logger.log_enabled else "⏹️ Logging Disabled"
    initial_color = "lightgreen" if logger.log_enabled else "lightgray"
    log_button = tk.Button(logging_frame, text=initial_text, bg=initial_color, width=22, command=toggle_log_button)

    log_button.pack(pady=5)

    # === INA219 Monitor ===
    def toggle_ina_read():
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            msg = json.dumps({"command": "toggle_ina_auto_read"})
            sock.sendto(msg.encode(), (ESP32_IP, UDP_PORT))
            print("📤 Sent toggle INA auto-read command")
        except Exception as e:
            print("Send error:", e)

    ina_frame = tk.LabelFrame(scroll_frame, text="INA219 Monitor", padx=10, pady=5)
    ina_frame.pack(padx=10, pady=5, fill="x")

    ina_status = tk.Label(ina_frame, text="INA Auto-Read: Unknown", fg="blue")
    ina_status.pack()

    def update_ina_status(state):
        ina_status.config(
            text=f"INA Auto-Read: {'ENABLED' if state else 'DISABLED'}",
            fg="green" if state else "red"
        )

    register_ina_status_callback(update_ina_status)
    tk.Button(ina_frame, text="Toggle INA Auto-Read", command=toggle_ina_read).pack(pady=5)

    # === Mode Display ===
    mode_frame = tk.LabelFrame(scroll_frame, text="Mode Selection", padx=10, pady=5)
    mode_frame.pack(pady=10)

    mode_label = tk.Label(mode_frame, text="Current Mode: Unknown", font=("Arial", 14), width=30)
    mode_label.pack(pady=5)

    btn_auto = tk.Button(mode_frame, text="🤖 Self-Driving", width=15, command=lambda: send_mode("autonomous"))
    btn_auto.pack(side="left", padx=5)

    btn_controller = tk.Button(mode_frame, text="🎮 Controller", width=15, command=lambda: send_mode("controller"))
    btn_controller.pack(side="left", padx=5)

    def update_mode_display():
        mode = latest_data.get("mode", "unknown").lower()
        pretty = mode.capitalize() if mode != "unknown" else "Unknown"
        color = {"autonomous": "lightgreen", "controller": "lightblue"}.get(mode, "lightgray")
        mode_label.config(text=f"Current Mode: {pretty}", bg=color)
        btn_auto.config(state="normal" if mode != "autonomous" else "disabled")
        btn_controller.config(state="normal" if mode != "controller" else "disabled")
        root.after(500, update_mode_display)

    update_mode_display()
    root.mainloop()
