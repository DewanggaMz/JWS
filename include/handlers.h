#pragma once

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

void handleNotFound(AsyncWebServerRequest *request);
void handleHelloWorldGet(AsyncWebServerRequest *request);
void handlePrayerConfigGet(AsyncWebServerRequest *request);
void handleDatabaseGet(AsyncWebServerRequest *request);

void handleHelloWorldPostJson(AsyncWebServerRequest *request, JsonVariant &json);
void handleDatabasePostJson(AsyncWebServerRequest *request, JsonVariant &json);
void handlePrayerConfigPostJson(AsyncWebServerRequest *request, JsonVariant &json);
void handleDateTimeAdjustmentPostJson(AsyncWebServerRequest *request, JsonVariant &json);
void handleLayout1MessagePostJson(AsyncWebServerRequest *request, JsonVariant &json);
void handleLayout2MessagePostJson(AsyncWebServerRequest *request, JsonVariant &json);
void handleLayout3MessagePostJson(AsyncWebServerRequest *request, JsonVariant &json);
void handleLayout4MessagePostJson(AsyncWebServerRequest *request, JsonVariant &json);
void handleLayout1PrayerDisplayPostJson(AsyncWebServerRequest *request, JsonVariant &json);
void handleWiFiConfigPostJson(AsyncWebServerRequest *request, JsonVariant &json);
void handleRelayConfigPostJson(AsyncWebServerRequest *request, JsonVariant &json);
void handleRelayPrayerStatesPostJson(AsyncWebServerRequest *request, JsonVariant &json);
