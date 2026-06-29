#pragma once

#include <ArduinoJson.h>

#include "panel/PanelMessages.h"

bool ensurePanelMessages();
bool loadPanelMessages(PanelMessages &messages);
void setDefaultPanelMessages(PanelMessages &messages);
bool updatePanelLayoutMessages(
  uint8_t layoutNumber,
  JsonVariantConst payload,
  PanelMessages &messages,
  String &message
);
