#include "panel/panelSetup.h"

#include <DMD32.h>
#include <Ticker.h>

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
Layout1Split layout1(dmd, 3);
Layout2FullRunning layout2(dmd);
Layout3SlideText layout3(dmd);
Layout4PrayerSchedule layout4(dmd);

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
}

void setupPanelInit(const Time &time, const PrayerSchedule &schedule) {
  dmd.clearScreen(true);
  dmd.setBrightness(PANEL_BRIGHTNESS);
  layout1.setPrayerSchedule(schedule);

  panelAnimations.addLayout(layout1);
  // panelAnimations.addLayout(layout3);
  panelAnimations.addLayout(layout4);
  // panelAnimations.addLayout(layout2);
  panelAnimations.begin(time.hour, time.minute, time.second);

  startDmdRefresh();
}

void panelLoop () {
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
  layout1.setPrayerSchedule(schedule);
}
