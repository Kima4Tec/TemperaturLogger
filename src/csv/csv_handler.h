#ifndef CSV_HANDLER_H
#define CSV_HANDLER_H

#include "config.h"

/**
 * @brief Tilføjer en ny linje med tidsstempel og temperatur til en CSV-fil.
 * @param filename Navnet på CSV-filen der skal skrives til.
 * @param time Tidsstempel som en streng (typisk i formatet "YYYY-MM-DD HH:MM:SS").
 * @param temperature Temperaturen i grader Celsius.
 */
void appendToCSV(const char* filename, const char* time, float temperature);

#endif
