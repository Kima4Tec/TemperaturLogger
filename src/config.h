#ifndef CONFIG_H
#define CONFIG_H

// Pin-konfigurationer
#define ONE_WIRE_BUS 4            // Pin for DS18B20-sensoren
#define RESET_BUTTON_PIN 14       // Pin for reset-knap

//Wifi
#define WIFI_AP_NAME "ESP32-Logger"
#define WIFI_AP_PASSWORD "admin123"

// SPIFFS
#define DATA_FILENAME "/output.csv"

// CSV-header til fil
constexpr const char* CSV_HEADER = "Time,Temperature";

// Deklaration af globale variabler
extern unsigned long buttonPressStart;
extern bool resetInitiated;

extern bool isReset;

#endif // CONFIG_H
