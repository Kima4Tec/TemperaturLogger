#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include "secrets.h"  // Indeholder: const char* ssid og password
#include <SPIFFS.h>
#include <WebServer.h>

#define ONE_WIRE_BUS 4
#define RESET_BUTTON_PIN 14

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
WebServer server(80);

const char *filename = "/output.csv";

unsigned long previousMillis = 0;
unsigned long interval = 30000;

unsigned long buttonPressStart = 0;
bool resetInitiated = false;

char timeString[64];

void setTimezone(String timezone) {
  setenv("TZ", timezone.c_str(), 1);
  tzset();
}

void initTime(String timezone) {
  configTime(0, 0, "pool.ntp.org");
  setTimezone(timezone);
}

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

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

void checkResetButton() {
  if (digitalRead(RESET_BUTTON_PIN) == LOW) {
    if (!resetInitiated) {
      buttonPressStart = millis();
      resetInitiated = true;
    } else if (millis() - buttonPressStart >= 10000) {
      Serial.println("Reset udført efter 10 sek. knaptryk!");
      ESP.restart();
    }
  } else {
    resetInitiated = false;
  }
}

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
}

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

  // Formater tid og log første måling
  strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);
  appendToCSV(filename, timeString, tempC);

  // Webserver – startside
  server.on("/", HTTP_GET, []() {
    File file = SPIFFS.open("/index.html", "r");
    if (!file) {
      server.send(500, "text/plain", "index.html ikke fundet");
      return;
    }
    server.streamFile(file, "text/html");
    file.close();
  });

  // Webserver – JSON temperatur
  server.on("/temperature", HTTP_GET, []() {
    struct tm now;
    if (!getLocalTime(&now)) {
      server.send(500, "text/plain", "Failed to get time");
      return;
    }

    sensors.requestTemperatures();
    float currentTemp = sensors.getTempCByIndex(0);
    char nowString[64];
    strftime(nowString, sizeof(nowString), "%Y-%m-%d %H:%M:%S", &now);

    String response = "{\"date\": \"" + String(nowString) + "\", \"temperature\": " + String(currentTemp) + "}";
    server.send(200, "application/json", response);
  });

  // Webserver – vis CSV-data
  server.on(filename, HTTP_GET, []() {
    File file = SPIFFS.open(filename, "r");
    if (!file) {
      server.send(500, "text/plain", "CSV-fil ikke fundet");
      return;
    }

    server.streamFile(file, "text/csv");
    file.close();
  });

  server.on("/delete", HTTP_GET, []() {
    if (SPIFFS.exists("/output.csv")) {
      SPIFFS.remove("/output.csv");
      server.send(200, "text/plain", "CSV-fil slettet.");
    } else {
      server.send(404, "text/plain", "CSV-fil findes ikke.");
    }
  });

  server.begin();
}

void loop() {
  printLog();
  checkResetButton();

  if ((WiFi.status() != WL_CONNECTED) && (millis() - previousMillis >= interval)) {
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = millis();
  }

  server.handleClient();
  delay(5000);  // Vent 5 sekunder mellem målinger
}
