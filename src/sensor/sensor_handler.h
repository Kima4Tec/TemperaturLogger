#ifndef SENSOR_HANDLER_H
#define SENSOR_HANDLER_H

#include <DallasTemperature.h>
#include <OneWire.h>
#include "config.h"

/**
 * @brief Henter den aktuelle lokale tid som en formateret streng.
 *
 * Funktionen gemmer det aktuelle tidspunkt i formatet "YYYY-MM-DD HH:MM:SS"
 * i det angivne `timeString`-array.
 *
 * @param timeString Peger på en buffer (mindst 20 tegn), hvor det formatterede tidspunkt gemmes.
 */
void getFormattedTime(char *timeString);

/**
 * @brief Logger den aktuelle temperatur til en CSV-fil.
 *
 * Funktionen læser temperaturen fra den tilsluttede Dallas-temperatursensor og
 * tilføjer en ny linje med tid og temperatur til CSV-filen.
 *
 * @param sensors Reference til DallasTemperature-objektet, der håndterer temperatursensoren.
 * @param timeString Den aktuelle tid som formateret streng.
 */
void logTemperatureToCSV(DallasTemperature &sensors, char *timeString);

/**
 * @brief Udskriver indholdet af CSV-logfilen til Serial Monitor.
 *
 * Bruges til fejlsøgning og kontrol af de loggede data.
 */
void printLog();

#endif
