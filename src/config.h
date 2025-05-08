#ifndef CONFIG_H
#define CONFIG_H

#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>   

// -----------------------------------------------------------------------------
//                              PIN-KONFIGURATION
// -----------------------------------------------------------------------------

/**
 * @brief Pin tilsluttet DS18B20 temperatursensoren.
 */
#define ONE_WIRE_BUS 4

/**
 * @brief Pin tilsluttet reset-knappen.
 */
#define RESET_BUTTON_PIN 14

// -----------------------------------------------------------------------------
//                              WiFi-KONFIGURATION
// -----------------------------------------------------------------------------

/**
 * @brief Navn på Access Point (AP), hvis ESP32 ikke tidligere har været tilsluttet WiFi.
 */
#define WIFI_AP_NAME "ESP32-Logger"

/**
 * @brief Adgangskode til Access Point (AP).
 */
#define WIFI_AP_PASSWORD "admin123"

// -----------------------------------------------------------------------------
//                              SPIFFS & CSV
// -----------------------------------------------------------------------------

/**
 * @brief Filnavn for CSV-datafil gemt i SPIFFS.
 */
#define DATA_FILENAME "/output.csv"

/**
 * @brief Header til CSV-filen.
 */
constexpr const char* CSV_HEADER = "Time,Temperature";

// -----------------------------------------------------------------------------
//                              GLOBALE VARIABLER
// -----------------------------------------------------------------------------

/**
 * @brief Tidsstempel for hvornår reset-knappen blev trykket ned.
 */
extern unsigned long buttonPressStart;

/**
 * @brief Angiver om reset-sekvensen er startet.
 */
extern bool resetInitiated;

/**
 * @brief OneWire-instans til kommunikation med sensorer.
 */
extern OneWire oneWire;

/**
 * @brief DallasTemperature-instans til håndtering af temperaturmåling.
 */
extern DallasTemperature sensors;

/**
 * @brief Webserver-instans til håndtering af HTTP-forespørgsler.
 */
extern AsyncWebServer server;

/**
 * @brief WebSocket-instans til realtidskommunikation med klienter.
 */
extern AsyncWebSocket ws;

/**
 * @brief Angiver om enheden er blevet nulstillet.
 */
extern bool isReset;

#endif // CONFIG_H
