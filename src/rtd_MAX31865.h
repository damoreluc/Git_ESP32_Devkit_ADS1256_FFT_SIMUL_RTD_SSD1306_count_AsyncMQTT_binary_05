#ifndef _RTD_MAX31865_H
#define _RTD_MAX31865_H
#include <Adafruit_MAX31865.h>

#define MSG_FAULT_LEN 40
// return true if measure is valid
// else false in case of hardware problems
// faultMsg holds fault reason
bool getTemperature(Adafruit_MAX31865 &rtd, float *temperature, char *faultMsg);

#endif