#include "services/wifi_config/wifi_config_service.h"

#include <string.h>

#include "config.h"
#include "services/database/database_service.h"

namespace {

const size_t MAX_SSID_LENGTH = 32;
const size_t MIN_PASSWORD_LENGTH = 8;
const size_t MAX_PASSWORD_LENGTH = 63;

bool isValidMode(const char *mode)
{
  if (mode == nullptr) {
    return false;
  }

  return strcmp(mode, "AP") == 0 || strcmp(mode, "STA") == 0;
}

bool isValidSsid(const char *ssid)
{
  if (ssid == nullptr) {
    return false;
  }

  const size_t length = strlen(ssid);
  return length > 0 && length <= MAX_SSID_LENGTH;
}

bool isValidPassword(const char *password)
{
  if (password == nullptr) {
    return false;
  }

  const size_t length = strlen(password);
  return length >= MIN_PASSWORD_LENGTH && length <= MAX_PASSWORD_LENGTH;
}

}

bool ensureWiFiConfig()
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
  if (!database["wifiConfig"].is<JsonObject>()) {
    database["wifiConfig"].to<JsonObject>();
    changed = true;
  }

  JsonObject wifiConfig = database["wifiConfig"].as<JsonObject>();
  const char *mode = wifiConfig["mode"] | "";
  const char *ssid = wifiConfig["ssid"] | "";
  const char *password = wifiConfig["password"] | "";
  const bool modeValid = isValidMode(mode);
  const bool ssidValid = isValidSsid(ssid);
  const bool passwordValid = isValidPassword(password);

  if (!modeValid) {
    wifiConfig["mode"] = DEFAULT_WIFI_MODE;
    changed = true;
  }

  if (!ssidValid) {
    wifiConfig["ssid"] = DEFAULT_WIFI_AP_SSID;
    changed = true;
  }

  if (!passwordValid) {
    wifiConfig["password"] = DEFAULT_WIFI_AP_PASSWORD;
    changed = true;
  }

  return !changed || saveDatabase(database);
}

bool loadWiFiConfig(WiFiConfig &config)
{
  config.mode = DEFAULT_WIFI_MODE;
  config.ssid = DEFAULT_WIFI_AP_SSID;
  config.password = DEFAULT_WIFI_AP_PASSWORD;

  JsonDocument database;
  if (!loadDatabase(database) || !database["wifiConfig"].is<JsonObjectConst>()) {
    return false;
  }

  const char *mode = database["wifiConfig"]["mode"] | "";
  const char *ssid = database["wifiConfig"]["ssid"] | "";
  const char *password = database["wifiConfig"]["password"] | "";
  if (!isValidMode(mode) ||
      !isValidSsid(ssid) ||
      !isValidPassword(password)) {
    return false;
  }

  config.mode = mode;
  config.ssid = ssid;
  config.password = password;
  return true;
}

bool updateWiFiConfig(
  JsonVariantConst payload,
  WiFiConfig &config,
  String &message
)
{
  DatabaseGuard guard;
  if (!guard) {
    message = "Database sedang digunakan";
    return false;
  }

  if (!payload.is<JsonObjectConst>()) {
    message = "Payload WiFi harus berupa object";
    return false;
  }

  JsonObjectConst object = payload.as<JsonObjectConst>();
  const bool hasMode = !object["mode"].isUnbound();
  const bool hasSsid = !object["ssid"].isUnbound();
  const bool hasPassword = !object["password"].isUnbound();
  if (!hasMode && !hasSsid && !hasPassword) {
    message = "Isi minimal salah satu field: mode, ssid, atau password";
    return false;
  }

  if (hasMode && !object["mode"].is<const char *>()) {
    message = "Field mode harus berupa string AP atau STA";
    return false;
  }

  if (hasSsid && !object["ssid"].is<const char *>()) {
    message = "Field ssid harus berupa string";
    return false;
  }

  if (hasPassword && !object["password"].is<const char *>()) {
    message = "Field password harus berupa string";
    return false;
  }

  JsonDocument database;
  if (!loadDatabase(database)) {
    message = "Gagal membaca database";
    return false;
  }
  JsonObject storedConfig = database["wifiConfig"].is<JsonObject>()
                              ? database["wifiConfig"].as<JsonObject>()
                              : database["wifiConfig"].to<JsonObject>();

  String mode = hasMode
                  ? object["mode"].as<const char *>()
                  : storedConfig["mode"] | DEFAULT_WIFI_MODE;
  String ssid = hasSsid
                  ? object["ssid"].as<const char *>()
                  : storedConfig["ssid"] | DEFAULT_WIFI_AP_SSID;
  String password = hasPassword
                      ? object["password"].as<const char *>()
                      : storedConfig["password"] | DEFAULT_WIFI_AP_PASSWORD;

  mode.toUpperCase();
  if (!isValidMode(mode.c_str())) {
    message = "Mode WiFi harus AP atau STA";
    return false;
  }

  if (!isValidSsid(ssid.c_str())) {
    message = "SSID wajib diisi dan maksimal 32 karakter";
    return false;
  }

  if (!isValidPassword(password.c_str())) {
    message = "Password WiFi harus 8 sampai 63 karakter";
    return false;
  }

  storedConfig["mode"] = mode;
  storedConfig["ssid"] = ssid;
  storedConfig["password"] = password;
  if (!saveDatabase(database)) {
    message = "Gagal menyimpan konfigurasi WiFi";
    return false;
  }

  config.mode = mode;
  config.ssid = ssid;
  config.password = password;
  message = "Konfigurasi WiFi tersimpan dan aktif setelah perangkat restart";
  return true;
}
