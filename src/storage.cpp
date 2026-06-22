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

bool saveJsonToDatabase(const JsonDocument &json) {
  File database = LittleFS.open(DATABASE_PATH, FILE_WRITE);
  if (!database) {
    return false;
  }

  const size_t written = serializeJsonPretty(json, database);
  database.close();
  return written > 0;
}
