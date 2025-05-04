# RoboRacer

# Omnix

The `omnix` robot is one of the vehicles in the RoboRacer project, equipped with omni wheels for enhanced maneuverability. It serves as a core component of the project, handling various functionalities such as sensor management, motor control, AI steering, and communication.

## Directory Structure

- **`ai.cpp` / `ai.h`**: Implements AI steering logic for autonomous navigation.
- **`controller.cpp` / `controller.h`**: Manages gamepad/controller input.
- **`globals.cpp` / `globals.h`**: Contains global variables and constants used across the module.
- **`imu.cpp` / `imu.h`**: Handles IMU (Inertial Measurement Unit) initialization and data reading.
- **`interrupts.cpp` / `interrupts.h`**: Configures and manages sensor interrupts.
- **`motors.cpp` / `motors.h`**: Controls motor initialization and updates.
- **`mux.cpp` / `mux.h`**: Manages the I2C multiplexer for sensor communication.
- **`params.cpp` / `params.h`**: Defines and manages parameters for steering and sensor thresholds.
- **`sensors.cpp` / `sensors.h`**: Handles sensor initialization, reading, and management.
- **`tasks.cpp` / `tasks.h`**: Defines FreeRTOS tasks for handling sensors, motors, and auxiliary logic.
- **`udp_handler.cpp` / `udp_comm.h`**: Handles UDP communication for parameter updates and telemetry.
- **`udp_sender.cpp`**: Sends telemetry data over UDP.
- **`wifi_setup.cpp` / `wifi_setup.h`**: Configures WiFi and UDP communication.
- **`xshut.cpp` / `xshut.h`**: Manages sensor power control via GPIO.

## Key Features

- **Omni Wheels**: Enables smooth and precise movement in any direction.
- **Sensor Management**: Supports multiple VL53L4CD sensors via I2C multiplexers.
- **Motor Control**: Smooth motor updates and autonomous driving logic.
- **AI Steering**: Implements algorithms for wall centering, curve anticipation, and obstacle avoidance.
- **Communication**: Real-time telemetry and parameter updates via UDP.
- **IMU Integration**: Reads acceleration, gyroscope, and temperature data.

## Setup Instructions

1. Initialize the I2C buses and configure the multiplexer.
2. Set up sensors, motors, and WiFi.
3. Start FreeRTOS tasks for sensor reading, motor control, and auxiliary logic.

## Usage

- **Autonomous Mode**: The AI steering logic takes control of the motors based on sensor data.
- **Controller Mode**: A gamepad can be used to manually control the motors.

## Dependencies

- [ArduinoJson](https://arduinojson.org/)
- [Adafruit Motor Shield Library](https://github.com/adafruit/Adafruit_Motor_Shield_V2_Library)
- [Adafruit LSM6DSOX Library](https://github.com/adafruit/Adafruit_LSM6DSOX)
- [Bluepad32](https://github.com/ricardoquesada/Bluepad32)
- [VL53L4CD Library](https://github.com/stm32duino/VL53L4CD)
