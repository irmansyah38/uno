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
  bool state;
  int takaran;
  PumpClass(int p, String nm, bool ety, bool st, int tk);
  void initialize();
  void on();
  void off();
};

#endif