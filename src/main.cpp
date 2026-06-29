#include <Arduino.h>

#include "connection.h"
#include "server_app.h"
#include "storage.h"
#include "prayer_schedule.h"
#include "services/prayer_times/prayer_times_service.h"
#include "datetime/date_and_time.h"
#include "panel/panelSetup.h"
#include "datetime/hijriah.h"
#include "datetime/pasaran.h"
#include "services/panel_messages/panel_messages_service.h"
#include "services/panel_config/panel_config_service.h"
#include "services/relay/relay_service.h"

unsigned long previousMillis;
unsigned long lastPrayerRefreshAttemptAt;
uint32_t loadedPrayerScheduleDateKey;
uint32_t lastPrayerRefreshAttemptDateKey;

namespace {

uint32_t dateKey(const Date &date) {
  return
    (static_cast<uint32_t>(date.year) * 10000UL) +
    (static_cast<uint32_t>(date.month) * 100UL) +
    date.day;
}

bool refreshPanelPrayerSchedule(const Date &today) {
  const uint32_t todayKey = dateKey(today);
  lastPrayerRefreshAttemptAt = millis();
  lastPrayerRefreshAttemptDateKey = todayKey;

  PrayerSchedule schedule = getPrayerTimes();
  if (!schedule.valid) {
    Serial.println("Gagal menghitung ulang jadwal sholat");
    return false;
  }

  setPanelPrayerSchedule(schedule);
  setRelayPrayerSchedule(schedule);
  loadedPrayerScheduleDateKey = todayKey;
  Serial.println("Jadwal sholat Layout 1 berhasil diperbarui");
  return true;
}

void processPrayerScheduleRefreshRequest() {
  if (!consumePrayerScheduleRefreshRequest()) {
    return;
  }

  refreshPanelPrayerSchedule(dayNow());
}

void refreshPrayerScheduleWhenDateChanges(const Date &today) {
  const uint32_t todayKey = dateKey(today);
  if (todayKey == loadedPrayerScheduleDateKey) {
    return;
  }

  const bool firstAttemptForDate =
    todayKey != lastPrayerRefreshAttemptDateKey;
  const bool retryDelayElapsed =
    millis() - lastPrayerRefreshAttemptAt >= 60000UL;
  if (firstAttemptForDate || retryDelayElapsed) {
    refreshPanelPrayerSchedule(today);
  }
}

}

void cetakWaktu() {
  if(millis() - previousMillis >= 1000) {
    Time now = timeNow();
    Date today = dayNow();
    setPanelClock(now.hour, now.minute, now.second);
    updatePanelBrightness(now);
    relayLoop(now, today);

    Serial.printf("Current time: %02d:%02d:%02d\n", now.hour, now.minute, now.second);
    Serial.println("=====================================");
    
    Serial.printf("Today: %s %02d %02d %04d\n", today.dayName, today.day, today.month, today.year);
    Serial.println("=====================================");
    
    refreshPrayerScheduleWhenDateChanges(today);
    Serial.println("=====================================");

    HijriDate hijri = HijriModule::getHijriDate(
      today.year,
      today.month,
      today.day,
      1
    );

    const char* hijriDayName = getHijriMonthName(hijri.month);

    String hijriDate = String(hijri.day);
    String hijriMonth = String(hijri.month);
    String hijriYear = String(hijri.year);

    Serial.printf("Hijri: %s %s %s \n",hijriDate.c_str() , hijriDayName , hijriYear.c_str());
    Serial.println("=====================================");

    String pasaran = getPasaran(today.day, today.month, today.year);
    Serial.printf("Pasaran: %s %s\n", today.dayName, pasaran.c_str());
    Serial.println("=====================================");

    previousMillis = millis();
  }
}

void setup() {
  Serial.begin(115200);
  setupRelayPins();
  
  initTime();
  if (!initStorage()) {
    Serial.println("LittleFS gagal dimount atau database.json gagal dibuat");
    return;
  }

  if (!ensurePrayerTimesConfig()) {
    Serial.println("Konfigurasi prayerTimes gagal disiapkan");
    return;
  }

  if (!ensurePanelMessages()) {
    Serial.println("Konfigurasi panelMessages gagal disiapkan");
    return;
  }

  if (!ensurePanelConfig()) {
    Serial.println("Konfigurasi panel gagal disiapkan");
    return;
  }

  if (!ensureRelayConfig()) {
    Serial.println("Konfigurasi relay gagal disiapkan");
    return;
  }

  PanelMessages panelMessages;
  if (!loadPanelMessages(panelMessages)) {
    Serial.println("Konfigurasi panelMessages gagal dibaca");
    return;
  }
  PanelConfig panelConfig;
  if (!loadPanelConfig(panelConfig)) {
    Serial.println("Konfigurasi panel gagal dibaca");
    return;
  }


  Time now = timeNow();
  PrayerSchedule schedule = getPrayerTimes();
  Date today = dayNow();
  const uint32_t todayKey = dateKey(today);
  lastPrayerRefreshAttemptAt = millis();
  lastPrayerRefreshAttemptDateKey = todayKey;
  loadedPrayerScheduleDateKey = schedule.valid ? todayKey : 0;

  if (!beginRelayScheduler(schedule)) {
    Serial.println("Scheduler relay gagal disiapkan");
    return;
  }
  relayLoop(now, today);

  connectToWiFi();
  setupPanelInit(
    now,
    schedule,
    panelMessages,
    panelConfig
  );
  setupServer();
}

void loop() {
  processPrayerScheduleRefreshRequest();
  panelLoop();
  cetakWaktu();
}
