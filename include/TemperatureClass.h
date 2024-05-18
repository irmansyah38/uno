#ifndef TEMPERATURECLASS_H
#define TEMPERATURECLASS_H

#include <OneWire.h>
#include <DallasTemperature.h>

class DS18B20Sensor
{
private:
  const int ONE_WIRE_BUS;
  OneWire oneWire;
  DallasTemperature sensors;

public:
  DS18B20Sensor(int pin);
  void begin();
  float getTemperature();
};

#endif