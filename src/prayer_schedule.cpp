#include "prayer_schedule.h"

#include <PrayerTimes.h>
#include "services/prayer_times/prayer_times_service.h"
#include "date_and_time.h"

namespace {
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
    Serial.print("ERROR: Invalid coordinates for ");
    return emptyPrayerSchedule();
  }

  if (pt.isHighLatitude()) {
    Serial.println("  * Lokasi high-latitude terdeteksi");
  }

  applyPrayerTimesConfig(pt, config);


  Date dateNow = dayNow();
  Serial.printf("Prayer times for %d %d, %d\n", dateNow.day, dateNow.month, dateNow.year);

  PrayerTimesResult result = pt.calculate(dateNow.day, dateNow.month, dateNow.year);

  if (!result.valid) {
    Serial.print("ERROR: Calculation failed for ");
    return emptyPrayerSchedule();
  }
  Serial.println();

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

  // Imsak
  Serial.print("  Imsak                 : ");
  Serial.printf("%02u:%02u\n", schedule.imsak.hour, schedule.imsak.minute);
  
  // Fajr
  Serial.print("  Fajr                  : ");
  Serial.printf("%02u:%02u\n", schedule.subuh.hour, schedule.subuh.minute);
  
  // Sunrise
  Serial.print("  Terbit                : ");
  Serial.printf("%02u:%02u\n", schedule.terbit.hour, schedule.terbit.minute);
  
  // Duha
  Serial.print("  Duha                  : ");
  Serial.printf("%02u:%02u\n", schedule.duha.hour, schedule.duha.minute);
  
  // Dhuhr
  Serial.print("  Dhuhr                 : ");
  Serial.printf("%02u:%02u\n", schedule.dzuhur.hour, schedule.dzuhur.minute);

  //ashar
  Serial.print("  Ashar                 : ");
  Serial.printf("%02u:%02u\n", schedule.ashar.hour, schedule.ashar.minute);
  
  // Maghrib
  Serial.print("  Maghrib               : ");
  Serial.printf("%02u:%02u\n", schedule.maghrib.hour, schedule.maghrib.minute);


  Serial.print("  Isha                  : ");
  Serial.printf("%02u:%02u\n", schedule.isya.hour, schedule.isya.minute);

  
  if (pt.isHighLatitude()) {
    Serial.println();
    Serial.println("  * High-latitude adjustments applied");
  }

  return schedule;
}
