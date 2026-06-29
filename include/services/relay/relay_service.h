#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include "datetime/date_and_time.h"
#include "prayer_schedule.h"

struct RelayConfig {
  bool enabled;
  uint16_t prePrayerMinutes;
  uint16_t fridayPrePrayerMinutes;
  uint16_t relay12OnDelaySeconds;
  uint16_t relay12OffDelayMinutes;
};

void setupRelayPins();
bool ensureRelayConfig();
bool beginRelayScheduler(const PrayerSchedule &schedule);
void setRelayPrayerSchedule(const PrayerSchedule &schedule);
void relayLoop(const Time &now, const Date &today);

bool loadRelayConfig(RelayConfig &config);
bool updateRelayConfig(
  JsonVariantConst payload,
  RelayConfig &config,
  String &message
);
void requestRelayConfigReload();
