#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include "sensor_handler.h"
#include "csv/csv_handler.h"

/**
 * @file logger.cpp
 * @brief Indeholder funktioner til at hente og logge temperaturdata samt sende dem via WebSocket.
 */

/// Filnavn for CSV-logfilen (defineret andetsteds)
extern const char *filename;

/**
 * @brief Henter og formaterer den aktuelle tid som en streng.
 * 
 * @param[out] timeString En buffer hvor den formaterede tid (YYYY-MM-DD HH:MM:SS) gemmes.
 * 
 * Denne funktion venter indtil gyldig tid er tilgængelig via NTP og formaterer den til en læsbar streng.
 */
void getFormattedTime(char *timeString) {
    struct tm timeinfo;
    while (!getLocalTime(&timeinfo)) {
        Serial.println("Venter på gyldig tid...");
        delay(500);
    }
    strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);
}

/**
 * @brief Logger temperaturen og det aktuelle tidspunkt til en CSV-fil.
 * 
 * @param sensors En reference til DallasTemperature-sensorobjektet.
 * @param timeString En streng med den aktuelle tid.
 * 
 * Funktionen venter på en gyldig temperaturmåling og logger derefter både tidspunkt og temperatur til fil.
 */
void logTemperatureToCSV(DallasTemperature &sensors, char *timeString) {
    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0);

    while (tempC == DEVICE_DISCONNECTED_C || tempC == -127.0) {
        Serial.println("Venter på gyldig temperatur...");
        delay(500);
        sensors.requestTemperatures();
        tempC = sensors.getTempCByIndex(0);
    }

    appendToCSV(filename, timeString, tempC);
}

/**
 * @brief Læser og logger temperaturdata til både Serial og CSV samt sender dem via WebSocket.
 * 
 * Funktionen henter det aktuelle tidspunkt og temperatur, skriver dem til Serial og CSV,
 * og sender temperaturværdien som JSON til alle tilsluttede WebSocket-klienter.
 */
void printLog() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to get time");
        return;
    }

    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0);

    char logTime[64];
    strftime(logTime, sizeof(logTime), "%Y-%m-%d %H:%M:%S", &timeinfo);

    Serial.println(String(logTime) + " - Temperatur: " + String(tempC, 2) + " ºC");
    appendToCSV(filename, logTime, tempC);

    String message = "{\"temperature\": " + String(tempC, 2) + "}";
    ws.textAll(message);  ///< Sender til alle forbundne WebSocket-klienter
}
