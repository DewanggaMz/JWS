#include <Arduino.h>

#include "connection.h"
#include "server_app.h"
#include "storage.h"
#include "prayer_schedule.h"

void setup() {
  Serial.begin(115200);

  if (!initStorage()) {
    Serial.println("LittleFS gagal dimount atau database.json gagal dibuat");
    return;
  }

  connectToWiFi();
  setupServer();
}

void loop() {
  getPrayerTimes();

  delay(4000);
}
