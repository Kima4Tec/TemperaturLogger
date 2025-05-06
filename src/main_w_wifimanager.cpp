// /**
//  * @file main.cpp
//  * @brief Temperatur-logger med WiFi, SPIFFS og WebServer.
//  *
//  * Logger temperaturer fra en DS18B20-sensor og gemmer dem i en CSV-fil på SPIFFS.
//  * Webserver giver adgang til CSV-data og aktuelle temperaturmålinger.
//  */

//  #include <OneWire.h> ///en seriel kommunikationsprotokol
//  #include <DallasTemperature.h> ///bygget ovenpå OneWire og giver en nem måde at håndtere DS18B20-temperatursensorer
//  #include <WiFi.h> ///bruges til at forbinde ESP32 til et WiFi-netværk
//  #include "secrets.h"  ///< Indeholder: const char* ssid og password
//  #include <SPIFFS.h> ///et lille filsystem, der ligger i flash-hukommelsen på mikrocontrolleren
//  #include <WebServer.h> ///Starter en simpel HTTP-server på ESP32.
 
//  #define ONE_WIRE_BUS 4             ///< GPIO-pin til DS18B20 data
//  #define RESET_BUTTON_PIN 14        ///< GPIO-pin til reset-knap
 
//  OneWire oneWire(ONE_WIRE_BUS);
//  DallasTemperature sensors(&oneWire); ///&oneWire er adresseoperatoren
//  WebServer server(80);
 
//  const char *filename = "/output.csv";  ///< Filnavn for temperatur-log
 
//  unsigned long previousMillis = 0;
//  unsigned long interval = 30000;  ///< Tidsinterval for WiFi-genforbindelse
 
//  unsigned long buttonPressStart = 0;
//  bool resetInitiated = false;
 
//  char timeString[64];
 
//  /**
//   * @brief Sætter tidszonen.
//   * @param timezone Tidszone-streng (POSIX-format)
//   */
//  void setTimezone(String timezone) {
//    setenv("TZ", timezone.c_str(), 1);
//    tzset();
//  }
 
//  /**
//   * @brief Initialiserer NTP-tid og sætter tidszone.
//   * @param timezone Tidszone-streng
//   */
//  void initTime(String timezone) {
//    configTime(0, 0, "pool.ntp.org");
//    setTimezone(timezone);
//  }
 
//  /**
//   * @brief Initialiserer WiFi-forbindelse og skriver IP til Serial.
//   */
//  void initWiFi() {
//    WiFi.mode(WIFI_STA);
//    WiFi.begin(ssid, password);
//    Serial.print("Connecting to WiFi");
//    while (WiFi.status() != WL_CONNECTED) {
//      delay(1000);
//      Serial.print(".");
//    }
//    Serial.println("\nConnected to WiFi!");
//    Serial.print("IP address: ");
//    Serial.println(WiFi.localIP());
//  }
 
//  /**
//   * @brief Tilføjer ny temperaturmåling til CSV-fil.
//   * @param filename Sti til filen
//   * @param time Tidsstempel som tekst
//   * @param temperature Målt temperatur i grader Celsius
//   */
//  void appendToCSV(const char* filename, const char* time, float temperature) {
//    bool fileExists = SPIFFS.exists(filename);
 
//    File file = SPIFFS.open(filename, FILE_APPEND);
//    if (!file) {
//      Serial.println("Filen eksisterer ikke. Opretter ny fil...");
//      file = SPIFFS.open(filename, FILE_WRITE);
//      if (!file) {
//        Serial.println("Kunne ikke oprette filen!");
//        return;
//      }
//    }
//    if (!fileExists) {
//      file.println("Time,Temperature");
//    }
 
//    file.print(time);
//    file.print(",");
//    file.println(temperature);
//    file.close();
//  }
 
//  /**
//   * @brief Tjekker om reset-knappen holdes nede i 10 sekunder og genstarter derefter ESP32.
//   */
//  void checkResetButton() {
//    if (digitalRead(RESET_BUTTON_PIN) == LOW) {
//      if (!resetInitiated) {
//        buttonPressStart = millis();
//        resetInitiated = true;
//      } else if (millis() - buttonPressStart >= 10000) {
//        Serial.println("Reset udført efter 10 sek. knaptryk!");
//        ESP.restart();
//      }
//    } else {
//      resetInitiated = false;
//    }
//  }
 
//  /**
//   * @brief Læser temperatur og tidspunkt og skriver til Serial og CSV.
//   */
//  void printLog() {
//    struct tm timeinfo;
//    if (!getLocalTime(&timeinfo)) {
//      Serial.println("Failed to get time");
//      return;
//    }
 
//    sensors.requestTemperatures();
//    float tempC = sensors.getTempCByIndex(0);
 
//    char logTime[64];
//    strftime(logTime, sizeof(logTime), "%Y-%m-%d %H:%M:%S", &timeinfo);
 
//    Serial.println(String(logTime) + " - Temperatur: " + String(tempC, 2) + " ºC");
 
//    appendToCSV(filename, logTime, tempC);
//  }
 
//  /**
//   * @brief Setup-funktion. Initialiserer systemet, SPIFFS, WiFi, tid, temperatur og webserver.
//   */
//  void setup() {
//    Serial.begin(115200);
//    pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
 
//    sensors.begin();
//    initWiFi();
//    initTime("CET-1CEST,M3.5.0,M10.5.0/3");
 
//    if (!SPIFFS.begin(true)) {
//      Serial.println("SPIFFS initialisering fejlede");
//      return;
//    }
 
//    // Vent på gyldig temperatur
//    sensors.requestTemperatures();
//    float tempC = sensors.getTempCByIndex(0);
//    while (tempC == DEVICE_DISCONNECTED_C || tempC == -127.0) {
//      Serial.println("Venter på gyldig temperatur...");
//      delay(500);
//      sensors.requestTemperatures();
//      tempC = sensors.getTempCByIndex(0);
//    }
 
//    // Vent på gyldig tid
//    struct tm timeinfo;
//    while (!getLocalTime(&timeinfo)) {
//      Serial.println("Venter på gyldig tid...");
//      delay(500);
//    }
 
//    strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);
//    appendToCSV(filename, timeString, tempC);
 
//    // Webserver routes
//    server.on("/", HTTP_GET, []() {
//      File file = SPIFFS.open("/index.html", "r");
//      if (!file) {
//        server.send(500, "text/plain", "index.html ikke fundet");
//        return;
//      }
//      server.streamFile(file, "text/html");
//      file.close();
//    });
 
//    server.on("/temperature", HTTP_GET, []() {
//      struct tm now;
//      if (!getLocalTime(&now)) {
//        server.send(500, "text/plain", "Failed to get time");
//        return;
//      }
 
//      sensors.requestTemperatures();
//      float currentTemp = sensors.getTempCByIndex(0);
//      char nowString[64];
//      strftime(nowString, sizeof(nowString), "%Y-%m-%d %H:%M:%S", &now);
 
//      String response = "{\"date\": \"" + String(nowString) + "\", \"temperature\": " + String(currentTemp) + "}";
//      server.send(200, "application/json", response);
//    });
 
//    server.on(filename, HTTP_GET, []() {
//      File file = SPIFFS.open(filename, "r");
//      if (!file) {
//        server.send(500, "text/plain", "CSV-fil ikke fundet");
//        return;
//      }
 
//      server.streamFile(file, "text/csv");
//      file.close();
//    });
 
//    server.on("/delete", HTTP_GET, []() {
//      if (SPIFFS.exists("/output.csv")) {
//        SPIFFS.remove("/output.csv");
//        server.send(200, "text/plain", "CSV-fil slettet.");
//      } else {
//        server.send(404, "text/plain", "CSV-fil findes ikke.");
//      }
//    });
 
//    server.begin();
//  }
 
//  /**
//   * @brief Loop-funktion. Logger temperatur, håndterer webklient og overvåger WiFi og reset-knap.
//   */
//  void loop() {
//    printLog();
//    checkResetButton();
 
//    if ((WiFi.status() != WL_CONNECTED) && (millis() - previousMillis >= interval)) {
//      WiFi.disconnect();
//      WiFi.reconnect();
//      previousMillis = millis();
//    }
 
//    server.handleClient();
//    delay(5000);  // Vent 5 sekunder mellem målinger
//  }
 