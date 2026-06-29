#pragma once

#include <stdint.h>

constexpr const char *DEFAULT_WIFI_AP_SSID = "esp32";
constexpr const char *DEFAULT_WIFI_AP_PASSWORD = "11223344";
constexpr const char *DEFAULT_WIFI_MODE = "AP";
constexpr unsigned long WIFI_STA_CONNECT_TIMEOUT_MS = 15000;

constexpr const char *DATABASE_PATH = "/database.json";
constexpr const char *DEFAULT_PRAYER_TIMES_PATH = "/default_prayer_times.json";
constexpr uint16_t HTTP_SERVER_PORT = 80;

constexpr const int PIN_SDA = 16;
constexpr const int PIN_SCL = 17;
