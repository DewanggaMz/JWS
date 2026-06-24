#include "handlers.h"

#include <ArduinoJson.h>

#include "utils/json_utils.h"
#include "storage.h"
#include "services/prayer_times/prayer_times_service.h"
#include "date_and_time.h"

void handleNotFound(AsyncWebServerRequest *request) {
  sendJsonResponse(request, 404, "Endpoint tidak ditemukan");
}

void handleHelloWorldGet(AsyncWebServerRequest *request) {
  JsonDocument response;
  response["text"] = "hello world";

  sendJsonDocument(request, 200, response);
}

void handlePrayerConfigGet(AsyncWebServerRequest *request) {
  JsonDocument config;
  if (!loadPrayerTimesConfig(config)) {
    sendJsonResponse(request, 500, "Gagal membaca konfigurasi prayerTimes");
    return;
  }

  JsonDocument response;
  response["success"] = true;
  response["prayerTimes"].set(config.as<JsonVariantConst>());

  sendJsonDocument(request, 200, response);
}

void handleDatabasePostJson(AsyncWebServerRequest *request, JsonVariant &json) {
  if (!json.is<JsonObject>() && !json.is<JsonArray>()) {
    sendJsonResponse(request, 400, "JSON harus berupa object atau array");
    return;
  }

  JsonDocument document;
  document.set(json);

  if (!saveJsonToDatabase(document)) {
    sendJsonResponse(request, 500, "Gagal menyimpan data ke LittleFS");
    return;
  }

  sendJsonResponse(request, 200, "Data berhasil disimpan ke database.json");
}


void handlePrayerConfigPostJson(AsyncWebServerRequest *request, JsonVariant &json) {
  Serial.println("handlePrayerConfigPostJson");
  String message;
  if (!updatePrayerTimesConfig(json.as<JsonVariantConst>(), message)) {
    sendJsonResponse(request, 400, message.c_str());
    return;
  }

  JsonDocument config;
  loadPrayerTimesConfig(config);

  JsonDocument response;
  response["success"] = true;
  response["message"] = message;
  response["prayerTimes"].set(config.as<JsonVariantConst>());

  sendJsonDocument(request, 200, response);
}

void handleDateTimeAdjustmentPostJson(AsyncWebServerRequest *request, JsonVariant &json) {
  Serial.println("handleDateTimeAdjustmentPostJson");
  String message;
  if (!updateDateTimeAdjustment(json.as<JsonVariantConst>(), message)) {
    sendJsonResponse(request, 400, message.c_str());
    return;
  }

  JsonDocument response;
  response["success"] = true;
  response["message"] = message;

  sendJsonDocument(request, 200, response);
}
