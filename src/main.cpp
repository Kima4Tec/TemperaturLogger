/**
 * @file main.cpp
 * @brief Hovedfil til ESP32 temperaturlogger med WebSocket, SPIFFS og WiFiManager.
 */

 #include <WiFi.h>              ///< WiFi-funktioner for ESP32
 #include <OneWire.h>           ///< Bibliotek til at kommunikere med DS18B20
 #include <DallasTemperature.h> ///< Bibliotek til temperaturdata fra DS18B20-sensor
 #include <WiFiManager.h>       ///< WiFi-konfiguration og portal
 #include <SPIFFS.h>            ///< SPI Flash File System til filhåndtering
 #include <ESPAsyncWebServer.h> ///< Asynkron webserver og WebSocket-support
 #include <AsyncTCP.h>          ///< TCP-support til async-server
 
 #include "Arduino.h"
 #include "config.h"
 #include "csv/csv_handler.h"
 #include "wifi/wifi_handler.h"
 #include "time/time_handler.h"
 #include "reset/reset_handler.h"
 #include "route/route_handler.h"
 #include "sensor/sensor_handler.h"
 #include "websocket/websocket_handler.h"
 
 /// OneWire-bus til DS18B20
 OneWire oneWire(ONE_WIRE_BUS);
 
 /// Temperatur-sensor
 DallasTemperature sensors(&oneWire);
 
 /// Asynkron webserver på port 80
 AsyncWebServer server(80);
 
 /// WebSocket objekt på endpoint "/ws"
 AsyncWebSocket ws("/ws");
 
 /// Filnavn til CSV-log
 const char *filename = DATA_FILENAME;
 
 /// Tid siden sidste WiFi-reconnect
 unsigned long previousMillis = 0;
 
 /// Interval mellem WiFi-reconnects (ms)
 unsigned long interval = 30000;
 
 /// Starttidspunkt for reset-knap
 unsigned long buttonPressStart = 0;
 
 /// Bool for igangværende reset
 bool resetInitiated = false;
 
 /// Formatér tid som tekst
 char timeString[64];
 
 /// Bool for reset-status
 bool isReset = false;
 
 /**
  * @brief Initialiserer systemet, WiFi, SPIFFS, sensorer og WebSocket.
  */
 void setup()
 {
   Serial.begin(115200);
   pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
 
   sensors.begin();
   initWiFi();
   initTime("CET-1CEST,M3.5.0,M10.5.0/3");
 
   if (!SPIFFS.begin(true))
   {
     Serial.println("SPIFFS initialisering fejlede");
     return;
   }
 
   getFormattedTime(timeString);              ///< Hent og formatér tid
   logTemperatureToCSV(sensors, timeString);  ///< Log temperatur til CSV
 
   setupRoutes(); ///< Initialiser webserver routes
 
   ws.onEvent(onWebSocketEvent);  ///< WebSocket event-handler
   server.addHandler(&ws);        ///< Tilføj WebSocket til server
 
   server.begin(); ///< Start webserver
 }
 
 /**
  * @brief Loop: logger temperatur samt håndterer WiFi og reset-knap.
  */
 void loop()
 {
   static unsigned long lastLogTime = 0;
   unsigned long currentMillis = millis();
 
   checkResetButton(); /// Checker om reset-knappen er blevet trykket ned 
 
   if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >= interval))
   {
     WiFi.disconnect();
     WiFi.reconnect();
     previousMillis = currentMillis;
   }
 
   if (currentMillis - lastLogTime >= 300000) // Hvert 5. minut
   {
     printLog(); ///< Logger temperatur og sender til klienter
     lastLogTime = currentMillis;
   }
 }
 