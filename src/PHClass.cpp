// #include "PHClass.h"

// PHClass::PHClass(int pi, float p4, float p7) : pin(pi), ph4(p4), ph7(p7) {}

// void PHClass::initialize()
// {
//   pinMode(pin, INPUT);
// }

// float PHClass::getValue()
// {
//   nilaiAnalogPh = analogRead(pin);
//   Tegangan = 3.3 / 4095.0 * nilaiAnalogPh;
//   phStep = (ph4 - ph7) / 3;
//   po = 7.00 + ((ph7 - Tegangan) / phStep);
//   return po;
// }