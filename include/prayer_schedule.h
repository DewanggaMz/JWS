#pragma once

#include <PrayerTimes.h>

struct PrayerScheduleTime {
  uint8_t hour;
  uint8_t minute;
  bool valid;
};

struct PrayerSchedule {
  PrayerScheduleTime imsak;
  PrayerScheduleTime subuh;
  PrayerScheduleTime terbit;
  PrayerScheduleTime duha;
  PrayerScheduleTime dzuhur;
  PrayerScheduleTime ashar;
  PrayerScheduleTime maghrib;
  PrayerScheduleTime isya;
  bool valid;
};

PrayerSchedule getPrayerTimes();
