#include <WiFi.h>          // WiFi-funktioner for ESP32
#include <WiFiManager.h>   // WiFiManager til at nulstille WiFi-indstillinger
#include "reset_handler.h"

/**
 * @brief Tjekker om reset-knappen holdes nede i mindst 10 sekunder og udfører software-reset.
 * 
 * Når knappen holdes nede (LOW-niveau på RESET_BUTTON_PIN) i mindst 10 sekunder,
 * nulstilles WiFi-indstillinger via WiFiManager og ESP32 genstartes.
 * 
 * Hvis knappen slippes før tiden er gået, annulleres reset-processen.
 * 
 * @note Bør kaldes regelmæssigt i loop().
 */
void checkResetButton() {
    int buttonState = digitalRead(RESET_BUTTON_PIN);

    // Knappen holdes nede
    if (buttonState == LOW) {
        if (!resetInitiated) {
            buttonPressStart = millis();
            resetInitiated = true;
            Serial.println("Resetknap trykket ned. Tæller ned...");
        } else if (millis() - buttonPressStart >= 10000 && !isReset) {
            Serial.println("Reset udført efter 10 sek. knaptryk!");
            isReset = true;

            // Nulstil WiFi-indstillinger og genstart
            WiFiManager wm;
            wm.resetSettings();
            delay(100); // Giv tid til at sende seriel besked
            ESP.restart();
        }
    } 
    // Knappen er sluppet
    else {
        if (resetInitiated) {
            Serial.println("Reset blev afbrudt før 10 sekunder.");
        }
        resetInitiated = false;
    }
}
