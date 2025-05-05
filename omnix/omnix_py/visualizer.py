import pygame
import math
import config
import socket
import json
import csv
import time
from datetime import datetime

# === Config ===
INA_SCALE = 0.1
AUTO_READ_INTERVAL = 2.0

# Shared States
auto_read_enabled = False
log_enabled = False
force_manual_read = False

log_file = open("ina_log.csv", "w", newline="")
csv_writer = csv.writer(log_file)
csv_writer.writerow(["time", "front_mA", "back_mA"])

def distance_to_color(dist):
    ratio = min(max(dist / config.params["INVALID_DISTANCE"], 0), 1)
    return (int((1 - ratio) * 255), int(ratio * 255), 0)

def set_auto_read(state: bool):
    global auto_read_enabled
    auto_read_enabled = state

def set_logging(state: bool):
    global log_enabled
    log_enabled = state

def trigger_manual_read():
    global force_manual_read
    force_manual_read = True

def draw_pygame():
    global INA_SCALE, log_enabled, auto_read_enabled, force_manual_read

    pygame.init()
    screen = pygame.display.set_mode((900, 800), pygame.RESIZABLE)
    pygame.display.set_caption("Sensor Visualizer + INA + IMU")
    clock = pygame.time.Clock()
    font = pygame.font.SysFont(None, 24)

    smoothed_ina60 = smoothed_ina61 = 0

    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                log_file.close()
                pygame.quit()
                return

        width, height = screen.get_size()
        cx, cy = width // 2, height // 2
        sensor_radius = 60
        screen.fill((0, 0, 0))

        latest = config.latest_data.copy()
        last_ina60 = latest.get("ina60", 0)
        last_ina61 = latest.get("ina61", 0)

        if log_enabled:
            csv_writer.writerow([datetime.now().strftime("%H:%M:%S"), last_ina60, last_ina61])
            log_file.flush()

        smoothed_ina60 += (last_ina60 - smoothed_ina60) * 0.2
        smoothed_ina61 += (last_ina61 - smoothed_ina61) * 0.2

        pygame.draw.circle(screen, (255, 255, 255), (cx, cy), sensor_radius, 1)

        sensor_labels = ["f", "fl", "l", "bl", "b", "br", "r", "fr"]
        angles = [270, 225, 180, 135, 90, 45, 0, 315]

        for i, label in enumerate(sensor_labels):
            dist = min(latest.get(label, 2000), 2000)
            angle_rad = math.radians(angles[i])
            offset = sensor_radius + 10
            length = (dist / 2000) * 200
            x0 = cx + offset * math.cos(angle_rad)
            y0 = cy + offset * math.sin(angle_rad)
            x1 = x0 + length * math.cos(angle_rad)
            y1 = y0 + length * math.sin(angle_rad)
            color = distance_to_color(dist)

            pygame.draw.line(screen, color, (x0, y0), (x1, y1), 3)
            text = font.render(f"{label.upper()} {dist}mm", True, color)
            text_rect = text.get_rect(center=(x1, y1))
            screen.blit(text, text_rect)

        # === IMU Visualization ===
        imu_ax = latest.get("imu_ax", 0)
        imu_ay = latest.get("imu_ay", 0)
        imu_gz = latest.get("imu_gz", 0)
        imu_temp = latest.get("imu_temp", 0)

        # Gyroscope Z rotation
        rotation_length = 50
        gz_scaled = max(min(imu_gz / 5.0, 1.0), -1.0)
        gz_angle = gz_scaled * math.pi
        gx1 = cx
        gy1 = cy
        gx2 = cx + math.cos(gz_angle) * rotation_length
        gy2 = cy + math.sin(gz_angle) * rotation_length
        pygame.draw.line(screen, (255, 255, 0), (gx1, gy1), (gx2, gy2), 3)
        pygame.draw.circle(screen, (255, 255, 0), (int(gx2), int(gy2)), 5)

        # Accelerometer vector
        accel_scale = 30
        ax = imu_ax * accel_scale
        ay = -imu_ay * accel_scale
        pygame.draw.line(screen, (0, 128, 255), (cx, cy), (cx + ax, cy + ay), 3)
        pygame.draw.circle(screen, (0, 128, 255), (int(cx + ax), int(cy + ay)), 5)

        # IMU labels
        imu_label = font.render(f"IMU: ax={imu_ax:.2f} ay={imu_ay:.2f} gz={imu_gz:.2f}", True, (255, 255, 255))
        screen.blit(imu_label, (10, height - 50))

        imu_temp_label = font.render(f"IMU Temp: {imu_temp:.1f} °C", True, (255, 255, 0))
        screen.blit(imu_temp_label, (10, height - 30))

        # === Current Bars (shifted down) ===
        bar_width = 30
        bar_height = 150
        bar_spacing = 60
        bar_x = cx - bar_spacing
        bar_y = cy + 300  # ⬅️ moved from 220 to 300

        # Front
        pygame.draw.rect(screen, (80, 80, 80), (bar_x, bar_y - bar_height, bar_width, bar_height))
        fill_h = min(smoothed_ina60 * INA_SCALE, bar_height)
        pygame.draw.rect(screen, (0, 255, 0), (bar_x, bar_y - fill_h, bar_width, fill_h))
        text = font.render(f"Front: {last_ina60:.1f} mA", True, (255, 255, 255))
        screen.blit(text, (bar_x - 15, bar_y + 5))

        # Back
        bar_x += bar_spacing + bar_width
        pygame.draw.rect(screen, (80, 80, 80), (bar_x, bar_y - bar_height, bar_width, bar_height))
        fill_h = min(smoothed_ina61 * INA_SCALE, bar_height)
        pygame.draw.rect(screen, (0, 255, 0), (bar_x, bar_y - fill_h, bar_width, fill_h))
        text = font.render(f"Back: {last_ina61:.1f} mA", True, (255, 255, 255))
        screen.blit(text, (bar_x - 10, bar_y + 5))

        pygame.display.flip()
        clock.tick(20)
