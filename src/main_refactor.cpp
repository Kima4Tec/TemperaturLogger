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

void setupRoutes()
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html", "text/html"); });

  // Øvrige HTML-sider
  server.on("/wificonf.html", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/wificonf.html", "text/html"); });

  server.on("/diagram.html", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/diagram.html", "text/html"); });

  server.on("/service.html", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/service.html", "text/html"); });

  // CSS
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/style.css", "text/css"); });
  // charts.js
  server.on("/chart.js", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/chart.js", "application/javascript"); });
  server.on("/papaparse.min.js", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/papaparse.min.js", "application/javascript"); });
            server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request) {
              struct tm now;
              if (!getLocalTime(&now)) {
                request->send(500, "text/plain", "Failed to get time");
                return;
              }    
              sensors.requestTemperatures();
              float currentTemp = sensors.getTempCByIndex(0);
              char nowString[64];
              strftime(nowString, sizeof(nowString), "%Y-%m-%d %H:%M:%S", &now);
          
              String response = "{\"date\": \"" + String(nowString) + "\", \"temperature\": " + String(currentTemp) + "}";
              request->send(200, "application/json", response);
            });
            server.on(filename, HTTP_GET, [](AsyncWebServerRequest *request) {
              if (!SPIFFS.exists(filename)) {
                request->send(500, "text/plain", "CSV-fil ikke fundet");
                return;
              }
              request->send(SPIFFS, filename, "text/csv");
            });
          
            server.on("/delete", HTTP_GET, [](AsyncWebServerRequest *request) {
              if (SPIFFS.exists(filename)) {
                SPIFFS.remove(filename);
                request->send(200, "text/plain", "CSV-fil slettet.");
              } else {
                request->send(404, "text/plain", "CSV-fil findes ikke.");
              }
            });
          
            server.on("/resetwifi", HTTP_GET, [](AsyncWebServerRequest *request) {
              WiFiManager wm;
              wm.resetSettings();
              request->send(200, "text/plain", "WiFi indstillinger er nulstillet.");
              ESP.restart(); // Genstart ESP32
            });
          
            server.on("/wifi", HTTP_POST, [](AsyncWebServerRequest *request) {
              if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
                String ssid = request->getParam("ssid", true)->value();
                String password = request->getParam("password", true)->value();
            
                Serial.println("Modtaget nye WiFi oplysninger:");
                Serial.println("SSID: " + ssid);
            
                WiFi.disconnect();
                WiFi.begin(ssid.c_str(), password.c_str());
            
                // Vent på forbindelse i op til 10 sekunder
                unsigned long startAttemptTime = millis();
                while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
                  delay(500);
                  Serial.print(".");
                }
            
                if (WiFi.status() == WL_CONNECTED) {
                  String ip = WiFi.localIP().toString();
                  Serial.println("\nWiFi tilsluttet! IP: " + ip);
                  request->send(200, "text/plain", "Tilsluttet! IP: " + ip);
                } else {
                  Serial.println("\nKunne ikke oprette forbindelse til WiFi.");
                  request->send(500, "text/plain", "Kunne ikke oprette forbindelse til WiFi");
                }
            
              } else {
                request->send(400, "text/plain", "SSID eller adgangskode mangler");
              }
            });
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
  
