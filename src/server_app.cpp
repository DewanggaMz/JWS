#include "server_app.h"

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <LittleFS.h>

#include "config.h"
#include "handlers.h"


AsyncWebServer server(HTTP_SERVER_PORT);

void setupServer() {
  DefaultHeaders::Instance().addHeader(
    "Access-Control-Allow-Origin", "*"
  );
  DefaultHeaders::Instance().addHeader(
    "Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS"
  );
  DefaultHeaders::Instance().addHeader(
    "Access-Control-Allow-Headers", "Content-Type, Authorization"
  );



  // server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
  server.on("/dummy", HTTP_GET, handleHelloWorldGet);
  server.on("/konfigurasi_prayer", HTTP_GET, handlePrayerConfigGet);
  server.on("/database", HTTP_GET, handleDatabaseGet);

  AsyncCallbackJsonWebHandler *databaseHandler =
    new AsyncCallbackJsonWebHandler("/database", handleDatabasePostJson);
  databaseHandler->setMethod(HTTP_POST);
  server.addHandler(databaseHandler);

  server.addHandler(new AsyncCallbackJsonWebHandler("/api/helloworld", [](AsyncWebServerRequest *request, JsonVariant &json) {
    // send response helloworld
    handleHelloWorldPostJson(request, json);
  }));

  AsyncCallbackJsonWebHandler *helloWorld =
    new AsyncCallbackJsonWebHandler("/hello", handleHelloWorldPostJson);
  helloWorld->setMethod(HTTP_POST);
  server.addHandler(helloWorld);


  AsyncCallbackJsonWebHandler *prayerConfigHandler =
    new AsyncCallbackJsonWebHandler("/konfigurasi_prayer", handlePrayerConfigPostJson);
  prayerConfigHandler->setMethod(HTTP_POST);
  server.addHandler(prayerConfigHandler);


  AsyncCallbackJsonWebHandler *dateTimeAdjustHandler =
    new AsyncCallbackJsonWebHandler("/time_adjust", handleDateTimeAdjustmentPostJson);
  dateTimeAdjustHandler->setMethod(HTTP_POST);
  server.addHandler(dateTimeAdjustHandler);

  AsyncCallbackJsonWebHandler *layout1MessageHandler =
    new AsyncCallbackJsonWebHandler(
      "/api/messages/layout1",
      handleLayout1MessagePostJson
    );
  layout1MessageHandler->setMethod(HTTP_POST);
  server.addHandler(layout1MessageHandler);

  AsyncCallbackJsonWebHandler *layout2MessageHandler =
    new AsyncCallbackJsonWebHandler(
      "/api/messages/layout2",
      handleLayout2MessagePostJson
    );
  layout2MessageHandler->setMethod(HTTP_POST);
  server.addHandler(layout2MessageHandler);

  AsyncCallbackJsonWebHandler *layout3MessageHandler =
    new AsyncCallbackJsonWebHandler(
      "/api/messages/layout3",
      handleLayout3MessagePostJson
    );
  layout3MessageHandler->setMethod(HTTP_POST);
  server.addHandler(layout3MessageHandler);

  AsyncCallbackJsonWebHandler *layout4MessageHandler =
    new AsyncCallbackJsonWebHandler(
      "/api/messages/layout4",
      handleLayout4MessagePostJson
    );
  layout4MessageHandler->setMethod(HTTP_POST);
  server.addHandler(layout4MessageHandler);

  AsyncCallbackJsonWebHandler *layout1PrayerDisplayHandler =
    new AsyncCallbackJsonWebHandler(
      "/api/layout1/prayer-times",
      handleLayout1PrayerDisplayPostJson
    );
  layout1PrayerDisplayHandler->setMethod(HTTP_POST);
  server.addHandler(layout1PrayerDisplayHandler);

  AsyncCallbackJsonWebHandler *wifiConfigHandler =
    new AsyncCallbackJsonWebHandler(
      "/api/wifi/config",
      handleWiFiConfigPostJson
    );
  wifiConfigHandler->setMethod(HTTP_POST);
  server.addHandler(wifiConfigHandler);

  AsyncCallbackJsonWebHandler *relayConfigHandler =
    new AsyncCallbackJsonWebHandler(
      "/api/relay/config",
      handleRelayConfigPostJson
    );
  relayConfigHandler->setMethod(HTTP_POST);
  server.addHandler(relayConfigHandler);

  AsyncCallbackJsonWebHandler *relayPrayerStatesHandler =
    new AsyncCallbackJsonWebHandler(
      "/api/relay/prayer-states",
      handleRelayPrayerStatesPostJson
    );
  relayPrayerStatesHandler->setMethod(HTTP_POST);
  server.addHandler(relayPrayerStatesHandler);


  // server.onNotFound(handleNotFound);

  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
  
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.html", "text/html");
  });
  server.begin();
  Serial.printf("HTTP server berjalan di port %u\n", HTTP_SERVER_PORT);
}
