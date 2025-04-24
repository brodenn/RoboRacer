#include "globals.h"
#include "params.h"

// ----------------------
// WiFi
// ----------------------

const char* ssid = "ESP32_Hotspot";
const char* password = "12345678";
WiFiUDP udp;
const char* laptop_ip = "192.168.4.2";
const int udp_port = 12345;
SemaphoreHandle_t wifiSemaphore = nullptr;

// ----------------------
// I2C
// ----------------------

SemaphoreHandle_t i2cBusy = nullptr;
SemaphoreHandle_t i2cBusyWire0 = nullptr;
SemaphoreHandle_t i2cBusyWire1 = nullptr;

// ----------------------
// Motor
// ----------------------

int target_m1 = 0, target_m2 = 0, target_m3 = 0, target_m4 = 0;
int current_m1 = 0, current_m2 = 0, current_m3 = 0, current_m4 = 0;
const int MAX_STEP = 128;
const int motorUpdateInterval = 200;

Adafruit_MotorShield AFMS_60(0x60);
Adafruit_MotorShield AFMS_61(0x61);

Adafruit_DCMotor* motor1 = nullptr;
Adafruit_DCMotor* motor2 = nullptr;
Adafruit_DCMotor* motor3 = nullptr;
Adafruit_DCMotor* motor4 = nullptr;

bool motorsEnabled = true;

// ----------------------
// Sensors (VL53L4CD)
// ----------------------

VL53L4CD sensor(&Wire, -1);
uint16_t distances[NUM_SENSORS] = {0};
bool allSensorsFailed = false;
int sensorFailureCount[NUM_SENSORS] = {0};
bool needsRestart[NUM_SENSORS] = {false};

const int SENSOR_FAILURE_THRESHOLD = 5;

// --- Sensor GPIO interrupt mapping (MATCHING muxMap[]) ---
const uint8_t SENSOR_GPIO[NUM_SENSORS] = {
    32,  // Sensor 0: Left
    25,  // Sensor 1: Front
    27,  // Sensor 2: Front-Right
     4,  // Sensor 3: Front-Left
    19,  // Sensor 4: Back-Left
    33,  // Sensor 5: Right
     5,  // Sensor 6: Back
    26   // Sensor 7: Back-Right
};

// --- Trigger flags updated via ISR ---
volatile bool sensorTriggered[NUM_SENSORS] = {false};

// ----------------------
// Reverse Mode
// ----------------------

bool reversing = false;
unsigned long reverseStart = 0;
unsigned long closeFrontStart = 0;
const int stuckDuration = 1200;
const int reverseTime = 500;

// ----------------------
// Zigzag Locking
// ----------------------

int headingHoldDir = 0;
unsigned long headingLockStart = 0;
const int headingLockDuration = 700;

// ----------------------
// Sensor Thresholds
// ----------------------

int MAX_RELIABLE_SIGMA = 20;
int MIN_SIGNAL_THRESHOLD = 600;
int MAX_VALID_DISTANCE = 1300;
int INVALID_DISTANCE = 2000;

// ----------------------
// INA219
// ----------------------

Adafruit_INA219 ina60(0x40);
Adafruit_INA219 ina61(0x40);

// ----------------------
// Timing
// ----------------------

unsigned long lastSensorRead = 0;
unsigned long lastDataSend = 0;
const int sensorInterval = 10;     // Polling rate
const int sendInterval = 250;      // Master send interval for all UDP data

// ----------------------
// Mode Control
// ----------------------

ControlMode controlMode = MODE_AUTONOMOUS;
SteeringParams params;

// ----------------------
// IMU
// ----------------------

Adafruit_LSM6DSOX imu;
float imu_ax = 0;
float imu_ay = 0;
float imu_az = 0;
float imu_gx = 0;
float imu_gy = 0;
float imu_gz = 0;
float imu_temp = 0;

// ----------------------
// Loop Counter
// ----------------------

unsigned long loopCounter = 0;
