#ifndef WIFI_HANDLER_H
#define WIFI_HANDLER_H

#include "config.h"

/**
 * @brief Initialiserer WiFi-forbindelse med WiFiManager.
 *
 * Funktionen forsøger automatisk at forbinde til tidligere kendte netværk.
 * Hvis forbindelsen fejler, starter den en konfigurationsportal for manuel opsætning.
 * Ved fejl genstartes enheden automatisk.
 */
void initWiFi();

#endif // WIFI_HANDLER_H
