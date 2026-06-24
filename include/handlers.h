#pragma once

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

void handleNotFound(AsyncWebServerRequest *request);
void handleHelloWorldGet(AsyncWebServerRequest *request);
void handlePrayerConfigGet(AsyncWebServerRequest *request);


void handleDatabasePostJson(AsyncWebServerRequest *request, JsonVariant &json);
void handlePrayerConfigPostJson(AsyncWebServerRequest *request, JsonVariant &json);
void handleDateTimeAdjustmentPostJson(AsyncWebServerRequest *request, JsonVariant &json);
