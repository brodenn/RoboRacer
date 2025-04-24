#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <Bluepad32.h>
#include "globals.h"   // ✅ You MUST include this to get ControlMode

// --- Controller Instance ---
extern ControllerPtr myController;

// --- Controller Functions ---
void controllerSetup();
void handleController();
void handleUdpMode();

// --- Optional: Mode to string ---
inline const char* controlModeToString() {
    return (controlMode == MODE_AUTONOMOUS) ? "AUTONOMOUS" : "CONTROLLER";
}

#endif
