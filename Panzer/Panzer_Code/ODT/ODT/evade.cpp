#include "evade.h"  // Inkluderar funktioner och variabler för hinderundvikning
#include "steering.h"

bool dirLeftForward = true;
bool dirRightForward = true;

// === Globala PWM-värden för visning på TFT ===
int currentPWM_L = 0;
int currentPWM_R = 0;

// === Funktion för att vikta avståndsmätningar ===
float calcWeight(uint16_t dist, uint16_t maxRange) {
  if (dist == 0 || dist > maxRange) return 0;
  return 1.0 - ((float)dist / maxRange);
}

// === Skriver ut aktuell motorstatus till Serial Monitor ===
void printMotorStatus(int pwmL, int pwmR, bool forwardL, bool forwardR) {
  Serial.println("Motor LEFT:        Motor RIGHT:");
  int percentL = map(pwmL, 0, 255, 0, 100);
  int percentR = map(pwmR, 0, 255, 0, 100);
  if (!forwardL) percentL *= -1;
  if (!forwardR) percentR *= -1;
  Serial.printf("   %4d%%              %4d%%\n", percentL, percentR);
}

// === Säker funktion för riktningbyte ===
void safeDirectionChange(uint8_t dirLeft, uint8_t dirRight, uint8_t pwmL, uint8_t pwmR) {
  motorLeft->setSpeed(0);
  motorRight->setSpeed(0);
  motorLeft->run(RELEASE);
  motorRight->run(RELEASE);
  dirLeftForward  = (dirLeft == FORWARD);
  dirRightForward = (dirRight == FORWARD);
  delay(5);

  motorLeft->run(dirLeft);
  motorRight->run(dirRight);
  motorLeft->setSpeed(pwmL);
  motorRight->setSpeed(pwmR);

  // Uppdatera globala PWM-värden
  currentPWM_L = pwmL;
  currentPWM_R = pwmR;
}

// === Backar med olika hastighet på vänster och höger motor ===
void backUp(uint8_t pwmL, uint8_t pwmR, uint16_t duration) {
  Serial.println("🔙 Backar");
  printMotorStatus(pwmL, pwmR, false, false);
  safeDirectionChange(BACKWARD, BACKWARD, pwmL, pwmR);
  avoidUntil = millis() + duration;
}

// === Rotera vänster på plats ===
void rotateLeft(uint16_t duration) {
  Serial.println("🔄 Roterar vänster");
  int pwmL = 100, pwmR = 100;
  printMotorStatus(pwmL, pwmR, false, true);
  safeDirectionChange(BACKWARD, FORWARD, pwmL, pwmR);
  avoidUntil = millis() + duration;
}

// === Rotera höger på plats ===
void rotateRight(uint16_t duration) {
  Serial.println("🔄 Roterar höger");
  int pwmL = 100, pwmR = 100;
  printMotorStatus(pwmL, pwmR, true, false);
  safeDirectionChange(BACKWARD, FORWARD, pwmL, pwmR);
  avoidUntil = millis() + duration;
}

void performAvoidance(uint16_t vlL, uint16_t vlR, uint16_t optL, uint16_t optR, uint16_t front) {
  Serial.println("➡️ performAvoidance() anropad");
  // === Filtrera bort ogiltiga värden ===
  if (front == INVALID_DISTANCE && optL == INVALID_DISTANCE && optR == INVALID_DISTANCE) {
    Serial.println("🛑 Panikläge: Alla front- och sidovärden är ogiltiga – STOPP");
    motorLeft->run(RELEASE);
    motorRight->run(RELEASE);
    delay(10);
    return;
  }

  // === Kontroll: Hinder framför? ===
  if (front != INVALID_DISTANCE && front < 300) {
    Serial.println("🚨 Hinder fram (<300 mm) → backa och välj friaste riktning");
    Serial.print(">>> FRONT: ");
    Serial.println(front);

    if (optL != INVALID_DISTANCE && optR != INVALID_DISTANCE) {
      if (optL < optR) {
        backUp(150, 100, BACKUP_DURATION);
      } else {
        backUp(100, 150, BACKUP_DURATION);
      }

      if (optL > optR) {
        rotateLeft(800);
      } else {
        rotateRight(800);
      }
    } else {
      Serial.println("⚠️ Inga giltiga sidovärden från OPT – ingen rotation utförd");
    }

    return;
  }

  // === Raksträcka? ===
  bool straightPath = (
    vlL > BOOST_THRESH_VL &&
    vlR > BOOST_THRESH_VL &&
    optL > BOOST_THRESH_OPT &&
    optR > BOOST_THRESH_OPT
  );

  int baseSpeed = straightPath ? BOOST_SPEED : BASE_SPEED;

  if (straightPath) {
    Serial.println("🛣️ Raksträcka upptäckt → högre hastighet");
  }

  float steerBias = 0.0;
  if (optL > 350 && optR > 350) {
    int16_t diff = (int16_t)optR - (int16_t)optL;
    if (abs(diff) > 50) {
      steerBias = constrain(diff / 500.0, -0.25, 0.25);
      Serial.print("🧭 Förhandsstyrning (steerBias): ");
      Serial.println(steerBias, 2);
    }
  }

  // === Reaktiv styrning vid hinder nära någon sida ===
  float steerStrength = 0;

  if (optL < OBSTACLE_CLOSE_OPT && optR > OBSTACLE_CLOSE_OPT + 50) {
    Serial.println("⚠️ OPT: Närmare vänster sida → styr höger");
    steerStrength = -0.6;
  } else if (optR < OBSTACLE_CLOSE_OPT && optL > OBSTACLE_CLOSE_OPT + 50) {
    Serial.println("⚠️ OPT: Närmare höger sida → styr vänster");
    steerStrength = 0.6;
  } else if (vlL > 0 && vlL < OBSTACLE_CLOSE_VL && vlR > OBSTACLE_CLOSE_VL + 50) {
    Serial.println("⚠️ VL53: Hinder vänster → styr höger");
    steerStrength = -0.5;
  } else if (vlR > 0 && vlR < OBSTACLE_CLOSE_VL && vlL > OBSTACLE_CLOSE_VL + 50) {
    Serial.println("⚠️ VL53: Hinder höger → styr vänster");
    steerStrength = 0.5;
  } else {
    // Viktad opt-baserad styrning + manuell bias
    float optWeightL = calcWeight(optL);
    float optWeightR = calcWeight(optR);
    steerStrength = optWeightR - optWeightL + getManualSteeringBias();
  }

  steerStrength = constrain(steerStrength, -0.6, 0.6);
  int16_t steer = steerStrength * MAX_CORRECTION;

  if (abs(steerStrength) > 0.5) {
    baseSpeed -= 30;
  }

  int pwmL = constrain(baseSpeed - steer, 0, 255);
  int pwmR = constrain(baseSpeed + steer, 0, 255);

  motorLeft->setSpeed(pwmL);
  motorRight->setSpeed(pwmR);
  motorLeft->run(FORWARD);
  motorRight->run(FORWARD);

  // Uppdatera globala PWM-värden för TFT
  currentPWM_L = pwmL;
  currentPWM_R = pwmR;

  printMotorStatus(pwmL, pwmR, true, true);
}
// === WebSocket-stödjande kontrollfunktioner ===
void setLeftSpeed(int percent) {
  percent = constrain(percent, -100, 100);
  currentPWM_L = map(abs(percent), 0, 100, 0, 255);
  dirLeftForward = (percent >= 0);
  motorLeft->setSpeed(currentPWM_L);
  motorLeft->run(dirLeftForward ? FORWARD : BACKWARD);
}

void setRightSpeed(int percent) {
  percent = constrain(percent, -100, 100);
  currentPWM_R = map(abs(percent), 0, 100, 0, 255);
  dirRightForward = (percent >= 0);
  motorRight->setSpeed(currentPWM_R);
  motorRight->run(dirRightForward ? FORWARD : BACKWARD);
}


void moveForward() {
  motorLeft->setSpeed(currentPWM_L);
  motorRight->setSpeed(currentPWM_R);
  motorLeft->run(FORWARD);
  motorRight->run(FORWARD);
}

void moveBackward() {
  motorLeft->setSpeed(currentPWM_L);
  motorRight->setSpeed(currentPWM_R);
  motorLeft->run(BACKWARD);
  motorRight->run(BACKWARD);
}

void turnLeft() {
  motorLeft->setSpeed(currentPWM_L);
  motorRight->setSpeed(currentPWM_R);
  motorLeft->run(BACKWARD);
  motorRight->run(FORWARD);
}

void turnRight() {
  motorLeft->setSpeed(currentPWM_L);
  motorRight->setSpeed(currentPWM_R);
  motorLeft->run(FORWARD);
  motorRight->run(BACKWARD);
}

void stopOrReverse() {
  motorLeft->run(RELEASE);
  motorRight->run(RELEASE);
}

