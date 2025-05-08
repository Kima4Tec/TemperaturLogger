#ifndef SENSOR_HANDLER_H
#define SENSOR_HANDLER_H

#include <DallasTemperature.h>
#include <OneWire.h>
#include "config.h"

// Funktionsdeklarationer
void getFormattedTime(char *timeString);
void logTemperatureToCSV(DallasTemperature &sensors, char *timeString);
void printLog(); 

#endif