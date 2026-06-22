#pragma once

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

void sendJsonResponse(AsyncWebServerRequest *request, int statusCode, const char *message);
void sendJsonDocument(AsyncWebServerRequest *request, int statusCode, const JsonDocument &document);
