#include <WiFi.h>              // WiFi-funktioner for ESP32
#include <OneWire.h>           // Bibliotek til at kommunikere med DS18B20
#include <DallasTemperature.h> // Bibliotek, der bruges specifikt til at læse temperaturdata fra DS18B20-sensor
#include <WiFiManager.h>       // Bibliotek der håndterer WiFi-forbindelse og konfigurationsportal til netværksvalg
#include <SPIFFS.h>            // SPI Flash File System – bruges til at læse og skrive filer i flash-hukommelsen (fx CSV)
#include <ESPAsyncWebServer.h> // Asynkront webserver/websocket-bibliotek til ESP32
#include <AsyncTCP.h>          // TCP-understøttelse til asynkrone operationer – kræves af ESPAsyncWebServer
#include "Arduino.h"
#include "config.h"
#include "csv/csv_handler.h"
#include "wifi/wifi_handler.h"
#include "time/time_handler.h"
#include "reset/reset_handler.h"
#include "route/route_handler.h"
#include "sensor/sensor_handler.h"
#include "websocket/websocket_handler.h"

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const char *filename = DATA_FILENAME;

unsigned long previousMillis = 0;
unsigned long interval = 30000;

unsigned long buttonPressStart = 0;
bool resetInitiated = false;

char timeString[64];

bool isReset = false;

/**
 * @brief Setup-funktion.
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

  // Hent og formatér tid
  getFormattedTime(timeString);

  // Log temperatur og tid til CSV
  logTemperatureToCSV(sensors, timeString);

  // Webserver routes (Async-style)
  // Root-side (forside)
  setupRoutes();

  // Tilføj WebSocket handler
  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);

  server.begin();
}

/**
 * @brief Loop-funktion. Logger temperatur, og overvåger WiFi og reset-knap.
 */
void loop()
{
  static unsigned long lastLogTime = 0;
  unsigned long currentMillis = millis();

  checkResetButton(); // Kaldes hele tiden

  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >= interval))
  {
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = currentMillis;
  }

  if (currentMillis - lastLogTime >= 300000)
  { // 5 minutter
    printLog();
    lastLogTime = currentMillis;
  }
}
