#ifndef TIME_HANDLER_H
#define TIME_HANDLER_H

#include <Arduino.h>

/**
 * @brief Sætter tidszonen for ESP32-systemet.
 *
 * Funktionen konfigurerer miljøvariablen for tidszone, så korrekt lokal tid
 * kan hentes via NTP.
 *
 * @param timezone Tidszone-streng i POSIX-format, f.eks. "CET-1CEST,M3.5.0/2,M10.5.0/3".
 */
void setTimezone(String timezone);

/**
 * @brief Initialiserer NTP-tidsfunktionen med den angivne tidszone.
 *
 * Funktionen sætter tidszonen og starter tidssynkronisering via NTP-servere.
 * Skal kaldes i setup().
 *
 * @param timezone Tidszone-streng i POSIX-format.
 */
void initTime(String timezone);

#endif
