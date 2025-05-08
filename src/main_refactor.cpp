#include <WiFi.h>                 // WiFi-funktioner for ESP32
#include <OneWire.h>              // Bibliotek til at kommunikere med DS18B20
#include <DallasTemperature.h>    // Bibliotek, der bruges specifikt til at læse temperaturdata fra DS18B20-sensor
#include <WiFiManager.h>          // Bibliotek der håndterer WiFi-forbindelse og konfigurationsportal til netværksvalg
#include <SPIFFS.h>               // SPI Flash File System – bruges til at læse og skrive filer i flash-hukommelsen (fx CSV)
#include <ESPAsyncWebServer.h>    // Asynkront webserver/websocket-bibliotek til ESP32
#include <AsyncTCP.h>             // TCP-understøttelse til asynkrone operationer – kræves af ESPAsyncWebServer
#include "config.h"
#include "csv/csv_handler.h"
#include "wifi/wifi_handler.h"
#include "time/time_handler.h"
#include "reset/reset_handler.h"
#include "Arduino.h"

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

/**
 * @brief WebSocket event handler
 */
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
                      void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.println("WebSocket klient forbundet");
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.println("WebSocket klient afbrudt");
  } else if (type == WS_EVT_DATA) {
    // Håndter evt. beskeder fra klienten
  }
}

/**
 * @brief Setup-funktion.
 */
void setup() {
  Serial.begin(115200);
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);

  sensors.begin();

  initWiFi();
  initTime("CET-1CEST,M3.5.0,M10.5.0/3");

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS initialisering fejlede");
    return;
  }
  
  // Vent på gyldig temperatur
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  while (tempC == DEVICE_DISCONNECTED_C || tempC == -127.0) {
    Serial.println("Venter på gyldig temperatur...");
    delay(500);
    sensors.requestTemperatures();
    tempC = sensors.getTempCByIndex(0);
  }

  // Vent på gyldig tid
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.println("Venter på gyldig tid...");
    delay(500);
  }

  strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);
  appendToCSV(filename, timeString, tempC);

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
void loop() {
    static unsigned long lastLogTime = 0;
    unsigned long currentMillis = millis();
  
    checkResetButton();  // Kaldes hele tiden
  
    if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >= interval)) {
      WiFi.disconnect();
      WiFi.reconnect();
      previousMillis = currentMillis;
    }
  
    if (currentMillis - lastLogTime >= 300000) { // 5 minutter
      printLog();
      lastLogTime = currentMillis;
    }
  }
  
