#pragma once

#include <ArduinoJson.h>

bool initStorage();
bool prepareDatabaseFile();
bool readJsonFile(const char *path, JsonDocument &document);
bool writeJsonFile(const char *path, const JsonDocument &document);
bool saveJsonToDatabase(const JsonDocument &json);
