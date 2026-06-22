#include "handlers.h"

#include <ArduinoJson.h>

#include "utils/json_utils.h"
#include "storage.h"

void handleHelloWorldGet(AsyncWebServerRequest *request) {
  JsonDocument response;
  response["text"] = "hello world";

  sendJsonDocument(request, 200, response);
}

void collectDatabaseBody(AsyncWebServerRequest *request, uint8_t *data, size_t len,
                         size_t index, size_t total) {
  String *body = static_cast<String *>(request->_tempObject);

  if (index == 0) {
    body = new String();
    body->reserve(total);
    request->_tempObject = body;
  }

  for (size_t i = 0; i < len; i++) {
    body->concat(static_cast<char>(data[i]));
  }
}

void handleDatabasePost(AsyncWebServerRequest *request) {
  String *body = static_cast<String *>(request->_tempObject);

  if (body == nullptr || body->isEmpty()) {
    delete body;
    request->_tempObject = nullptr;
    sendJsonResponse(request, 400, "Body JSON wajib dikirim");
    return;
  }

  JsonDocument document;
  DeserializationError error = deserializeJson(document, *body);

  delete body;
  request->_tempObject = nullptr;

  if (error) {
    sendJsonResponse(request, 400, "Body request bukan JSON yang valid");
    return;
  }

  if (!document.is<JsonObject>() && !document.is<JsonArray>()) {
    sendJsonResponse(request, 400, "JSON harus berupa object atau array");
    return;
  }

  if (!saveJsonToDatabase(document)) {
    sendJsonResponse(request, 500, "Gagal menyimpan data ke LittleFS");
    return;
  }

  sendJsonResponse(request, 200, "Data berhasil disimpan ke database.json");
}

void handleNotFound(AsyncWebServerRequest *request) {
  sendJsonResponse(request, 404, "Endpoint tidak ditemukan");
}
