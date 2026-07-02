#include "services/panel_config/panel_config_service.h"

#include <stdio.h>
#include <string.h>

#include "services/database/database_service.h"

namespace {

const uint8_t DEFAULT_BRIGHTNESS = 200;
const uint8_t DEFAULT_PANEL_COUNT = 3;
const uint8_t MIN_PANEL_COUNT = 3;
const uint8_t MAX_PANEL_COUNT = 5;
const bool DEFAULT_DIM_ENABLED = true;
const char DEFAULT_DIM_START_TIME[] = "22:00";
const char DEFAULT_DIM_END_TIME[] = "04:00";
const uint8_t DEFAULT_DIM_BRIGHTNESS = 50;

bool isValidBrightness(JsonVariantConst value)
{
  if (!value.is<int>()) {
    return false;
  }

  const int brightness = value.as<int>();
  return brightness >= 0 && brightness <= 255;
}

bool isValidPanelCount(JsonVariantConst value)
{
  if (!value.is<int>()) {
    return false;
  }

  const int panelCount = value.as<int>();
  return panelCount >= MIN_PANEL_COUNT &&
         panelCount <= MAX_PANEL_COUNT;
}

bool parseTime(const char *value, uint16_t &minutes)
{
  if (value == nullptr ||
      strlen(value) != 5 ||
      value[2] != ':' ||
      value[0] < '0' || value[0] > '9' ||
      value[1] < '0' || value[1] > '9' ||
      value[3] < '0' || value[3] > '9' ||
      value[4] < '0' || value[4] > '9') {
    return false;
  }

  const uint8_t hour =
    static_cast<uint8_t>((value[0] - '0') * 10 + (value[1] - '0'));
  const uint8_t minute =
    static_cast<uint8_t>((value[3] - '0') * 10 + (value[4] - '0'));
  if (hour > 23 || minute > 59) {
    return false;
  }

  minutes = static_cast<uint16_t>(hour * 60U + minute);
  return true;
}

void setDefaultDimSchedule(JsonObject schedule)
{
  schedule["enabled"] = DEFAULT_DIM_ENABLED;
  schedule["startTime"] = DEFAULT_DIM_START_TIME;
  schedule["endTime"] = DEFAULT_DIM_END_TIME;
  schedule["dimBrightness"] = DEFAULT_DIM_BRIGHTNESS;
}

void ensureDimScheduleDefaults(JsonObject schedule)
{
  if (!schedule["enabled"].is<bool>()) {
    schedule["enabled"] = DEFAULT_DIM_ENABLED;
  }
  if (!schedule["startTime"].is<const char *>()) {
    schedule["startTime"] = DEFAULT_DIM_START_TIME;
  }
  if (!schedule["endTime"].is<const char *>()) {
    schedule["endTime"] = DEFAULT_DIM_END_TIME;
  }
  if (!isValidBrightness(schedule["dimBrightness"])) {
    schedule["dimBrightness"] = DEFAULT_DIM_BRIGHTNESS;
  }
}

bool parsePanelConfig(
  JsonVariantConst source,
  PanelConfig &config,
  String &message
)
{
  if (!source.is<JsonObjectConst>()) {
    message = "Konfigurasi panel harus berupa object";
    return false;
  }

  JsonObjectConst object = source.as<JsonObjectConst>();
  JsonObjectConst schedule = object["dimSchedule"].as<JsonObjectConst>();
  if (!isValidBrightness(object["brightness"]) ||
      !isValidPanelCount(object["panelCount"]) ||
      !schedule["enabled"].is<bool>() ||
      !schedule["startTime"].is<const char *>() ||
      !schedule["endTime"].is<const char *>() ||
      !isValidBrightness(schedule["dimBrightness"])) {
    message = "Tipe data konfigurasi panel tidak valid";
    return false;
  }

  uint16_t startMinutes = 0;
  uint16_t endMinutes = 0;
  if (!parseTime(schedule["startTime"].as<const char *>(), startMinutes) ||
      !parseTime(schedule["endTime"].as<const char *>(), endMinutes)) {
    message = "Format waktu harus HH:MM";
    return false;
  }
  if (startMinutes == endMinutes) {
    message = "startTime dan endTime tidak boleh sama";
    return false;
  }

  config.brightness =
    static_cast<uint8_t>(object["brightness"].as<int>());
  config.panelCount =
    static_cast<uint8_t>(object["panelCount"].as<int>());
  config.dimEnabled = schedule["enabled"].as<bool>();
  config.dimStartMinutes = startMinutes;
  config.dimEndMinutes = endMinutes;
  config.dimBrightness =
    static_cast<uint8_t>(schedule["dimBrightness"].as<int>());
  return true;
}

}

bool ensurePanelConfig()
{
  DatabaseGuard guard;
  if (!guard) {
    return false;
  }

  JsonDocument database;
  if (!loadDatabase(database)) {
    return false;
  }

  bool changed = false;
  if (!database["panelConfig"].is<JsonObject>()) {
    database["panelConfig"].to<JsonObject>();
    changed = true;
  }

  JsonObject panelConfig = database["panelConfig"].as<JsonObject>();
  if (!isValidBrightness(panelConfig["brightness"])) {
    panelConfig["brightness"] = DEFAULT_BRIGHTNESS;
    changed = true;
  }
  if (!isValidPanelCount(panelConfig["panelCount"])) {
    panelConfig["panelCount"] = DEFAULT_PANEL_COUNT;
    changed = true;
  }

  if (!panelConfig["dimSchedule"].is<JsonObject>()) {
    JsonObject schedule = panelConfig["dimSchedule"].to<JsonObject>();
    setDefaultDimSchedule(schedule);
    changed = true;
  } else {
    JsonObject schedule = panelConfig["dimSchedule"].as<JsonObject>();
    const bool scheduleValid =
      schedule["enabled"].is<bool>() &&
      schedule["startTime"].is<const char *>() &&
      schedule["endTime"].is<const char *>() &&
      isValidBrightness(schedule["dimBrightness"]);
    ensureDimScheduleDefaults(schedule);
    changed |= !scheduleValid;

    PanelConfig parsed;
    String message;
    if (!parsePanelConfig(panelConfig, parsed, message)) {
      setDefaultDimSchedule(schedule);
      changed = true;
    }
  }

  return !changed || saveDatabase(database);
}

bool loadPanelConfig(PanelConfig &config)
{
  JsonDocument database;
  if (!loadDatabase(database)) {
    return false;
  }

  String message;
  return parsePanelConfig(database["panelConfig"], config, message);
}

bool updatePanelConfig(
  JsonVariantConst payload,
  PanelConfig &config,
  String &message
)
{
  DatabaseGuard guard;
  if (!guard) {
    message = "Database sedang digunakan";
    return false;
  }

  if (!payload.is<JsonObjectConst>()) {
    message = "Payload konfigurasi panel harus berupa object";
    return false;
  }

  JsonObjectConst input = payload.as<JsonObjectConst>();
  const bool hasBrightness = !input["brightness"].isUnbound();
  const bool hasPanelCount = !input["panelCount"].isUnbound();
  const bool hasDimSchedule = !input["dimSchedule"].isUnbound();
  const bool hasEnabled = !input["enabled"].isUnbound();
  const bool hasStartTime = !input["startTime"].isUnbound();
  const bool hasEndTime = !input["endTime"].isUnbound();
  const bool hasDimBrightness = !input["dimBrightness"].isUnbound();
  if (!hasBrightness &&
      !hasPanelCount &&
      !hasDimSchedule &&
      !hasEnabled &&
      !hasStartTime &&
      !hasEndTime &&
      !hasDimBrightness) {
    message = "Tidak ada field konfigurasi panel yang dikenali";
    return false;
  }

  if (hasBrightness && !isValidBrightness(input["brightness"])) {
    message = "Field brightness harus berupa angka 0 sampai 255";
    return false;
  }
  if (hasPanelCount && !isValidPanelCount(input["panelCount"])) {
    message = "Field panelCount harus berupa angka 3 sampai 5";
    return false;
  }
  if (hasDimSchedule && !input["dimSchedule"].is<JsonObjectConst>()) {
    message = "Field dimSchedule harus berupa object";
    return false;
  }
  if (hasEnabled && !input["enabled"].is<bool>()) {
    message = "Field enabled harus berupa boolean";
    return false;
  }
  if (hasStartTime && !input["startTime"].is<const char *>()) {
    message = "Field startTime harus berupa string HH:MM";
    return false;
  }
  if (hasEndTime && !input["endTime"].is<const char *>()) {
    message = "Field endTime harus berupa string HH:MM";
    return false;
  }
  if (hasDimBrightness &&
      !isValidBrightness(input["dimBrightness"])) {
    message = "Field dimBrightness harus berupa angka 0 sampai 255";
    return false;
  }
  if (hasDimSchedule) {
    JsonObjectConst inputSchedule =
      input["dimSchedule"].as<JsonObjectConst>();
    if (!inputSchedule["enabled"].isUnbound() &&
        !inputSchedule["enabled"].is<bool>()) {
      message = "Field dimSchedule.enabled harus berupa boolean";
      return false;
    }
    if (!inputSchedule["startTime"].isUnbound() &&
        !inputSchedule["startTime"].is<const char *>()) {
      message = "Field dimSchedule.startTime harus berupa string HH:MM";
      return false;
    }
    if (!inputSchedule["endTime"].isUnbound() &&
        !inputSchedule["endTime"].is<const char *>()) {
      message = "Field dimSchedule.endTime harus berupa string HH:MM";
      return false;
    }
    if (!inputSchedule["dimBrightness"].isUnbound() &&
        !isValidBrightness(inputSchedule["dimBrightness"])) {
      message = "Field dimSchedule.dimBrightness harus berupa angka 0 sampai 255";
      return false;
    }
  }

  JsonDocument database;
  if (!loadDatabase(database)) {
    message = "Gagal membaca database";
    return false;
  }
  JsonObject panelConfig = database["panelConfig"].is<JsonObject>()
                             ? database["panelConfig"].as<JsonObject>()
                             : database["panelConfig"].to<JsonObject>();
  if (!isValidBrightness(panelConfig["brightness"])) {
    panelConfig["brightness"] = DEFAULT_BRIGHTNESS;
  }
  if (!isValidPanelCount(panelConfig["panelCount"])) {
    panelConfig["panelCount"] = DEFAULT_PANEL_COUNT;
  }
  JsonObject schedule = panelConfig["dimSchedule"].is<JsonObject>()
                          ? panelConfig["dimSchedule"].as<JsonObject>()
                          : panelConfig["dimSchedule"].to<JsonObject>();
  ensureDimScheduleDefaults(schedule);
  PanelConfig currentConfig;
  String parseMessage;
  if (!parsePanelConfig(panelConfig, currentConfig, parseMessage)) {
    setDefaultDimSchedule(schedule);
  }

  if (hasBrightness) {
    panelConfig["brightness"] = input["brightness"].as<int>();
  }
  if (hasPanelCount) {
    panelConfig["panelCount"] = input["panelCount"].as<int>();
  }
  if (hasEnabled) {
    schedule["enabled"] = input["enabled"].as<bool>();
  }
  if (hasStartTime) {
    schedule["startTime"] = input["startTime"].as<const char *>();
  }
  if (hasEndTime) {
    schedule["endTime"] = input["endTime"].as<const char *>();
  }
  if (hasDimBrightness) {
    schedule["dimBrightness"] = input["dimBrightness"].as<int>();
  }
  if (hasDimSchedule) {
    JsonObjectConst inputSchedule =
      input["dimSchedule"].as<JsonObjectConst>();
    if (!inputSchedule["enabled"].isUnbound()) {
      schedule["enabled"] = inputSchedule["enabled"].as<bool>();
    }
    if (!inputSchedule["startTime"].isUnbound()) {
      schedule["startTime"] =
        inputSchedule["startTime"].as<const char *>();
    }
    if (!inputSchedule["endTime"].isUnbound()) {
      schedule["endTime"] =
        inputSchedule["endTime"].as<const char *>();
    }
    if (!inputSchedule["dimBrightness"].isUnbound()) {
      schedule["dimBrightness"] =
        inputSchedule["dimBrightness"].as<int>();
    }
  }

  if (!parsePanelConfig(panelConfig, config, message)) {
    return false;
  }
  if (!saveDatabase(database)) {
    message = "Gagal menyimpan konfigurasi panel";
    return false;
  }

  message = "Konfigurasi panel berhasil diperbarui";
  return true;
}

String formatPanelScheduleTime(uint16_t minutes)
{
  char value[6];
  snprintf(
    value,
    sizeof(value),
    "%02u:%02u",
    minutes / 60U,
    minutes % 60U
  );
  return String(value);
}
