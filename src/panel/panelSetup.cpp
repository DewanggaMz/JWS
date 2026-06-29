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
#include "panel/PanelAnimations.h"

static const uint32_t DMD_REFRESH_MS = 1;
static const uint32_t FRAME_MS = 30;
static const uint8_t PANEL_BRIGHTNESS = 200;

DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);
Ticker dmdTicker;
PanelAnimations panelAnimations(dmd);
Layout1Split *layout1 = nullptr;
Layout2FullRunning *layout2 = nullptr;
Layout3SlideText *layout3 = nullptr;
Layout4PrayerSchedule *layout4 = nullptr;
SemaphoreHandle_t panelMessagesMutex = nullptr;
PanelMessages pendingPanelMessages;
bool panelMessagesUpdatePending = false;

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
        messages.layout1RepeatCount
      );
      layout1->setPrayerDisplayConfig(
        messages.layout1ShowImsak,
        messages.layout1ShowSunrise,
        messages.layout1ShowDhuha
      );
    }

    if (layout2 != nullptr) {
      layout2->setMessage(messages.layout2Running);
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
        messages.layout4HijriCorrection
      );
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

    xSemaphoreGive(panelMessagesMutex);
  }
}

void setupPanelInit(
  const Time &time,
  const PrayerSchedule &schedule,
  const PanelMessages &messages
) {
  if (panelMessagesMutex == nullptr) {
    panelMessagesMutex = xSemaphoreCreateMutex();
  }

  dmd.clearScreen(true);
  dmd.setBrightness(PANEL_BRIGHTNESS);

  layout1 = new Layout1Split(
    dmd,
    messages.layout1Bottom,
    messages.layout1RepeatCount
  );
  layout2 = new Layout2FullRunning(dmd, messages.layout2Running);
  layout3 = new Layout3SlideText(
    dmd,
    messages.layout3Slides,
    messages.layout3SlideCount
  );
  layout4 = new Layout4PrayerSchedule(
    dmd,
    messages.layout4Running,
    messages.layout4ShowPasaran,
    messages.layout4ShowHijriDate,
    messages.layout4RepeatCount,
    messages.layout4HijriCorrection
  );

  layout1->setPrayerDisplayConfig(
    messages.layout1ShowImsak,
    messages.layout1ShowSunrise,
    messages.layout1ShowDhuha
  );
  layout1->setPrayerSchedule(schedule);

  panelAnimations.addLayout(*layout1);
  // panelAnimations.addLayout(*layout3);
  // panelAnimations.addLayout(*layout4);
  // panelAnimations.addLayout(*layout2);
  panelAnimations.begin(time.hour, time.minute, time.second);

  startDmdRefresh();
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
