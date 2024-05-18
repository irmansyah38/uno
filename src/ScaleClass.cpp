#include "ScaleClass.h"

ScaleClass::ScaleClass(int doutPin, int clkPin, float calFactor)
    : scale(doutPin, clkPin), calibration_factor(calFactor) {}

void ScaleClass::initialize()
{

  scale.set_scale();
  scale.tare();
}

int ScaleClass::getWeight()
{
  scale.set_scale(calibration_factor);
  int GRAM = scale.get_units();
  return GRAM;
}

void ScaleClass::scaleOn()
{
  scale.power_up();
}

void ScaleClass::scaleOff()
{
  scale.power_down();
}