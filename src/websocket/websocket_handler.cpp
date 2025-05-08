#include "websocket_handler.h"
#include <Arduino.h>  // For Serial

/**
 * @brief WebSocket event handler
 */
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
    void *arg, uint8_t *data, size_t len)
{
if (type == WS_EVT_CONNECT)
{
Serial.println("WebSocket klient forbundet");
}
else if (type == WS_EVT_DISCONNECT)
{
Serial.println("WebSocket klient afbrudt");
}
else if (type == WS_EVT_DATA)
{
// Håndter evt. beskeder fra klienten
}
}
