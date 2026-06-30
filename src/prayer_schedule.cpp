#include "prayer_schedule.h"

#include <PrayerTimes.h>
#include <atomic>
#include "services/prayer_times/prayer_times_service.h"
#include "datetime/date_and_time.h"

namespace {
std::atomic<bool> prayerScheduleRefreshRequested(false);

PrayerSchedule emptyPrayerSchedule()
{
  PrayerSchedule schedule{};
  schedule.valid = false;
  return schedule;
}

PrayerScheduleTime toPrayerScheduleTime(PrayerTimes &pt, float minutes)
{
  int hour = 0;
  int minute = 0;
  pt.minutesToTime(minutes, hour, minute);

  PrayerScheduleTime time{};
  time.hour = hour;
  time.minute = minute;
  time.valid = true;
  return time;
}
}

void requestPrayerScheduleRefresh()
{
  prayerScheduleRefreshRequested.store(true, std::memory_order_release);
}

bool consumePrayerScheduleRefreshRequest()
{
  return prayerScheduleRefreshRequested.exchange(
    false,
    std::memory_order_acq_rel
  );
}

PrayerSchedule getPrayerTimes() {
  JsonDocument configDocument;
  if (!loadPrayerTimesConfig(configDocument)) {
    Serial.println("ERROR: Gagal membaca konfigurasi prayerTimes");
    return emptyPrayerSchedule();
  }

  String message;
  PrayerTimesConfig config;
  if (!parsePrayerTimesConfig(configDocument.as<JsonVariantConst>(), config, message)) {
    Serial.print("ERROR: ");
    Serial.println(message);
    return emptyPrayerSchedule();
  }

  PrayerTimes pt(config.latitude, config.longitude, config.timezoneOffsetMinutes);

  if (!pt.isInitialized()) {
    Serial.println("ERROR: Koordinat jadwal sholat tidak valid");
    return emptyPrayerSchedule();
  }

  applyPrayerTimesConfig(pt, config);


  Date dateNow = dayNow();

  PrayerTimesResult result = pt.calculate(dateNow.day, dateNow.month, dateNow.year);

  if (!result.valid) {
    Serial.println("ERROR: Kalkulasi jadwal sholat gagal");
    return emptyPrayerSchedule();
  }
  PrayerSchedule schedule{};
  schedule.imsak = toPrayerScheduleTime(pt, result.imsak);
  schedule.subuh = toPrayerScheduleTime(pt, result.fajr);
  schedule.terbit = toPrayerScheduleTime(pt, result.sunrise);
  schedule.duha = toPrayerScheduleTime(pt, result.duha);
  schedule.dzuhur = toPrayerScheduleTime(pt, result.dhuhr);
  schedule.ashar = toPrayerScheduleTime(pt, result.asr);
  schedule.maghrib = toPrayerScheduleTime(pt, result.maghrib);
  schedule.isya = toPrayerScheduleTime(pt, result.isha);
  schedule.valid = true;

  return schedule;
}
