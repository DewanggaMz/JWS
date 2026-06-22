#pragma once

#include <ESPAsyncWebServer.h>

void handleHelloWorldGet(AsyncWebServerRequest *request);
void collectDatabaseBody(AsyncWebServerRequest *request, uint8_t *data, size_t len,
                         size_t index, size_t total);
void handleDatabasePost(AsyncWebServerRequest *request);
void handleNotFound(AsyncWebServerRequest *request);
