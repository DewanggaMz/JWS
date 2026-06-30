#include <Arduino.h>

#include "connection.h"
#include "datetime/date_and_time.h"
#include "panel/panelSetup.h"
#include "prayer_schedule.h"
#include "server_app.h"
#include "services/database/database_service.h"
#include "services/panel_config/panel_config_service.h"
#include "services/panel_messages/panel_messages_service.h"
#include "services/prayer_times/prayer_times_service.h"
#include "services/relay/relay_service.h"
#include "services/wifi_config/wifi_config_service.h"
#include "storage.h"

namespace {

const uint32_t SECOND_TICK_MS = 1000;
const uint32_t PRAYER_REFRESH_RETRY_MS = 60000;

bool applicationReady = false;
uint32_t lastSecondTickAt = 0;
uint32_t lastPrayerRefreshAttemptAt = 0;
uint32_t loadedPrayerScheduleDateKey = 0;
uint32_t lastPrayerRefreshAttemptDateKey = 0;
bool prayerRefreshPending = false;

uint32_t dateKey(const Date &date)
{
  return
    (static_cast<uint32_t>(date.year) * 10000UL) +
    (static_cast<uint32_t>(date.month) * 100UL) +
    date.day;
}

bool refreshPrayerSchedule(const Date &today)
{
  const uint32_t todayKey = dateKey(today);
  lastPrayerRefreshAttemptAt = millis();
  lastPrayerRefreshAttemptDateKey = todayKey;

  const PrayerSchedule schedule = getPrayerTimes();
  if (!schedule.valid) {
    Serial.println("Gagal menghitung ulang jadwal sholat");
    return false;
  }

  setPanelPrayerSchedule(schedule);
  setRelayPrayerSchedule(schedule);
  loadedPrayerScheduleDateKey = todayKey;
  Serial.println("Jadwal sholat berhasil diperbarui");
  return true;
}

void refreshPrayerScheduleWhenNeeded(const Date &today)
{
  const uint32_t todayKey = dateKey(today);
  const bool newRequest = consumePrayerScheduleRefreshRequest();
  if (newRequest) {
    prayerRefreshPending = true;
  }
  const bool dateChanged =
    todayKey != loadedPrayerScheduleDateKey;
  if (!prayerRefreshPending && !dateChanged) {
    return;
  }

  const bool firstAttemptForDate =
    todayKey != lastPrayerRefreshAttemptDateKey;
  const bool retryDelayElapsed =
    millis() - lastPrayerRefreshAttemptAt >=
    PRAYER_REFRESH_RETRY_MS;
  if (newRequest ||
      firstAttemptForDate ||
      retryDelayElapsed) {
    if (refreshPrayerSchedule(today)) {
      prayerRefreshPending = false;
    }
  }
}

void runSecondTick()
{
  const uint32_t nowMs = millis();
  if (nowMs - lastSecondTickAt < SECOND_TICK_MS) {
    return;
  }
  lastSecondTickAt = nowMs;

  DateTimeState now;
  if (!dateTimeNow(now)) {
    return;
  }

  setPanelClock(
    now.time.hour,
    now.time.minute,
    now.time.second
  );
  updatePanelBrightness(now.time);
  relayLoop(now.time, now.date);
  refreshPrayerScheduleWhenNeeded(now.date);
}

bool preparePersistentConfiguration()
{
  return
    ensurePrayerTimesConfig() &&
    ensurePanelMessages() &&
    ensurePanelConfig() &&
    ensureRelayConfig() &&
    ensureWiFiConfig();
}

bool initializeApplication()
{
  setupRelayPins();

  if (!initTime()) {
    Serial.println("Inisialisasi RTC gagal");
    return false;
  }
  if (!initStorage()) {
    Serial.println("Inisialisasi LittleFS gagal");
    return false;
  }
  if (!initDatabaseService()) {
    Serial.println("Inisialisasi database mutex gagal");
    return false;
  }
  if (!preparePersistentConfiguration()) {
    Serial.println("Konfigurasi persisten gagal disiapkan");
    return false;
  }

  PanelMessages panelMessages;
  PanelConfig panelConfig;
  if (!loadPanelMessages(panelMessages) ||
      !loadPanelConfig(panelConfig)) {
    Serial.println("Konfigurasi panel gagal dibaca");
    return false;
  }

  DateTimeState now;
  if (!dateTimeNow(now)) {
    return false;
  }

  const PrayerSchedule schedule = getPrayerTimes();
  const uint32_t todayKey = dateKey(now.date);
  lastPrayerRefreshAttemptAt = millis();
  lastPrayerRefreshAttemptDateKey = todayKey;
  loadedPrayerScheduleDateKey =
    schedule.valid ? todayKey : 0;

  if (!beginRelayScheduler(schedule)) {
    Serial.println("Scheduler relay gagal disiapkan");
    return false;
  }
  relayLoop(now.time, now.date);

  if (!setupPanelInit(
        now.time,
        schedule,
        panelMessages,
        panelConfig
      )) {
    Serial.println("Inisialisasi panel gagal");
    return false;
  }

  if (connectToWiFi()) {
    setupServer();
  } else {
    Serial.println(
      "WiFi gagal, panel tetap berjalan tanpa web server"
    );
  }

  lastSecondTickAt = millis();
  return true;
}

}

void setup()
{
  Serial.begin(115200);
  applicationReady = initializeApplication();
  if (!applicationReady) {
    setupRelayPins();
    Serial.println(
      "Startup gagal; output relay tetap OFF dan loop dihentikan"
    );
  }
}

void loop()
{
  if (!applicationReady) {
    delay(100);
    return;
  }

  panelLoop();
  runSecondTick();
}
