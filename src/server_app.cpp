#include "server_app.h"

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include "config.h"
#include "handlers.h"

AsyncWebServer server(HTTP_SERVER_PORT);

void setupServer() {
  DefaultHeaders::Instance().addHeader(
    "Access-Control-Allow-Origin", "*"
  );
  DefaultHeaders::Instance().addHeader(
    "Access-Control-Allow-Methods", "GET, POST"
  );
  DefaultHeaders::Instance().addHeader(
    "Access-Control-Allow-Headers", "Content-Type, Authorization"
  );

  // server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

  server.on("/dummy", HTTP_GET, handleHelloWorldGet);
  server.on("/database", HTTP_POST, handleDatabasePost, nullptr, collectDatabaseBody);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.printf("HTTP server berjalan di port %u\n", HTTP_SERVER_PORT);
}
