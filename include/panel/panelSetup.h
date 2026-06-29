#pragma once

#include <Arduino.h>
#include "datetime/date_and_time.h"
#include "panel/PanelMessages.h"
#include "prayer_schedule.h"

void setupPanelInit(
  const Time &time,
  const PrayerSchedule &schedule,
  const PanelMessages &messages
);
void panelLoop();
void setPanelClock(uint8_t hour, uint8_t minute, uint8_t second);
void setPanelPrayerSchedule(const PrayerSchedule &schedule);
bool queuePanelMessagesUpdate(const PanelMessages &messages);
