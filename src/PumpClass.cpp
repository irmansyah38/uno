#include "PumpClass.h"

PumpClass::PumpClass(int p, String nm, bool ety) : pin(p), name(nm), empty(ety) {}

void PumpClass::initialize()
{
  pinMode(pin, OUTPUT);
}

void PumpClass::on()
{
  digitalWrite(pin, HIGH);
}

void PumpClass::off()
{
  digitalWrite(pin, LOW);
}