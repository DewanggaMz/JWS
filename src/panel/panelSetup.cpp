#include "panel/panelSetup.h"

#include <DMD32.h>
#include <Ticker.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include "panel/DisplayConfig.h"
#include "panel/Layout1Split.h"
#include "panel/Layout2FullRunning.h"
#include "panel/Layout3SlideText.h"
#include "panel/Layout4PrayerSchedule.h"
#include "panel/Layout5PrayerCountdown.h"
#include "panel/PanelAnimations.h"
#include "utils/StaticObject.h"

static const uint32_t DMD_REFRESH_MS = 1;
static const uint32_t FRAME_MS = 30;

DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);
Ticker dmdTicker;
PanelAnimations panelAnimations(dmd);
StaticObject<Layout1Split> layout1Storage;
StaticObject<Layout2FullRunning> layout2Storage;
StaticObject<Layout3SlideText> layout3Storage;
StaticObject<Layout4PrayerSchedule> layout4Storage;
StaticObject<Layout5PrayerCountdown> layout5Storage;
Layout1Split *layout1 = nullptr;
Layout2FullRunning *layout2 = nullptr;
Layout3SlideText *layout3 = nullptr;
Layout4PrayerSchedule *layout4 = nullptr;
Layout5PrayerCountdown *layout5 = nullptr;
SemaphoreHandle_t panelMessagesMutex = nullptr;
PanelMessages pendingPanelMessages;
bool panelMessagesUpdatePending = false;
PanelConfig activePanelConfig{200, true, 1320, 240, 50};
PanelConfig pendingPanelConfig{200, true, 1320, 240, 50};
bool panelConfigUpdatePending = false;

uint32_t lastFrameAt = 0;

namespace{
  void triggerScan()
  {
      dmd.scanDisplayBySPI();
  }
  
  void startDmdRefresh()
  {
      dmdTicker.attach_ms(DMD_REFRESH_MS, triggerScan);
  }

  void applyPanelMessages(const PanelMessages &messages)
  {
    if (layout1 != nullptr) {
      layout1->setMessage(
        messages.layout1Bottom,
        messages.layout1RepeatCount,
        messages.layout1SpeedMs
      );
      layout1->setPrayerDisplayConfig(
        messages.layout1ShowImsak,
        messages.layout1ShowSunrise,
        messages.layout1ShowDhuha
      );
    }

    if (layout2 != nullptr) {
      layout2->setMessage(
        messages.layout2Running,
        messages.layout2SpeedMs
      );
    }

    if (layout3 != nullptr) {
      layout3->setMessages(
        messages.layout3Slides,
        messages.layout3SlideCount
      );
    }

    if (layout4 != nullptr) {
      layout4->setConfiguration(
        messages.layout4Running,
        messages.layout4ShowPasaran,
        messages.layout4ShowHijriDate,
        messages.layout4RepeatCount,
        messages.layout4HijriCorrection,
        messages.layout4SpeedMs
      );
    }

    if (layout5 != nullptr) {
      layout5->setRunningSpeed(messages.layout5SpeedMs);
    }
  }

  void applyPendingPanelMessages()
  {
    if (panelMessagesMutex == nullptr ||
        xSemaphoreTake(panelMessagesMutex, 0) != pdTRUE) {
      return;
    }

    if (panelMessagesUpdatePending) {
      applyPanelMessages(pendingPanelMessages);
      panelMessagesUpdatePending = false;
    }
    if (panelConfigUpdatePending) {
      activePanelConfig = pendingPanelConfig;
      panelConfigUpdatePending = false;
    }

    xSemaphoreGive(panelMessagesMutex);
  }
}

bool setupPanelInit(
  const Time &time,
  const PrayerSchedule &schedule,
  const PanelMessages &messages,
  const PanelConfig &panelConfig
) {
  if (panelMessagesMutex == nullptr) {
    panelMessagesMutex = xSemaphoreCreateMutex();
  }
  if (panelMessagesMutex == nullptr) {
    return false;
  }

  dmd.clearScreen(true);
  activePanelConfig = panelConfig;
  updatePanelBrightness(time);

  layout1 = layout1Storage.create(
    dmd,
    messages.layout1Bottom,
    messages.layout1RepeatCount,
    messages.layout1SpeedMs
  );
  layout2 = layout2Storage.create(
    dmd,
    messages.layout2Running,
    messages.layout2SpeedMs
  );
  layout3 = layout3Storage.create(
    dmd,
    messages.layout3Slides,
    messages.layout3SlideCount
  );
  layout4 = layout4Storage.create(
    dmd,
    messages.layout4Running,
    messages.layout4ShowPasaran,
    messages.layout4ShowHijriDate,
    messages.layout4RepeatCount,
    messages.layout4HijriCorrection,
    messages.layout4SpeedMs
  );
  layout5 = layout5Storage.create(
    dmd,
    schedule,
    messages.layout5SpeedMs
  );

  layout1->setPrayerDisplayConfig(
    messages.layout1ShowImsak,
    messages.layout1ShowSunrise,
    messages.layout1ShowDhuha
  );
  layout1->setPrayerSchedule(schedule);

  panelAnimations.addLayout(*layout1);
  panelAnimations.addLayout(*layout3);
  bool layoutsAdded = panelAnimations.addLayout(*layout4);
  layoutsAdded &= panelAnimations.addLayout(*layout5);
  panelAnimations.addLayout(*layout2);
  if (!layoutsAdded) {
    return false;
  }
  panelAnimations.begin(time.hour, time.minute, time.second);

  startDmdRefresh();
  return true;
}

void panelLoop () {
  applyPendingPanelMessages();
  panelAnimations.update();
  
  // if (millis() - lastFrameAt >= ) {
  //     lastFrameAt = now;
  //     panelAnimations.render();
  // }
  const uint32_t now = millis();
  if (now - lastFrameAt >= FRAME_MS) {
      lastFrameAt = now;
      panelAnimations.render();
  }
}

void setPanelClock(uint8_t hour, uint8_t minute, uint8_t second) {
  panelAnimations.setClock(hour, minute, second);
}

void setPanelPrayerSchedule(const PrayerSchedule &schedule) {
  if (layout1 != nullptr) {
    layout1->setPrayerSchedule(schedule);
  }
  if (layout5 != nullptr) {
    layout5->setPrayerSchedule(schedule);
  }
}

bool queuePanelMessagesUpdate(const PanelMessages &messages) {
  if (panelMessagesMutex == nullptr ||
      xSemaphoreTake(panelMessagesMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
    return false;
  }

  pendingPanelMessages = messages;
  panelMessagesUpdatePending = true;
  xSemaphoreGive(panelMessagesMutex);
  return true;
}

bool queuePanelConfigUpdate(const PanelConfig &config) {
  if (panelMessagesMutex == nullptr ||
      xSemaphoreTake(panelMessagesMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
    return false;
  }

  pendingPanelConfig = config;
  panelConfigUpdatePending = true;
  xSemaphoreGive(panelMessagesMutex);
  return true;
}

void updatePanelBrightness(const Time &time) {
  const uint16_t currentMinutes =
    static_cast<uint16_t>(time.hour * 60U + time.minute);
  bool withinDimSchedule = false;

  if (activePanelConfig.dimEnabled) {
    if (activePanelConfig.dimStartMinutes <
        activePanelConfig.dimEndMinutes) {
      withinDimSchedule =
        currentMinutes >= activePanelConfig.dimStartMinutes &&
        currentMinutes < activePanelConfig.dimEndMinutes;
    } else {
      withinDimSchedule =
        currentMinutes >= activePanelConfig.dimStartMinutes ||
        currentMinutes < activePanelConfig.dimEndMinutes;
    }
  }

  const uint8_t targetBrightness = withinDimSchedule
                                     ? activePanelConfig.dimBrightness
                                     : activePanelConfig.brightness;
  if (dmd.getBrightness() != targetBrightness) {
    dmd.setBrightness(targetBrightness);
  }
}
