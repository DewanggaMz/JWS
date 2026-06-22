#pragma once

#include <ArduinoJson.h>

bool loadDatabase(JsonDocument &database);
bool saveDatabase(const JsonDocument &database);
