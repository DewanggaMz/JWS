#pragma once

#include <stdint.h>

constexpr const char *WIFI_SSID = "unknown_fuz";
constexpr const char *WIFI_PASSWORD = "Dewanggamz56";
constexpr const char *WIFI_AP_SSID = "esp32";
constexpr const char *WIFI_AP_PASSWORD = "11223344";
constexpr bool WIFI_ENABLE_AP_STA = false;
constexpr bool WIFI_ENABLE_AP_FALLBACK = true;
constexpr unsigned long WIFI_CONNECT_TIMEOUT_MS = 15000;

constexpr const char *DATABASE_PATH = "/database.json";
constexpr const char *DEFAULT_PRAYER_TIMES_PATH = "/default_prayer_times.json";
constexpr uint16_t HTTP_SERVER_PORT = 80;

constexpr const int PIN_SDA = 16;
constexpr const int PIN_SCL = 17;
