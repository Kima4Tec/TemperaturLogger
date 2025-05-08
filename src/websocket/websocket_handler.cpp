#include "websocket_handler.h"
#include <Arduino.h>  // For Serial

/**
 * @brief WebSocket event handler.
 *
 * Denne funktion håndterer hændelser fra WebSocket-serveren såsom forbindelse, afbrydelse og data.
 *
 * @param server Pointer til den asynkrone WebSocket-server.
 * @param client Pointer til den klient, der genererede hændelsen.
 * @param type Typen af WebSocket-hændelse (forbindelse, afbrydelse, data, m.m.).
 * @param arg Ekstra argumenter (bruges normalt ikke).
 * @param data Pointer til data (kun relevant ved WS_EVT_DATA).
 * @param len Længden af dataen.
 */
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
                      void *arg, uint8_t *data, size_t len)
{
  if (type == WS_EVT_CONNECT)
  {
    if (client)
    {
      Serial.printf("WebSocket klient #%u forbundet fra %s\n", client->id(), client->remoteIP().toString().c_str());
    }
    else
    {
      Serial.println("Fejl: WebSocket klient er NULL ved forbindelse.");
    }
  }
  else if (type == WS_EVT_DISCONNECT)
  {
    if (client)
    {
      Serial.printf("WebSocket klient #%u afbrudt\n", client->id());
    }
    else
    {
      Serial.println("Fejl: WebSocket klient er NULL ved afbrydelse.");
    }
  }
  else if (type == WS_EVT_DATA)
  {
    if (client && data && len > 0)
    {
      // Eksempel: udskriv modtagne data som tekst (kan udbygges)
      Serial.printf("WebSocket data fra klient #%u: %.*s\n", client->id(), len, data);
      // Her kunne du tilføje parsing af dataen og evt. svar
    }
    else
    {
      Serial.println("Fejl: Ugyldig data modtaget via WebSocket.");
    }
  }
  else
  {
    Serial.println("Ukendt WebSocket-hændelsestype.");
  }
}
