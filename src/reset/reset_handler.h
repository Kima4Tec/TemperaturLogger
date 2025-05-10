#ifndef RESET_HANDLER_H
#define RESET_HANDLER_H

#include <Arduino.h>
#include "config.h"

/**
 * @brief Tjekker om reset-knappen holdes nede i mindst 10 sekunder og udfører så reset.
 */
 void checkResetButton();

#endif
