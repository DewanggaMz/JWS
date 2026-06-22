#pragma once

#include <ArduinoJson.h>

bool initStorage();
bool prepareDatabaseFile();
bool saveJsonToDatabase(const JsonDocument &json);
