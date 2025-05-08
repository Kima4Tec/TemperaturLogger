#include <WiFi.h>             // WiFi-funktioner for ESP32
#include <WiFiManager.h>
#include "reset_handler.h"


/**
 * @brief Tjekker om reset-knappen holdes nede i 10 sekunder (LOW) ved først at måle tiden og genstarter derefter ESP32 og udfører software-reset. 
 * Når knappen ikke længere holdes nede sættes resetInitiated = false. isReset bliver sat til true, når knappen har været holdt nede i 10 sekunder.
 */
void checkResetButton() {
  if (digitalRead(RESET_BUTTON_PIN) == LOW) {
    if (!resetInitiated) {
      buttonPressStart = millis();
      resetInitiated = true;
    } else if (millis() - buttonPressStart >= 10000 && !isReset) {
      Serial.println("Reset udført efter 10 sek. knaptryk!");
      isReset = true;
      WiFiManager wm;
      wm.resetSettings();
      ESP.restart(); // Udfør software-reset
    }
  } else {
    resetInitiated = false;
  }
}
