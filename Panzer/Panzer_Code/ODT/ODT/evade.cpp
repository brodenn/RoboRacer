#include "evade.h"

//unsigned long avoidUntil = 0;

float calcWeight(uint16_t dist, uint16_t maxRange) {
  if (dist == 0 || dist > maxRange) return 0;
  return 1.0 - ((float)dist / maxRange);
}

void printMotorStatus(int pwmL, int pwmR, bool forwardL, bool forwardR) {
  Serial.println("Motor LEFT:        Motor RIGHT:");

  int percentL = map(pwmL, 0, 255, 0, 100);
  int percentR = map(pwmR, 0, 255, 0, 100);

  if (!forwardL) percentL *= -1;
  if (!forwardR) percentR *= -1;

  Serial.printf("   %4d%%              %4d%%\n", percentL, percentR);
}

void backUp(uint8_t pwmL, uint8_t pwmR, uint16_t duration) {
  Serial.println("🔙 Backar");
  printMotorStatus(pwmL, pwmR, false, false);  // båda kör bakåt
  delay(100); // skydda motorsköld
  motorLeft->setSpeed(pwmL);
  motorRight->setSpeed(pwmR);
  motorLeft->run(BACKWARD);
  motorRight->run(BACKWARD);
  avoidUntil = millis() + duration;
}

void rotateLeft(uint16_t duration) {
  Serial.println("🔄 Roterar vänster");
  printMotorStatus(120, 100, false, true);  // vänster bakåt, höger framåt
  delay(100);
  motorLeft->setSpeed(100);
  motorRight->setSpeed(100);
  motorLeft->run(BACKWARD);
  motorRight->run(FORWARD);
  avoidUntil = millis() + duration;
}

void rotateRight(uint16_t duration) {
  Serial.println("🔄 Roterar höger");
  printMotorStatus(100, 120, true, false);  // vänster framåt, höger bakåt
  delay(100);
  motorLeft->setSpeed(100);
  motorRight->setSpeed(100);
  motorLeft->run(FORWARD);
  motorRight->run(BACKWARD);
  avoidUntil = millis() + duration;
}

void performAvoidance(uint16_t vlL, uint16_t vlR, uint16_t optL, uint16_t optR, uint16_t front) {
  // === Front nära → backa eller rotera
  if (front > 0 && front < 200) {
    Serial.println("🚨 Hinder fram (<200 mm) → backa och välj friaste riktning");

    // Backa först
    backUp(120, 120, 700);
    delay(150);

    // Välj riktning baserat på vilket håll som är friare
    if (optL > optR) {
      rotateLeft(600);
    } else {
      rotateRight(600);
    }

    delay(100);
    return;
  }

  // === Raksträcka
  bool straightPath = (vlL > 200 && vlR > 200 && optL > 400 && optR > 400);
  int baseSpeed = straightPath ? 255 : BASE_SPEED;

  if (straightPath) {
    Serial.println("🛣️ Raksträcka upptäckt → högre hastighet");
  }

  // === Förhandsstyrning (steerBias) vid öppen yta
  float steerBias = 0.0;
  if (optL > 300 && optR > 300) {
    int16_t diff = (int16_t)optR - (int16_t)optL;
    if (abs(diff) > 100) {
      steerBias = constrain(diff / 800.0, -0.1, 0.1);  // max ±0.3 bias
      Serial.print("🧭 Förhandsstyrning (steerBias): ");
      Serial.println(steerBias, 2);
    }
  }

  // === Reaktiv styrning vid hinder
  float steerStrength = 0;

  if (optL < 1000 && vlL > 0 && vlL < 200) {
    Serial.println("⚠️ Nära hinder på vänster sida (OPT+VL) → styr höger");
    steerStrength = -0.6;
  } else if (optR < 1000 && vlR > 0 && vlR < 200) {
    Serial.println("⚠️ Nära hinder på höger sida (OPT+VL) → styr vänster");
    steerStrength = 0.6;
  } else {
    float optWeightL = calcWeight(optL);
    float optWeightR = calcWeight(optR);
    steerStrength = optWeightR - optWeightL + steerBias;
  }

  // === Beräkna PWM med styrkompensation
  steerStrength = constrain(steerStrength, -0.6, 0.6);
  int16_t steer = steerStrength * MAX_CORRECTION;

  int pwmL = constrain(baseSpeed - steer, 0, 255);
  int pwmR = constrain(baseSpeed + steer, 0, 255);

  motorLeft->setSpeed(pwmL);
  motorRight->setSpeed(pwmR);
  motorLeft->run(FORWARD);
  motorRight->run(FORWARD);

  printMotorStatus(pwmL, pwmR, true, true);
}
