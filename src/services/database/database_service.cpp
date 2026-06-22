#include "services/database/database_service.h"

#include "config.h"
#include "storage.h"

bool loadDatabase(JsonDocument &database) {
  if (!readJsonFile(DATABASE_PATH, database) || !database.is<JsonObject>()) {
    database.clear();
    database.to<JsonObject>();
  }

  return true;
}

bool saveDatabase(const JsonDocument &database) {
  return writeJsonFile(DATABASE_PATH, database);
}
