#include "time_handler.h"
/**
 * @brief Sætter tidszonen.
 * @param timezone Tidszone-streng (POSIX-format, som bruges i NTP-konfiguration (Network Time Protocol) til at angive tidszoner.)
 */
void setTimezone(String timezone) {
    setenv("TZ", timezone.c_str(), 1);
    tzset();
  }
  /**
   * @brief Initialiserer NTP-tid og sætter tidszone.
   * @param timezone Tidszone-streng
   */
  void initTime(String timezone) {
    configTime(0, 0, "pool.ntp.org");
    setTimezone(timezone);
  }