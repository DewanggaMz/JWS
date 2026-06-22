#include "json_utils.h"

void sendJsonResponse(AsyncWebServerRequest *request, int statusCode, const char *message) {
  JsonDocument response;
  response["success"] = statusCode >= 200 && statusCode < 300;
  response["message"] = message;

  sendJsonDocument(request, statusCode, response);
}

void sendJsonDocument(AsyncWebServerRequest *request, int statusCode, const JsonDocument &document) {
  String payload;
  serializeJson(document, payload);
  request->send(statusCode, "application/json", payload);
}
