#include "handlers.h"

#include <ArduinoJson.h>

#include "config.h"
#include "utils/json_utils.h"
#include "storage.h"
#include "services/prayer_times/prayer_times_service.h"
#include "services/panel_messages/panel_messages_service.h"
#include "services/wifi_config/wifi_config_service.h"
#include "datetime/date_and_time.h"
#include "panel/panelSetup.h"

namespace {

void handlePanelLayoutMessagePostJson(
  AsyncWebServerRequest *request,
  JsonVariant &json,
  uint8_t layoutNumber
)
{
  PanelMessages panelMessages;
  String message;
  if (!updatePanelLayoutMessages(
        layoutNumber,
        json.as<JsonVariantConst>(),
        panelMessages,
        message
      )) {
    sendJsonResponse(request, 400, message.c_str());
    return;
  }

  if (!queuePanelMessagesUpdate(panelMessages)) {
    sendJsonResponse(
      request,
      500,
      "Message tersimpan tetapi gagal diterapkan ke panel"
    );
    return;
  }

  JsonDocument response;
  response["success"] = true;
  response["message"] = message;
  response["layout"] = layoutNumber;

  JsonObject data = response["data"].to<JsonObject>();
  if (layoutNumber == 1) {
    data["bottom"] = panelMessages.layout1Bottom;
    data["repeatCount"] = panelMessages.layout1RepeatCount;
  } else if (layoutNumber == 2) {
    data["running"] = panelMessages.layout2Running;
  } else if (layoutNumber == 3) {
    JsonArray slides = data["slides"].to<JsonArray>();
    for (uint8_t i = 0; i < panelMessages.layout3SlideCount; i++) {
      slides.add(panelMessages.layout3Slides[i]);
    }
  } else {
    data["running"] = panelMessages.layout4Running;
    data["showPasaran"] = panelMessages.layout4ShowPasaran;
    data["showHijriDate"] = panelMessages.layout4ShowHijriDate;
    data["repeatCount"] = panelMessages.layout4RepeatCount;
  }

  sendJsonDocument(request, 200, response);
}

}

void handleNotFound(AsyncWebServerRequest *request) {

  if (request->method() == HTTP_OPTIONS) {
    request->send(200);
    return;
  }

  
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
  response["prayerTimesConfig"].set(config.as<JsonVariantConst>());

  sendJsonDocument(request, 200, response);
}

void handleDatabaseGet(AsyncWebServerRequest *request) {
  JsonDocument database;
  if (!readJsonFile(DATABASE_PATH, database)) {
    sendJsonResponse(request, 500, "Gagal membaca database.json");
    return;
  }

  sendJsonDocument(request, 200, database);
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

void handleHelloWorldPostJson(AsyncWebServerRequest *request, JsonVariant &json) {
  Serial.println("handleHelloWorldPostJson");
  JsonDocument response;
  response["success"] = true;
  response["message"] = "hello world";
  response["payload"].set(json.as<JsonVariantConst>());

  sendJsonDocument(request, 200, response);
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
  response["prayerTimesConfig"].set(config.as<JsonVariantConst>());

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

void handleLayout1MessagePostJson(
  AsyncWebServerRequest *request,
  JsonVariant &json
) {
  handlePanelLayoutMessagePostJson(request, json, 1);
}

void handleLayout2MessagePostJson(
  AsyncWebServerRequest *request,
  JsonVariant &json
) {
  handlePanelLayoutMessagePostJson(request, json, 2);
}

void handleLayout3MessagePostJson(
  AsyncWebServerRequest *request,
  JsonVariant &json
) {
  handlePanelLayoutMessagePostJson(request, json, 3);
}

void handleLayout4MessagePostJson(
  AsyncWebServerRequest *request,
  JsonVariant &json
) {
  handlePanelLayoutMessagePostJson(request, json, 4);
}

void handleLayout1PrayerDisplayPostJson(
  AsyncWebServerRequest *request,
  JsonVariant &json
) {
  PanelMessages panelMessages;
  String message;
  if (!updateLayout1PrayerDisplay(
        json.as<JsonVariantConst>(),
        panelMessages,
        message
      )) {
    sendJsonResponse(request, 400, message.c_str());
    return;
  }

  if (!queuePanelMessagesUpdate(panelMessages)) {
    sendJsonResponse(
      request,
      500,
      "Konfigurasi tersimpan tetapi gagal diterapkan ke panel"
    );
    return;
  }

  JsonDocument response;
  response["success"] = true;
  response["message"] = message;
  response["prayerDisplay"]["showImsak"] =
    panelMessages.layout1ShowImsak;
  response["prayerDisplay"]["showSunrise"] =
    panelMessages.layout1ShowSunrise;
  response["prayerDisplay"]["showDhuha"] =
    panelMessages.layout1ShowDhuha;
  sendJsonDocument(request, 200, response);
}

void handleWiFiConfigPostJson(
  AsyncWebServerRequest *request,
  JsonVariant &json
) {
  const bool passwordUpdated =
    json.as<JsonVariantConst>()["password"].is<const char *>();
  WiFiConfig config;
  String message;
  if (!updateWiFiConfig(
        json.as<JsonVariantConst>(),
        config,
        message
      )) {
    sendJsonResponse(request, 400, message.c_str());
    return;
  }

  JsonDocument response;
  response["success"] = true;
  response["message"] = message;
  response["wifiConfig"]["mode"] = config.mode;
  response["wifiConfig"]["ssid"] = config.ssid;
  response["wifiConfig"]["passwordUpdated"] = passwordUpdated;
  response["restartRequired"] = true;
  sendJsonDocument(request, 200, response);
}
