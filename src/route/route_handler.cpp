#include <WiFiManager.h> 
#include <ESPAsyncWebServer.h>    // Asynkront webserver/websocket-bibliotek til ESP32
   
#include <SPIFFS.h>   
#include "route_handler.h"

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
            server.on(DATA_FILENAME, HTTP_GET, [](AsyncWebServerRequest *request) {
              if (!SPIFFS.exists(DATA_FILENAME)) {
                request->send(500, "text/plain", "CSV-fil ikke fundet");
                return;
              }
              request->send(SPIFFS, DATA_FILENAME, "text/csv");
            });
          
            server.on("/delete", HTTP_GET, [](AsyncWebServerRequest *request) {
              if (SPIFFS.exists(DATA_FILENAME)) {
                SPIFFS.remove(DATA_FILENAME);
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