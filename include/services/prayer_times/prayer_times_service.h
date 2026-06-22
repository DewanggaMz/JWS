#pragma once

#include <ArduinoJson.h>
#include <PrayerTimes.h>

struct PrayerTimesConfig {
  int day;
  int month;
  int year;
  float latitude;
  float longitude;
  int timezoneOffsetMinutes;
  String calculationMethod;
  float fajrAngle;
  float ishaAngle;
  bool ishaIsInterval;
  int ishaMinutes;
  String asrMethod;
  String highLatitudeRule;
  int imsakOffsetMinutes;
  float duhaAngle;
  int adjustmentFajr;
  int adjustmentSunrise;
  int adjustmentDhuhr;
  int adjustmentAsr;
  int adjustmentMaghrib;
  int adjustmentIsha;
};

bool ensurePrayerTimesConfig();
bool loadPrayerTimesConfig(JsonDocument &config);
bool updatePrayerTimesConfig(JsonVariantConst payload, String &message);
bool parsePrayerTimesConfig(JsonVariantConst source, PrayerTimesConfig &config, String &message);
void applyPrayerTimesConfig(PrayerTimes &prayerTimes, const PrayerTimesConfig &config);
void setDefaultPrayerTimesConfig(JsonObject target);
