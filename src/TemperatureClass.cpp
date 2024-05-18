#include "TemperatureClass.h"
DS18B20Sensor ::DS18B20Sensor(int pin) : ONE_WIRE_BUS(pin), oneWire(ONE_WIRE_BUS), sensors(&oneWire) {}

void DS18B20Sensor::begin()
{
  sensors.begin();
}

float DS18B20Sensor::getTemperature()
{
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);
  return temperatureC;
}
