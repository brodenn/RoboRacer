#include "evade.h"  // Inkluderar funktioner och variabler för hinderundvikning

//unsigned long avoidUntil = 0;  // Flyttad till global deklaration i annan fil

// === Funktion för att vikta avståndsmätningar ===
// Returnerar ett viktat värde (0–1) baserat på hur nära objektet är
float calcWeight(uint16_t dist, uint16_t maxRange) {
  if (dist == 0 || dist > maxRange) return 0;            // Ogiltig eller utanför räckvidd
  return 1.0 - ((float)dist / maxRange);                 // Närmare objekt → högre vikt
}

// === Skriver ut aktuell motorstatus till Serial Monitor ===
void printMotorStatus(int pwmL, int pwmR, bool forwardL, bool forwardR) {
  Serial.println("Motor LEFT:        Motor RIGHT:");

  int percentL = map(pwmL, 0, 255, 0, 100);
  int percentR = map(pwmR, 0, 255, 0, 100);

  if (!forwardL) percentL *= -1;  // Om bakåt, negativ procentsats
  if (!forwardR) percentR *= -1;

  Serial.printf("   %4d%%              %4d%%\n", percentL, percentR);
}

// === Backar med olika hastighet på vänster och höger motor ===
void backUp(uint8_t pwmL, uint8_t pwmR, uint16_t duration) {
  Serial.println("🔙 Backar");
  printMotorStatus(pwmL, pwmR, false, false);  // Visa status
  delay(100);  // Skydda motorskölden
  motorLeft->setSpeed(pwmL);
  motorRight->setSpeed(pwmR);
  motorLeft->run(BACKWARD);
  motorRight->run(BACKWARD);
  avoidUntil = millis() + duration;  // Vänta en stund innan ny styrning
}

// === Rotera vänster på plats ===
void rotateLeft(uint16_t duration) {
  Serial.println("🔄 Roterar vänster");
  printMotorStatus(120, 100, false, true);  // Visa status
  delay(100);
  motorLeft->setSpeed(100);
  motorRight->setSpeed(100);
  motorLeft->run(BACKWARD);
  motorRight->run(FORWARD);
  avoidUntil = millis() + duration;
}

// === Rotera höger på plats ===
void rotateRight(uint16_t duration) {
  Serial.println("🔄 Roterar höger");
  printMotorStatus(100, 120, true, false);  // Visa status
  delay(100);
  motorLeft->setSpeed(100);
  motorRight->setSpeed(100);
  motorLeft->run(FORWARD);
  motorRight->run(BACKWARD);
  avoidUntil = millis() + duration;
}

// === Huvudfunktion för hinderundvikning ===
void performAvoidance(uint16_t vlL, uint16_t vlR, uint16_t optL, uint16_t optR, uint16_t front) {
  // === Om hinder rakt fram: backa och välj friaste riktning ===
  if (front > 0 && front < 200) {
    Serial.println("🚨 Hinder fram (<200 mm) → backa och välj friaste riktning");

    delay(100);

    if (optL < optR) {
      // Hinder vänster – backa åt höger
      backUp(100, 50, BACKUP_DURATION);
    } else {
      // Hinder höger eller lika – backa åt vänster
      backUp(50, 100, BACKUP_DURATION);
    }

    delay(100);

    // Roterar mot det håll som har mest utrymme
    if (optL > optR) {
      rotateLeft(600);
    } else {
      rotateRight(600);
    }

    delay(100);
    return;  // Avsluta funktionen
  }

  // === Om inget hinder fram – kolla om raksträcka för ökad hastighet ===
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

  // === Förhandsstyrning baserat på opt-avstånd om båda sidor är öppna ===
  float steerBias = 0.0;
  if (optL > 350 && optR > 350) {
    int16_t diff = (int16_t)optR - (int16_t)optL;
    if (abs(diff) > 50) {
      steerBias = constrain(diff / 500.0, -0.25, 0.25);  // Begränsad till ±0.25
      Serial.print("🧭 Förhandsstyrning (steerBias): ");
      Serial.println(steerBias, 2);
    }
  }

  // === Reaktiv styrning vid hinder nära ena sidan ===
  float steerStrength = 0;

  if ((optL < OBSTACLE_CLOSE_OPT && vlL > 0 && vlL < OBSTACLE_CLOSE_VL) &&
      (optL + 100 < optR)) {
    Serial.println("⚠️ Tydligt närmare vänster sida → styr höger");
    steerStrength = -0.6;
  } else if ((optR < OBSTACLE_CLOSE_OPT && vlR > 0 && vlR < OBSTACLE_CLOSE_VL) &&
             (optR + 100 < optL)) {
    Serial.println("⚠️ Tydligt närmare höger sida → styr vänster");
    steerStrength = 0.6;
  } else {
    float optWeightL = calcWeight(optL);  // Vikta vänster
    float optWeightR = calcWeight(optR);  // Vikta höger
    steerStrength = optWeightR - optWeightL + steerBias;  // Skillnad = styrning
  }

  // === Omräkning till PWM-skillnad och begränsning ===
  steerStrength = constrain(steerStrength, -0.6, 0.6);
  int16_t steer = steerStrength * MAX_CORRECTION;

  // Vid skarp sväng – sänk grundhastigheten
  if (abs(steerStrength) > 0.5) {
    baseSpeed -= 30;
  }

  // Räkna ut PWM för båda motorerna
  int pwmL = constrain(baseSpeed - steer, 0, 255);
  int pwmR = constrain(baseSpeed + steer, 0, 255);

  // Skicka till motorerna
  motorLeft->setSpeed(pwmL);
  motorRight->setSpeed(pwmR);
  motorLeft->run(FORWARD);
  motorRight->run(FORWARD);

  printMotorStatus(pwmL, pwmR, true, true);  // Visa motorstatus
}
