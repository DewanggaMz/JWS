#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

struct PanelConfig {
  uint8_t brightness;
  uint8_t panelCount;
  bool dimEnabled;
  uint16_t dimStartMinutes;
  uint16_t dimEndMinutes;
  uint8_t dimBrightness;
};

bool ensurePanelConfig();
bool loadPanelConfig(PanelConfig &config);
bool updatePanelConfig(
  JsonVariantConst payload,
  PanelConfig &config,
  String &message
);
String formatPanelScheduleTime(uint16_t minutes);
