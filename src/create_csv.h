// // #include "SPIFFS.h"

// void appendToCSV(const char* filename, const char* time, float temperature) {
//     Serial.println("nu kommer b");

//     // Prøv at åbne filen i append-mode
//     File file = SPIFFS.open(filename, FILE_APPEND);
//     Serial.println("nu kommer c");
//     // Hvis filen ikke findes, prøv at oprette den
//     if (!file) {
//         Serial.println("Filen eksisterer ikke. Opretter ny fil...");
//         file = SPIFFS.open(filename, FILE_WRITE);
//         if (!file) {
//             Serial.println("Kunne ikke oprette filen!");
//             return;
//         }

//         // Skriv header hvis filen netop er oprettet
//         file.println("Time,Temperature");
//     }

//     // Tilføj data
//     file.print(time);
//     file.print(",");
//     file.println(temperature);  // Slutter med ny linje
//     file = SPIFFS.open("output.csv", FILE_READ);

//     Serial.println(file);
//     file.close();
// }
