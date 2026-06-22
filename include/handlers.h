#pragma once

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

void handleHelloWorldGet(AsyncWebServerRequest *request);
void handleDatabasePostJson(AsyncWebServerRequest *request, JsonVariant &json);
void handlePrayerConfigGet(AsyncWebServerRequest *request);
void handlePrayerConfigPostJson(AsyncWebServerRequest *request, JsonVariant &json);
void handleNotFound(AsyncWebServerRequest *request);
