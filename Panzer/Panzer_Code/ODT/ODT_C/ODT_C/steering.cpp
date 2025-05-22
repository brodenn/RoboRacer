#include "steering.h"      // Deklaration för correctCourse()
#include "evade.h"         // Innehåller motorLeft & motorRight
#include "opt.h"           // Innehåller optDistances[]
#include "mux.h"           // Innehåller vl53Distances[]

// === Funktion för att vikta VL53-avstånd (liknande opt-viktning) ===
// Returnerar ett viktat värde mellan 0.0 (noll) och 1.0 (närmast)
float calcVLWeight(uint16_t dist, uint16_t maxRange) {
  if (dist == 0 || dist > maxRange) return 0.0;        // Ogiltigt eller utanför räckvidd
  return 1.0 - ((float)dist / maxRange);               // Närmare → högre vikt
}

// === Kurskorrigering vid fri sikt framåt ===
void correctCourse() {
  const int baseSpeed = 150;                // Grundhastighet vid normal drift
  const int maxCorrection = 25;             // Max hastighetsändring vid styrning
  const uint16_t frontSafeDist = 700;       // Minsta säkra avstånd framåt (OPT)
  const uint16_t vlRangeLimit = 200;        // VL53 maxavstånd för styrkorrigering

  // Om det är fritt framåt – korrigera kurs med sidovärden från VL53
  if (optDistances[1] > frontSafeDist) {
    float leftWeight  = calcVLWeight(vl53Distances[0], vlRangeLimit);  // Vänster vikt
    float rightWeight = calcVLWeight(vl53Distances[1], vlRangeLimit);  // Höger vikt

    float steerStrength = rightWeight - leftWeight;                    // Positiv = högerdrift
    int correction = (int)(steerStrength * maxCorrection);            // Max ±25 PWM-enheter

    // Justera hastighet baserat på kurskorrigering
    int pwmL = constrain(baseSpeed + correction, 0, 255);              // Öka vänster om kursen drar höger
    int pwmR = constrain(baseSpeed - correction, 0, 255);              // Minska höger

    motorLeft->setSpeed(pwmL);
    motorRight->setSpeed(pwmR);
    motorLeft->run(FORWARD);
    motorRight->run(FORWARD);
  } else {
    // Om för nära objekt framåt – kör full undanmanöver istället
    performAvoidance(
      vl53Distances[0], vl53Distances[1],   // VL53 vänster, höger
      optDistances[0], optDistances[2],     // OPT vänster, höger
      optDistances[1]                       // OPT front
    );
  }
}
