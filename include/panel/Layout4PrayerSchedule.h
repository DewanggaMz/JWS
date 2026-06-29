#ifndef LAYOUT4_PRAYER_SCHEDULE_H
#define LAYOUT4_PRAYER_SCHEDULE_H

#include <DMD32.h>
#include "PanelLayout.h"

class Layout4PrayerSchedule : public PanelLayout {
  public:
    Layout4PrayerSchedule(
      DMD &display,
      const String &runningMessage,
      bool showPasaran,
      bool showHijriDate,
      uint8_t repeatTarget,
      int hijriCorrection
    );

    void begin() override;
    void update(const ClockState &clock) override;
    void render(const ClockState &clock) override;
    bool isFinished() const override;
    void setConfiguration(
      const String &runningMessage,
      bool showPasaran,
      bool showHijriDate,
      uint8_t repeatTarget,
      int hijriCorrection
    );

  private:
    enum TopMode {
      TOP_TIME,
      TOP_DAY,
      TOP_DATE
    };

    enum TopAnimState {
      TOP_ANIM_IN,
      TOP_ANIM_HOLD,
      TOP_ANIM_OUT
    };

    enum BottomAnimState {
      BOTTOM_HIJRI_RUNNING,
      BOTTOM_RUNNING
    };

    DMD &dmd;
    TopMode topMode;
    TopAnimState topAnimState;
    BottomAnimState bottomAnimState;
    int topTextY;
    int bottomTextX;
    uint8_t lightX;
    uint32_t lastTopAnimAt;
    uint32_t topHoldStartedAt;
    uint32_t lastBottomAnimAt;
    uint32_t lastLightAt;
    bool finished;
    bool showPasaran;
    bool showHijriDate;
    uint8_t repeatTarget;
    uint8_t repeatCount;
    int hijriCorrection;
    char topText[24];
    char hijriText[40];
    String runningMessage;

    void nextTopMode();
    void updateTopText(const ClockState &clock);
    void updateHijriText();
    void resetAnimationCycle(uint32_t now);
    void updateTopAnimation();
    void updateBottomAnimation();
    void updateLightAnimation();
    void drawTop();
    void drawBottom();
    void drawCenteredTopText(const char *text);
    void drawBoldCenteredTopText(const char *text);
    void drawMosqueLight();
};

#endif
