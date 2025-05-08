#include <SPIFFS.h>
#include "csv_handler.h"



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
      file.println(CSV_HEADER);
    }
  
    file.print(time);
    file.print(",");
    file.println(temperature);
    file.close();
  }