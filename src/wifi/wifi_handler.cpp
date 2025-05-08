#include <WiFi.h>    
#include <WiFiManager.h> 
#include "wifi_handler.h"

/**
 * @brief Initialiserer WiFi med WiFiManager. Starter konfigurationsportal, hvis ikke tidligere tilsluttet.
 */
void initWiFi() {
    WiFiManager wm;
    bool res = wm.autoConnect(WIFI_AP_NAME, WIFI_AP_PASSWORD);
    if (!res) {
      Serial.println("WiFi forbindelse mislykkedes - genstarter");
      delay(3000);
      ESP.restart();
    }
  
    Serial.println("WiFi tilsluttet!");
    Serial.print("IP adresse: ");
    Serial.println(WiFi.localIP());
    }