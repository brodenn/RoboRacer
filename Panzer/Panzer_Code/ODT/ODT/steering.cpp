#include "steering.h"
#include "evade.h"     // <-- Här finns motorLeft & motorRight
#include "opt.h"
#include "mux.h"

float calcVLWeight(uint16_t dist, uint16_t maxRange) {
  if (dist == 0 || dist > maxRange) return 0.0;
  return 1.0 - ((float)dist / maxRange);
}

void correctCourse() {
  const int baseSpeed = 150;
  const int maxCorrection = 25; // Max ±10% av 255
  const uint16_t frontSafeDist = 700;
  const uint16_t vlRangeLimit = 200;

  if (optDistances[1] > frontSafeDist) {
    float leftWeight  = calcVLWeight(vl53Distances[0], vlRangeLimit);
    float rightWeight = calcVLWeight(vl53Distances[1], vlRangeLimit);

    float steerStrength = rightWeight - leftWeight;
    int correction = (int)(steerStrength * maxCorrection);

    int pwmL = constrain(baseSpeed + correction, 0, 255);
    int pwmR = constrain(baseSpeed - correction, 0, 255);

    motorLeft->setSpeed(pwmL);
    motorRight->setSpeed(pwmR);
    motorLeft->run(FORWARD);
    motorRight->run(FORWARD);
  } else {
    performAvoidance(
      vl53Distances[0], vl53Distances[1],
      optDistances[0], optDistances[2],
      optDistances[1]
    );
  }
}
