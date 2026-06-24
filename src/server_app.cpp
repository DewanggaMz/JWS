#include "server_app.h"

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>

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
  server.on("/konfigurasi_prayer", HTTP_GET, handlePrayerConfigGet);

  AsyncCallbackJsonWebHandler *databaseHandler =
    new AsyncCallbackJsonWebHandler("/database", handleDatabasePostJson);
  databaseHandler->setMethod(HTTP_POST);
  server.addHandler(databaseHandler);

  AsyncCallbackJsonWebHandler *prayerConfigHandler =
    new AsyncCallbackJsonWebHandler("/konfigurasi_prayer", handlePrayerConfigPostJson);
  prayerConfigHandler->setMethod(HTTP_POST);
  server.addHandler(prayerConfigHandler);

  AsyncCallbackJsonWebHandler *dateTimeAdjustHandler =
    new AsyncCallbackJsonWebHandler("/time_adjust", handleDateTimeAdjustmentPostJson);
  dateTimeAdjustHandler->setMethod(HTTP_POST);
  server.addHandler(dateTimeAdjustHandler);


  server.onNotFound(handleNotFound);
  server.begin();
  Serial.printf("HTTP server berjalan di port %u\n", HTTP_SERVER_PORT);
}
