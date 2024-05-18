#include "PumpClass.h"

PumpClass::PumpClass(int p, String nm, bool ety, bool st, int tk) : pin(p), name(nm), empty(ety), state(st), takaran(tk) {}

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