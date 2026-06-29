#pragma once

#include <Arduino.h>
#include "datetime/date_and_time.h"
#include "panel/PanelMessages.h"
#include "prayer_schedule.h"
#include "services/panel_config/panel_config_service.h"

void setupPanelInit(
  const Time &time,
  const PrayerSchedule &schedule,
  const PanelMessages &messages,
  const PanelConfig &panelConfig
);
void panelLoop();
void setPanelClock(uint8_t hour, uint8_t minute, uint8_t second);
void setPanelPrayerSchedule(const PrayerSchedule &schedule);
bool queuePanelMessagesUpdate(const PanelMessages &messages);
bool queuePanelConfigUpdate(const PanelConfig &config);
void updatePanelBrightness(const Time &time);
