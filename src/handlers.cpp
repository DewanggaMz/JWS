#include "handlers.h"

#include <ArduinoJson.h>

#include "config.h"
#include "utils/json_utils.h"
#include "storage.h"
#include "services/prayer_times/prayer_times_service.h"
#include "services/panel_messages/panel_messages_service.h"
#include "services/panel_config/panel_config_service.h"
#include "services/wifi_config/wifi_config_service.h"
#include "services/relay/relay_service.h"
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
    data["speedMs"] = panelMessages.layout1SpeedMs;
  } else if (layoutNumber == 2) {
    data["running"] = panelMessages.layout2Running;
    data["speedMs"] = panelMessages.layout2SpeedMs;
  } else if (layoutNumber == 3) {
    JsonArray slides = data["slides"].to<JsonArray>();
    for (uint8_t i = 0; i < panelMessages.layout3SlideCount; i++) {
      slides.add(panelMessages.layout3Slides[i]);
    }
  } else if (layoutNumber == 4) {
    data["running"] = panelMessages.layout4Running;
    data["showPasaran"] = panelMessages.layout4ShowPasaran;
    data["showHijriDate"] = panelMessages.layout4ShowHijriDate;
    data["repeatCount"] = panelMessages.layout4RepeatCount;
    data["speedMs"] = panelMessages.layout4SpeedMs;
  } else {
    data["speedMs"] = panelMessages.layout5SpeedMs;
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


void handlePrayerConfigPostJson(AsyncWebServerRequest *request, JsonVariant &json) {
  String message;
  if (!updatePrayerTimesConfig(json.as<JsonVariantConst>(), message)) {
    sendJsonResponse(request, 400, message.c_str());
    return;
  }

  JsonDocument config;
  loadPrayerTimesConfig(config);
  requestPrayerScheduleRefresh();

  JsonDocument response;
  response["success"] = true;
  response["message"] = message;
  response["prayerTimesConfig"].set(config.as<JsonVariantConst>());
  response["scheduleRefreshQueued"] = true;

  sendJsonDocument(request, 200, response);
}

void handleDateTimeAdjustmentPostJson(AsyncWebServerRequest *request, JsonVariant &json) {
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

void handleLayout5MessagePostJson(
  AsyncWebServerRequest *request,
  JsonVariant &json
) {
  handlePanelLayoutMessagePostJson(request, json, 5);
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

void handlePanelBrightnessPostJson(
  AsyncWebServerRequest *request,
  JsonVariant &json
) {
  PanelConfig config;
  String message;
  if (!updatePanelConfig(
        json.as<JsonVariantConst>(),
        config,
        message
      )) {
    sendJsonResponse(request, 400, message.c_str());
    return;
  }

  if (!queuePanelConfigUpdate(config)) {
    sendJsonResponse(
      request,
      500,
      "Kecerahan tersimpan tetapi gagal diterapkan ke panel"
    );
    return;
  }

  JsonDocument response;
  response["success"] = true;
  response["message"] = message;
  response["panelConfig"]["brightness"] = config.brightness;
  sendJsonDocument(request, 200, response);
}

void handlePanelBrightnessSchedulePostJson(
  AsyncWebServerRequest *request,
  JsonVariant &json
) {
  PanelConfig config;
  String message;
  if (!updatePanelBrightnessSchedule(
        json.as<JsonVariantConst>(),
        config,
        message
      )) {
    sendJsonResponse(request, 400, message.c_str());
    return;
  }

  if (!queuePanelConfigUpdate(config)) {
    sendJsonResponse(
      request,
      500,
      "Jadwal tersimpan tetapi gagal diterapkan ke panel"
    );
    return;
  }

  JsonDocument response;
  response["success"] = true;
  response["message"] = message;
  JsonObject schedule =
    response["dimSchedule"].to<JsonObject>();
  schedule["enabled"] = config.dimEnabled;
  schedule["startTime"] =
    formatPanelScheduleTime(config.dimStartMinutes);
  schedule["endTime"] =
    formatPanelScheduleTime(config.dimEndMinutes);
  schedule["dimBrightness"] = config.dimBrightness;
  sendJsonDocument(request, 200, response);
}

void handleRelayConfigPostJson(
  AsyncWebServerRequest *request,
  JsonVariant &json
) {
  RelayConfig config;
  String message;
  if (!updateRelayConfig(
        json.as<JsonVariantConst>(),
        config,
        message
      )) {
    sendJsonResponse(request, 400, message.c_str());
    return;
  }

  requestRelayConfigReload();

  JsonDocument response;
  response["success"] = true;
  response["message"] = message;
  response["relayConfig"]["enabled"] = config.enabled;
  response["relayConfig"]["prePrayerMinutes"] =
    config.prePrayerMinutes;
  response["relayConfig"]["fridayPrePrayerMinutes"] =
    config.fridayPrePrayerMinutes;
  response["relayConfig"]["relay12OnDelaySeconds"] =
    config.relay12OnDelaySeconds;
  response["relayConfig"]["relay12OffDelayMinutes"] =
    config.relay12OffDelayMinutes;
  sendJsonDocument(request, 200, response);
}

void handleRelayPrayerStatesPostJson(
  AsyncWebServerRequest *request,
  JsonVariant &json
) {
  RelayConfig config;
  String message;
  if (!updateRelayPrayerStates(
        json.as<JsonVariantConst>(),
        config,
        message
      )) {
    sendJsonResponse(request, 400, message.c_str());
    return;
  }

  requestRelayConfigReload();

  JsonDocument response;
  response["success"] = true;
  response["message"] = message;
  response["prayerStates"]["tartilSubuh"] = config.tartilSubuh;
  response["prayerStates"]["tartilDzuhur"] = config.tartilDzuhur;
  response["prayerStates"]["tartilJumat"] = config.tartilJumat;
  response["prayerStates"]["tartilAshar"] = config.tartilAshar;
  response["prayerStates"]["tartilMagrib"] = config.tartilMagrib;
  response["prayerStates"]["tartilIsha"] = config.tartilIsha;
  sendJsonDocument(request, 200, response);
}
