#include <Arduino.h>

#include "connection.h"
#include "server_app.h"
#include "storage.h"
#include "prayer_schedule.h"
#include "services/prayer_times/prayer_times_service.h"
#include "date_and_time.h"
#include "panel/panelSetup.h"
unsigned long previousMillis;

void cetakWaktu() {
  // millis setiap 1 detik

  
  if(millis() - previousMillis >= 1000) {
    Time now = timeNow();
    Serial.printf("Current time: %02d:%02d:%02d\n", now.hour, now.minute, now.second);
    Serial.println("=====================================");
    
    Date today = dayNow();
    Serial.printf("Today: %s %02d %02d %04d\n", today.dayName, today.day, today.month, today.year);
    Serial.println("=====================================");
    
    if (now.hour == 0 && now.minute == 0 && now.second == 0) {
      getPrayerTimes();
    }
    Serial.println("=====================================");

    previousMillis = millis();
  }
}

void updateDisplay(void *parameter) {
  while (true) {
    panelLoop();
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);

  xTaskCreatePinnedToCore(
    updateDisplay,      // fungsi task
    "updateDisplay",  // nama task
    4096,         // stack size
    NULL,         // parameter
    1,            // priority
    NULL,         // task handle
    0             // core ESP32
  );

  if (!initStorage()) {
    Serial.println("LittleFS gagal dimount atau database.json gagal dibuat");
    return;
  }

  initTime();

  if (!ensurePrayerTimesConfig()) {
    Serial.println("Konfigurasi prayerTimes gagal disiapkan");
    return;
  }

  connectToWiFi();
  setupServer();
  setupPanelInit();
}

void loop() {
  
  cetakWaktu();
}



