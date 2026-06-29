#include "services/panel_config/panel_config_service.h"

#include <stdio.h>
#include <string.h>

#include "services/database/database_service.h"

namespace {

const uint8_t DEFAULT_BRIGHTNESS = 200;
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
  JsonDocument database;
  loadDatabase(database);

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

  if (!panelConfig["dimSchedule"].is<JsonObject>()) {
    JsonObject schedule = panelConfig["dimSchedule"].to<JsonObject>();
    setDefaultDimSchedule(schedule);
    changed = true;
  } else {
    PanelConfig parsed;
    String message;
    if (!parsePanelConfig(panelConfig, parsed, message)) {
      setDefaultDimSchedule(
        panelConfig["dimSchedule"].as<JsonObject>()
      );
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
  if (!payload.is<JsonObjectConst>()) {
    message = "Payload konfigurasi panel harus berupa object";
    return false;
  }

  JsonVariantConst brightness = payload["brightness"];
  if (brightness.isUnbound()) {
    message = "Field brightness wajib diisi";
    return false;
  }
  if (!isValidBrightness(brightness)) {
    message = "Field brightness harus berupa angka 0 sampai 255";
    return false;
  }

  JsonDocument database;
  loadDatabase(database);
  JsonObject panelConfig = database["panelConfig"].is<JsonObject>()
                             ? database["panelConfig"].as<JsonObject>()
                             : database["panelConfig"].to<JsonObject>();
  panelConfig["brightness"] = brightness.as<int>();
  if (!panelConfig["dimSchedule"].is<JsonObject>()) {
    setDefaultDimSchedule(
      panelConfig["dimSchedule"].to<JsonObject>()
    );
  }

  if (!parsePanelConfig(panelConfig, config, message)) {
    return false;
  }
  if (!saveDatabase(database)) {
    message = "Gagal menyimpan konfigurasi panel";
    return false;
  }

  message = "Kecerahan panel berhasil diperbarui";
  return true;
}

bool updatePanelBrightnessSchedule(
  JsonVariantConst payload,
  PanelConfig &config,
  String &message
)
{
  if (!payload.is<JsonObjectConst>()) {
    message = "Payload jadwal redup harus berupa object";
    return false;
  }

  JsonObjectConst input = payload.as<JsonObjectConst>();
  const bool hasEnabled = !input["enabled"].isUnbound();
  const bool hasStartTime = !input["startTime"].isUnbound();
  const bool hasEndTime = !input["endTime"].isUnbound();
  const bool hasDimBrightness = !input["dimBrightness"].isUnbound();
  if (!hasEnabled &&
      !hasStartTime &&
      !hasEndTime &&
      !hasDimBrightness) {
    message = "Tidak ada field jadwal redup yang dikenali";
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

  JsonDocument database;
  loadDatabase(database);
  JsonObject panelConfig = database["panelConfig"].is<JsonObject>()
                             ? database["panelConfig"].as<JsonObject>()
                             : database["panelConfig"].to<JsonObject>();
  if (!isValidBrightness(panelConfig["brightness"])) {
    panelConfig["brightness"] = DEFAULT_BRIGHTNESS;
  }
  JsonObject schedule = panelConfig["dimSchedule"].is<JsonObject>()
                          ? panelConfig["dimSchedule"].as<JsonObject>()
                          : panelConfig["dimSchedule"].to<JsonObject>();
  if (!schedule["enabled"].is<bool>() ||
      !schedule["startTime"].is<const char *>() ||
      !schedule["endTime"].is<const char *>() ||
      !isValidBrightness(schedule["dimBrightness"])) {
    setDefaultDimSchedule(schedule);
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

  if (!parsePanelConfig(panelConfig, config, message)) {
    return false;
  }
  if (!saveDatabase(database)) {
    message = "Gagal menyimpan jadwal redup panel";
    return false;
  }

  message = "Jadwal redup panel berhasil diperbarui";
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
