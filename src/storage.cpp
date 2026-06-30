#include "storage.h"

#include <LittleFS.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include "config.h"

namespace {

SemaphoreHandle_t storageMutex = nullptr;

class StorageGuard {
  public:
    StorageGuard()
      : locked(
          storageMutex != nullptr &&
          xSemaphoreTake(storageMutex, pdMS_TO_TICKS(2000)) == pdTRUE
        )
    {
    }

    ~StorageGuard()
    {
      if (locked) {
        xSemaphoreGive(storageMutex);
      }
    }

    explicit operator bool() const
    {
      return locked;
    }

  private:
    bool locked;
};

bool isValidJsonFile(const char *path, const bool requireObject = false)
{
  File file = LittleFS.open(path, FILE_READ);
  if (!file) {
    return false;
  }

  JsonDocument document;
  const DeserializationError error = deserializeJson(document, file);
  file.close();
  return !error && (!requireObject || document.is<JsonObject>());
}

}

bool initStorage() {
  if (!LittleFS.begin(true)) {
    return false;
  }

  if (storageMutex == nullptr) {
    storageMutex = xSemaphoreCreateMutex();
  }
  if (storageMutex == nullptr) {
    return false;
  }

  return prepareDatabaseFile();
}

bool prepareDatabaseFile() {
  StorageGuard guard;
  if (!guard) {
    return false;
  }

  const String temporaryPath = String(DATABASE_PATH) + ".tmp";
  const String backupPath = String(DATABASE_PATH) + ".bak";

  if (LittleFS.exists(DATABASE_PATH) &&
      isValidJsonFile(DATABASE_PATH, true)) {
    LittleFS.remove(temporaryPath);
    LittleFS.remove(backupPath);
    return true;
  }

  LittleFS.remove(DATABASE_PATH);
  if (LittleFS.exists(backupPath) &&
      isValidJsonFile(backupPath.c_str(), true) &&
      LittleFS.rename(backupPath, DATABASE_PATH)) {
    LittleFS.remove(temporaryPath);
    return true;
  }

  if (LittleFS.exists(temporaryPath) &&
      isValidJsonFile(temporaryPath.c_str(), true) &&
      LittleFS.rename(temporaryPath, DATABASE_PATH)) {
    LittleFS.remove(backupPath);
    return true;
  }

  LittleFS.remove(temporaryPath);
  LittleFS.remove(backupPath);
  File database = LittleFS.open(DATABASE_PATH, FILE_WRITE);
  if (!database) {
    return false;
  }

  database.print("{}");
  database.close();
  return true;
}

bool readJsonFile(const char *path, JsonDocument &document) {
  StorageGuard guard;
  if (!guard) {
    return false;
  }

  File file = LittleFS.open(path, FILE_READ);
  if (!file) {
    return false;
  }

  DeserializationError error = deserializeJson(document, file);
  file.close();
  return !error;
}

bool writeJsonFile(const char *path, const JsonDocument &document) {
  StorageGuard guard;
  if (!guard) {
    return false;
  }

  const String temporaryPath = String(path) + ".tmp";
  const String backupPath = String(path) + ".bak";
  LittleFS.remove(temporaryPath);

  File file = LittleFS.open(temporaryPath, FILE_WRITE);
  if (!file) {
    return false;
  }

  const size_t written = serializeJsonPretty(document, file);
  file.flush();
  file.close();
  if (written == 0 || !isValidJsonFile(temporaryPath.c_str())) {
    LittleFS.remove(temporaryPath);
    return false;
  }

  LittleFS.remove(backupPath);
  const bool hadOriginal = LittleFS.exists(path);
  if (hadOriginal && !LittleFS.rename(path, backupPath)) {
    LittleFS.remove(temporaryPath);
    return false;
  }

  if (!LittleFS.rename(temporaryPath, path)) {
    if (hadOriginal) {
      LittleFS.rename(backupPath, path);
    }
    LittleFS.remove(temporaryPath);
    return false;
  }

  LittleFS.remove(backupPath);
  return true;
}

bool saveJsonToDatabase(const JsonDocument &json) {
  return writeJsonFile(DATABASE_PATH, json);
}
