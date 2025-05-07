#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFiManager.h>        ///< AutoConnect WiFi Manager
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

#define ONE_WIRE_BUS 4
#define RESET_BUTTON_PIN 14

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const char *filename = "/output.csv";

unsigned long previousMillis = 0;
unsigned long interval = 30000;

unsigned long buttonPressStart = 0;
bool resetInitiated = false;

char timeString[64];

/**
 * @brief Sætter tidszonen.
 * @param timezone Tidszone-streng (POSIX-format)
 */
void setTimezone(String timezone) {
  setenv("TZ", timezone.c_str(), 1);
  tzset();
}

/**
 * @brief Initialiserer NTP-tid og sætter tidszone.
 * @param timezone Tidszone-streng
 */
void initTime(String timezone) {
  configTime(0, 0, "pool.ntp.org");
  setTimezone(timezone);
}

/**
 * @brief Initialiserer WiFi med WiFiManager. Starter konfigurationsportal, hvis ikke tidligere tilsluttet.
 */
void initWiFi() {
  WiFiManager wm;
  bool res = wm.autoConnect("ESP32-Logger", "admin123");
  if (!res) {
    Serial.println("WiFi forbindelse mislykkedes - genstarter");
    delay(3000);

    ESP.restart();
  }
  Serial.println("WiFi tilsluttet!");
  Serial.print("IP adresse: ");
  Serial.println(WiFi.localIP());
}

/**
 * @brief Tilføjer ny temperaturmåling til CSV-fil.
 * @param filename Sti til filen
 * @param time Tidsstempel som tekst
 * @param temperature Målt temperatur
 */
void appendToCSV(const char* filename, const char* time, float temperature) {
  bool fileExists = SPIFFS.exists(filename);

  File file = SPIFFS.open(filename, FILE_APPEND);
  if (!file) {
    Serial.println("Filen eksisterer ikke. Opretter ny fil...");
    file = SPIFFS.open(filename, FILE_WRITE);
    if (!file) {
      Serial.println("Kunne ikke oprette filen!");
      return;
    }
  }

  if (!fileExists) {
    file.println("Time,Temperature");
  }

  file.print(time);
  file.print(",");
  file.println(temperature);
  file.close();
}

/**
 * @brief Tjekker om reset-knappen holdes nede i 10 sekunder (LOW) ved først at måle tiden og genstarter derefter ESP32 og udfører software-reset. 
 * Når knappen ikke længere holdes nede sættes resetInitiated = false
 */
void checkResetButton() {
  if (digitalRead(RESET_BUTTON_PIN) == LOW) {
    if (!resetInitiated) {
      buttonPressStart = millis();
      resetInitiated = true;
    } else if (millis() - buttonPressStart >= 10000) {
      Serial.println("Reset udført efter 10 sek. knaptryk!");
      WiFiManager wm;
      wm.resetSettings();
      ESP.restart(); // Udfør software-reset
    }
  } else {
    resetInitiated = false;
  }
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
server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });
  
  // Øvrige HTML-sider
  server.on("/wificonf.html", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/wificonf.html", "text/html");
  });
  
  server.on("/diagram.html", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/diagram.html", "text/html");
  });
  
  server.on("/service.html", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/service.html", "text/html");
  });
  
  // CSS
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });
  //charts.js
  server.on("/chart.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/chart.js", "application/javascript");
  });
  server.on("/papaparse.min.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/papaparse.min.js", "application/javascript");
  });
  

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
  
