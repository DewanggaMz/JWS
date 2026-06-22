#include "storage.h"

#include <LittleFS.h>

#include "config.h"

bool initStorage() {
  if (!LittleFS.begin(true)) {
    return false;
  }

  return prepareDatabaseFile();
}

bool prepareDatabaseFile() {
  if (LittleFS.exists(DATABASE_PATH)) {
    return true;
  }

  File database = LittleFS.open(DATABASE_PATH, FILE_WRITE);
  if (!database) {
    return false;
  }

  database.print("{}");
  database.close();
  return true;
}

bool readJsonFile(const char *path, JsonDocument &document) {
  File file = LittleFS.open(path, FILE_READ);
  if (!file) {
    return false;
  }

  DeserializationError error = deserializeJson(document, file);
  file.close();
  return !error;
}

bool writeJsonFile(const char *path, const JsonDocument &document) {
  File file = LittleFS.open(path, FILE_WRITE);
  if (!file) {
    return false;
  }

  const size_t written = serializeJsonPretty(document, file);
  file.close();
  return written > 0;
}

bool saveJsonToDatabase(const JsonDocument &json) {
  return writeJsonFile(DATABASE_PATH, json);
}
