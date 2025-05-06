/**
 * @brief Læser temperaturdata fra en DS18B20-sensor og udskriver det til serial monitor.
 *
 * Dette program bruger OneWire og DallasTemperature bibliotekerne
 * til at læse temperatur i Celsius og Fahrenheit.
 *
 * @author Rui Santos
 * @see https://randomnerdtutorials.com/esp32-ds18b20-temperature-arduino-ide/
 */

 #include <OneWire.h>
 #include <DallasTemperature.h>
 
 /**
  * @brief GPIO hvor DS18B20-temperatursensoren er tilsluttet.
  */
 const int oneWireBus = 4;     
 
 /**
  * @brief Opretter en OneWire instans for at kommunikere med OneWire-enheder.
  */
 OneWire oneWire(oneWireBus);
 
 /**
  * @brief Opretter et DallasTemperature objekt ved at bruge OneWire-forbindelsen.
  */
 DallasTemperature sensors(&oneWire);
 
 /**
  * @brief Initialiserer serial kommunikation og temperatur-sensor.
  */
 void setup() {
   Serial.begin(115200); ///< Starter serial kommunikation.
   sensors.begin();      ///< Initialiserer DS18B20 sensoren.
 }
 
 /**
  * @brief Læser temperaturen og skriver den til Serial Monitor.
  *
  * Temperaturen læses både i Celsius og Fahrenheit hvert 5. sekund.
  */
 void loop() {
   sensors.requestTemperatures(); ///< Beder om ny temperaturmåling.
   
   float temperatureC = sensors.getTempCByIndex(0); ///< Temperatur i Celsius.
   float temperatureF = sensors.getTempFByIndex(0); ///< Temperatur i Fahrenheit.
 
   Serial.print(temperatureC);
   Serial.println("ºC");
   Serial.print(temperatureF);
   Serial.println("ºF");
 
   delay(5000); ///< Vent 5 sekunder før næste måling.
 }
 