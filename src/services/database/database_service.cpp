#include "services/database/database_service.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include "config.h"
#include "storage.h"

namespace {
SemaphoreHandle_t databaseMutex = nullptr;
}

DatabaseGuard::DatabaseGuard()
  : locked(
      databaseMutex != nullptr &&
      xSemaphoreTakeRecursive(
        databaseMutex,
        pdMS_TO_TICKS(2000)
      ) == pdTRUE
    )
{
}

DatabaseGuard::~DatabaseGuard()
{
  if (locked) {
    xSemaphoreGiveRecursive(databaseMutex);
  }
}

DatabaseGuard::operator bool() const
{
  return locked;
}

bool initDatabaseService()
{
  if (databaseMutex == nullptr) {
    databaseMutex = xSemaphoreCreateRecursiveMutex();
  }
  return databaseMutex != nullptr;
}

bool loadDatabase(JsonDocument &database) {
  if (!readJsonFile(DATABASE_PATH, database) || !database.is<JsonObject>()) {
    database.clear();
    database.to<JsonObject>();
    return false;
  }

  return true;
}

bool saveDatabase(const JsonDocument &database) {
  return writeJsonFile(DATABASE_PATH, database);
}
