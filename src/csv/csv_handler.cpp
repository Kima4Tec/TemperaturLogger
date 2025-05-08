#include <SPIFFS.h>
#include "csv_handler.h"

/**
 * @brief Tilføjer en ny temperaturmåling til en CSV-fil på SPIFFS.
 * 
 * Funktionen tjekker, om filen allerede eksisterer. Hvis ikke, tilføjes en header.
 * Målingen tilføjes derefter i formatet: tid,temperatur
 * 
 * @param filename Sti til CSV-filen (f.eks. "/log.csv").
 * @param time Tidsstempel som tekst (f.eks. "2025-05-08 10:00:00").
 * @param temperature Målt temperatur i grader Celsius.
 */
void appendToCSV(const char* filename, const char* time, float temperature) {
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS kunne ikke initialiseres!");
        return;
    }

    bool fileExists = SPIFFS.exists(filename);
    
    File file = SPIFFS.open(filename, FILE_APPEND);
    if (!file || file.isDirectory()) {
        Serial.println("Kunne ikke åbne filen i append-tilstand. Forsøger at oprette en ny...");
        
        file = SPIFFS.open(filename, FILE_WRITE);
        if (!file || file.isDirectory()) {
            Serial.println("Fejl: Kunne hverken åbne eller oprette filen!");
            return;
        }
    }

    // Hvis filen ikke eksisterede, tilføj header
    if (!fileExists) {
        if (file.println(CSV_HEADER) == 0) {
            Serial.println("Fejl ved skrivning af header til filen!");
            file.close();
            return;
        }
    }

    // Skriv data til filen
    if (file.print(time) == 0 || file.print(",") == 0 || file.println(temperature) == 0) {
        Serial.println("Fejl ved skrivning af temperaturdata til filen!");
        file.close();
        return;
    }

    file.close();
    Serial.println("Temperaturdata gemt i CSV.");
}
