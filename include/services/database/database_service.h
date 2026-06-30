#pragma once

#include <ArduinoJson.h>

class DatabaseGuard {
  public:
    DatabaseGuard();
    ~DatabaseGuard();

    DatabaseGuard(const DatabaseGuard &) = delete;
    DatabaseGuard &operator=(const DatabaseGuard &) = delete;

    explicit operator bool() const;

  private:
    bool locked;
};

bool initDatabaseService();
bool loadDatabase(JsonDocument &database);
bool saveDatabase(const JsonDocument &database);
