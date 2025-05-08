#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h> 
#include "sensor_handler.h"
#include "csv/csv_handler.h" 

extern const char *filename;

/**
 * @brief Hent og formatér den aktuelle tid.
 */
void getFormattedTime(char *timeString) {
    struct tm timeinfo;
    while (!getLocalTime(&timeinfo)) {
        Serial.println("Venter på gyldig tid...");
        delay(500);
    }

    // Formatér tiden
    strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);
}

/**
 * @brief Log temperatur og tid til CSV.
 */
void logTemperatureToCSV(DallasTemperature &sensors, char *timeString) {
    // Vent på gyldig temperatur
    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0);
    while (tempC == DEVICE_DISCONNECTED_C || tempC == -127.0) {
        Serial.println("Venter på gyldig temperatur...");
        delay(500);
        sensors.requestTemperatures();
        tempC = sensors.getTempCByIndex(0);
    }

    // Log temperatur og tid til CSV
    appendToCSV(filename, timeString, tempC);
}

/**
 * @brief Læser temperatur og tidspunkt og logger til serial og CSV.
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
  
    // Send temperaturdata til WebSocket-klienter
    String message = "{\"temperature\": " + String(tempC, 2) + "}";
    ws.textAll(message);  // Sender til alle forbundne klienter
  }

