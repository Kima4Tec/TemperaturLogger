// #include <OneWire.h>
// #include <DallasTemperature.h>
// #include <WiFi.h>
// #include "secrets.h"
// #include <SPIFFS.h>
// #include <WebServer.h>

// #define ONE_WIRE_BUS 4      // DS18B20 DATA pin
// #define RESET_BUTTON_PIN 14 // Tilslut din knap til GPIO14 (med GND)

// OneWire oneWire(ONE_WIRE_BUS);
// DallasTemperature sensors(&oneWire);

// // WiFi og tid
// unsigned long previousMillis = 0;
// unsigned long interval = 30000;

// // Reset-knap timer
// unsigned long buttonPressStart = 0;
// bool resetInitiated = false;

// WebServer server(80);

// void setTimezone(String timezone) {
//   setenv("TZ", timezone.c_str(), 1);
//   tzset();
// }

// void initTime(String timezone) {
//   configTime(0, 0, "pool.ntp.org");
//   struct tm timeinfo;
//   if (!getLocalTime(&timeinfo)) {
//     Serial.println("Failed to get time");
//     return;
//   }
//   setTimezone(timezone);
// }

// void initWiFi() {
//   WiFi.mode(WIFI_STA);
//   WiFi.begin(ssid, password);
//   Serial.print("Connecting to WiFi");
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(1000);
//     Serial.print(".");
//   }
//   Serial.println("\nConnected to WiFi!");
//   Serial.print("IP address: ");
//   Serial.println(WiFi.localIP());  // Udskriv IP-adressen til Serial Monitor
// }


// void printLog() {
//   struct tm timeinfo;
//   if (!getLocalTime(&timeinfo)) {
//     Serial.println("Failed to get time");
//     return;
//   }

//   sensors.requestTemperatures();
//   float tempC = sensors.getTempCByIndex(0);

//   char timeString[64];
//   strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);

//   // Udskriv kun til Serial Monitor
//   Serial.println(String(timeString) + " - Temperatur: " + String(tempC, 2) + " ºC");
// }


// void checkResetButton() {
//   if (digitalRead(RESET_BUTTON_PIN) == LOW) {
//     if (!resetInitiated) {
//       buttonPressStart = millis();
//       resetInitiated = true;
//     } else if (millis() - buttonPressStart >= 1000) {
//       Serial.println("Reset udført efter 10 sek. knaptryk!");
//       ESP.restart(); // Udfør software-reset
//     }
//   } else {
//     resetInitiated = false;
//   }
// }

// void setup() {
//   Serial.begin(115200);
//   pinMode(RESET_BUTTON_PIN, INPUT_PULLUP); // Brug evt. ekstern modstand hvis ustabil

//   sensors.begin();
//   initWiFi();
//   initTime("CET-1CEST,M3.5.0,M10.5.0/3");

//   Serial.println("Startet temperatur-logger...");
//   SPIFFS.begin();
//   server.on("/", HTTP_GET, []() {
//     File file = SPIFFS.open("/index.html", "r");
//     server.streamFile(file, "text/html");
//     file.close();
//   });

//   server.on("/temperature", HTTP_GET, []() {
//     struct tm timeinfo;
//     if (!getLocalTime(&timeinfo)) {
//         server.send(500, "text/plain", "Failed to get time");
//         return;
//     }

//     sensors.requestTemperatures();
//     float tempC = sensors.getTempCByIndex(0);

//     char timeString[64];
//     strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);

//     String response = "{\"date\": \"" + String(timeString) + "\", \"temperature\": " + String(tempC) + "}";
//     server.send(200, "application/json", response);
// });

//   server.begin();
// }

// void loop() {
//   printLog();
//   checkResetButton();

//   // WiFi re-connect hvis nødvendigt
//   if ((WiFi.status() != WL_CONNECTED) && (millis() - previousMillis >= interval)) {
//     WiFi.disconnect();
//     WiFi.reconnect();
//     previousMillis = millis();
//   }

//   delay(5000); // hver 5. sekund

//   server.handleClient();
// }
