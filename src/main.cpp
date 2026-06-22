#include <Arduino.h>

#include "connection.h"
#include "server_app.h"
#include "storage.h"
#include "prayer_schedule.h"
#include "services/prayer_times/prayer_times_service.h"

void setup() {
  Serial.begin(115200);

  if (!initStorage()) {
    Serial.println("LittleFS gagal dimount atau database.json gagal dibuat");
    return;
  }

  if (!ensurePrayerTimesConfig()) {
    Serial.println("Konfigurasi prayerTimes gagal disiapkan");
    return;
  }

  connectToWiFi();
  setupServer();
}

void loop() {
  getPrayerTimes();

  delay(5000);
}
