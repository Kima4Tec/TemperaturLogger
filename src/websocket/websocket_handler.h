#ifndef WEBSOCKET_HANDLER_H
#define WEBSOCKET_HANDLER_H

#include <ESPAsyncWebServer.h>

/**
 * @brief WebSocket event handler.
 *
 * Denne funktion håndterer WebSocket-hændelser for ESPAsyncWebServer, inkl. tilslutning,
 * afbrydelse og indkommende data.
 *
 * @param server Pointer til WebSocket-serveren.
 * @param client Pointer til den WebSocket-klient, der genererede hændelsen.
 * @param type Typen af WebSocket-hændelse (f.eks. tilslutning, afbrydelse, data).
 * @param arg Eventuelt argument (bruges normalt ikke).
 * @param data Pointer til eventuelle modtagne data (kun relevant ved WS_EVT_DATA).
 * @param len Længden af de modtagne data.
 */
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
                      void *arg, uint8_t *data, size_t len);

#endif
