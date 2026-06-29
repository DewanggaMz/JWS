#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

struct WiFiConfig {
  String mode;
  String ssid;
  String password;
};

bool ensureWiFiConfig();
bool loadWiFiConfig(WiFiConfig &config);
bool updateWiFiConfig(
  JsonVariantConst payload,
  WiFiConfig &config,
  String &message
);
