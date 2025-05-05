#include "controller.h"
#include <Bluepad32.h>
#include "globals.h"
#include "motors.h"

// Pointer to the connected controller
ControllerPtr myController = nullptr;

// Deadzone threshold
const int DEADZONE = 70;

// Track previous connection state
static bool wasConnected = false;

// Called on connection
void onConnected(ControllerPtr ctl) {
    myController = ctl;

    if (controlMode == MODE_AUTONOMOUS) {
        controlMode = MODE_CONTROLLER;
    }

    wasConnected = true;
}

// Called on disconnection
void onDisconnected(ControllerPtr ctl) {
    myController = nullptr;
    wasConnected = false;
}

// Setup controller
void controllerSetup() {
    BP32.setup(&onConnected, &onDisconnected);
}

void handleController() {
    BP32.update();

    if (!myController || !myController->isConnected()) return;

    // Joystick values
    int y = myController->axisY();
    int x = myController->axisX();

    // Apply deadzone
    if (abs(x) < DEADZONE) x = 0;
    if (abs(y) < DEADZONE) y = 0;

    // Arcade-style drive logic
    int new_m1 = constrain(y + x, -255, 255);
    int new_m2 = constrain(y - x, -255, 255);
    int new_m3 = constrain(y - x, -255, 255);
    int new_m4 = constrain(y + x, -255, 255);

    // === Debug prints ===
    Serial.printf("🎮 Controller Input → X: %4d | Y: %4d\n", x, y);
    Serial.printf("➡️ Target Motors → M1: %4d | M2: %4d | M3: %4d | M4: %4d\n",
                  new_m1, new_m2, new_m3, new_m4);

    // Update global motor targets
    target_m1 = new_m1;
    target_m2 = new_m2;
    target_m3 = new_m3;
    target_m4 = new_m4;

    // Thread-safe motor update
    if (xSemaphoreTake(i2cBusyWire1, pdMS_TO_TICKS(5)) == pdTRUE) {
        smoothMotorUpdate();
        xSemaphoreGive(i2cBusyWire1);
    }
}

