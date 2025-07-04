#ifndef STEERING_H
#define STEERING_H

#include <stdint.h>

void correctCourse();
float getManualSteeringBias();
void setManualSteeringBias(float bias);  // <- ny

#endif
