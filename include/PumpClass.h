#ifndef PumpClass_H
#define PumpClass_H

#include <Arduino.h>
#include <Wire.h>

class PumpClass
{

public:
  int pin;
  String name;
  bool empty;
  PumpClass(int p, String nm, bool ety);
  void initialize();
  void on();
  void off();
};

#endif