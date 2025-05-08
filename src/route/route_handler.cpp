#include <WiFiManager.h>           ///< Bibliotek til nem WiFi-konfiguration
#include <ESPAsyncWebServer.h>    ///< Asynkron webserver/websocket-bibliotek til ESP32
#include <SPIFFS.h>               ///< SPIFFS-filsystem til lagring af webfiler
#include "route_handler.h"        ///< Headerfil hvor server er deklareret

/**
 * @brief Registrerer alle HTTP-ruter til webserveren.
 * 
 * Webserveren håndterer HTML, CSS/JS, temperaturdata, WiFi-konfiguration og CSV-filer.
 * Inkluderer fejlhåndtering ved manglende filer eller hardwarefejl.
 */
void setupRoutes()
{
  // Forside
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!SPIFFS.exists("/index.html")) {
      request->send(500, "text/plain", "index.html mangler");
      return;
    }
    request->send(SPIFFS, "/index.html", "text/html");
  });

  // HTML-sider
  server.on("/wificonf.html", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!SPIFFS.exists("/wificonf.html")) {
      request->send(500, "text/plain", "wificonf.html mangler");
      return;
    }
    request->send(SPIFFS, "/wificonf.html", "text/html");
  });

  server.on("/diagram.html", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!SPIFFS.exists("/diagram.html")) {
      request->send(500, "text/plain", "diagram.html mangler");
      return;
    }
    request->send(SPIFFS, "/diagram.html", "text/html");
  });

  server.on("/service.html", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!SPIFFS.exists("/service.html")) {
      request->send(500, "text/plain", "service.html mangler");
      return;
    }
    request->send(SPIFFS, "/service.html", "text/html");
  });

  // CSS
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!SPIFFS.exists("/style.css")) {
      request->send(500, "text/plain", "style.css mangler");
      return;
    }
    request->send(SPIFFS, "/style.css", "text/css");
  });

  // JavaScript-filer
  server.on("/chart.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!SPIFFS.exists("/chart.js")) {
      request->send(500, "text/plain", "chart.js mangler");
      return;
    }
    request->send(SPIFFS, "/chart.js", "application/javascript");
  });

  server.on("/papaparse.min.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!SPIFFS.exists("/papaparse.min.js")) {
      request->send(500, "text/plain", "papaparse.min.js mangler");
      return;
    }
    request->send(SPIFFS, "/papaparse.min.js", "application/javascript");
  });

  // Temperaturdata i JSON-format
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request) {
    struct tm now;
    if (!getLocalTime(&now)) {
      request->send(500, "text/plain", "Kunne ikke hente tid");
      return;
    }

    sensors.requestTemperatures();
    float currentTemp = sensors.getTempCByIndex(0);

    if (currentTemp == DEVICE_DISCONNECTED_C) {
      request->send(500, "text/plain", "Temperatursensor ikke tilgængelig");
      return;
    }

    char nowString[64];
    strftime(nowString, sizeof(nowString), "%Y-%m-%d %H:%M:%S", &now);

    String response = "{\"date\": \"" + String(nowString) + "\", \"temperature\": " + String(currentTemp) + "}";
    request->send(200, "application/json", response);
  });

  // Hent CSV-data
  server.on(DATA_FILENAME, HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!SPIFFS.exists(DATA_FILENAME)) {
      request->send(404, "text/plain", "CSV-fil ikke fundet");
      return;
    }
    request->send(SPIFFS, DATA_FILENAME, "text/csv");
  });

  // Slet CSV-fil
  server.on("/delete", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (SPIFFS.exists(DATA_FILENAME)) {
      if (SPIFFS.remove(DATA_FILENAME)) {
        request->send(200, "text/plain", "CSV-fil slettet.");
      } else {
        request->send(500, "text/plain", "Kunne ikke slette CSV-fil.");
      }
    } else {
      request->send(404, "text/plain", "CSV-fil findes ikke.");
    }
  });

  // Nulstil WiFi-indstillinger
  server.on("/resetwifi", HTTP_GET, [](AsyncWebServerRequest *request) {
    WiFiManager wm;
    wm.resetSettings();
    request->send(200, "text/plain", "WiFi-indstillinger er nulstillet.");
    delay(100);
    ESP.restart();
  });

  // Modtag ny WiFi-konfiguration via POST
  server.on("/wifi", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
      String ssid = request->getParam("ssid", true)->value();
      String password = request->getParam("password", true)->value();

      Serial.println("Modtaget nye WiFi-oplysninger:");
      Serial.println("SSID: " + ssid);

      WiFi.disconnect(true);
      WiFi.begin(ssid.c_str(), password.c_str());

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
        request->send(500, "text/plain", "Kunne ikke oprette forbindelse til WiFi.");
      }
    } else {
      request->send(400, "text/plain", "SSID eller adgangskode mangler.");
    }
  });
}
