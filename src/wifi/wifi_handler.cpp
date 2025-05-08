#include <WiFi.h>    
#include <WiFiManager.h> 
#include "wifi_handler.h"

/**
 * @brief Initialiserer WiFi med WiFiManager.
 *
 * Funktionen forsøger automatisk at oprette forbindelse til gemte WiFi-netværk.
 * Hvis ingen forbindelse er mulig, starter den et adgangspunkt (AP) for manuel konfiguration.
 * Ved fejl i forbindelse genstarter den enheden.
 */
void initWiFi() {
    WiFiManager wm;

    // Forsøg automatisk forbindelse eller start AP-portal
    bool res = wm.autoConnect(WIFI_AP_NAME, WIFI_AP_PASSWORD);
    if (!res) {
        Serial.println("WiFi forbindelse mislykkedes - genstarter...");
        delay(3000); // Kort pause for at give brugeren tid til at læse fejlmeddelelse
        ESP.restart(); // Soft reset
        return;
    }

    // Succes
    Serial.println("WiFi tilsluttet!");
    Serial.print("IP adresse: ");
    Serial.println(WiFi.localIP());
}
