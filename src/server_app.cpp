#include "server_app.h"

#include <Arduino.h>
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

#include "config.h"
#include "handlers.h"

namespace {

AsyncWebServer server(HTTP_SERVER_PORT);

struct JsonPostRoute {
  const char *path;
  ArJsonRequestHandlerFunction handler;
};

void registerJsonPostRoute(const JsonPostRoute &route)
{
  AsyncCallbackJsonWebHandler *handler =
    new AsyncCallbackJsonWebHandler(route.path, route.handler);
  handler->setMethod(HTTP_POST);
  server.addHandler(handler);
}

void configureCors()
{
  DefaultHeaders::Instance().addHeader(
    "Access-Control-Allow-Origin",
    "*"
  );
  DefaultHeaders::Instance().addHeader(
    "Access-Control-Allow-Methods",
    "GET, POST, OPTIONS"
  );
  DefaultHeaders::Instance().addHeader(
    "Access-Control-Allow-Headers",
    "Content-Type, Authorization"
  );
}

void handleFallback(AsyncWebServerRequest *request)
{
  if (request->method() == HTTP_OPTIONS) {
    request->send(204);
    return;
  }

  const bool isApiRequest =
    request->url().startsWith("/api/") ||
    request->url() == "/konfigurasi_prayer" ||
    request->url() == "/time_adjust" ||
    request->url() == "/database";
  if (request->method() != HTTP_GET || isApiRequest) {
    handleNotFound(request);
    return;
  }

  if (LittleFS.exists("/index.html")) {
    request->send(LittleFS, "/index.html", "text/html");
  } else {
    handleNotFound(request);
  }
}

}

void setupServer()
{
  configureCors();

  server.on(
    "/konfigurasi_prayer",
    HTTP_GET,
    handlePrayerConfigGet
  );
  server.on("/database", HTTP_GET, handleDatabaseGet);

  const JsonPostRoute routes[] = {
    {"/konfigurasi_prayer", handlePrayerConfigPostJson},
    {"/time_adjust", handleDateTimeAdjustmentPostJson},
    {"/api/messages/layout1", handleLayout1MessagePostJson},
    {"/api/messages/layout2", handleLayout2MessagePostJson},
    {"/api/messages/layout3", handleLayout3MessagePostJson},
    {"/api/messages/layout4", handleLayout4MessagePostJson},
    {"/api/messages/layout5", handleLayout5MessagePostJson},
    {"/api/layout1/prayer-times", handleLayout1PrayerDisplayPostJson},
    {"/api/wifi/config", handleWiFiConfigPostJson},
    {"/api/panel/brightness", handlePanelBrightnessPostJson},
    {
      "/api/panel/brightness-schedule",
      handlePanelBrightnessSchedulePostJson
    },
    {"/api/relay/config", handleRelayConfigPostJson},
    {"/api/relay/prayer-states", handleRelayPrayerStatesPostJson}
  };

  for (const JsonPostRoute &route : routes) {
    registerJsonPostRoute(route);
  }

  server
    .serveStatic("/", LittleFS, "/")
    .setDefaultFile("index.html");
  server.onNotFound(handleFallback);
  server.begin();
  Serial.printf(
    "HTTP server berjalan di port %u\n",
    HTTP_SERVER_PORT
  );
}
