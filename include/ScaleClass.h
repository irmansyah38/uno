#ifndef SCALECLASS_H
#define SCALECLASS_H

#include "HX711.h"
class ScaleClass
{
private:
  HX711 scale;
  float calibration_factor;

public:
  ScaleClass(int doutPin, int clkPin, float calFactor);
  void initialize();
  int getWeight();
  void scaleOn();
  void scaleOff();
};

#endif