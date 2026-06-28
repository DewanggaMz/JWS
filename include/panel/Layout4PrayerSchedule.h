#ifndef LAYOUT4_PRAYER_SCHEDULE_H
#define LAYOUT4_PRAYER_SCHEDULE_H

#include <DMD32.h>
#include "PanelLayout.h"

class Layout4PrayerSchedule : public PanelLayout {
  public:
    Layout4PrayerSchedule(DMD &display, const String &bottomMessage);

    void begin() override;
    void update(const ClockState &clock) override;
    void render(const ClockState &clock) override;
    bool isFinished() const override;

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

    DMD &dmd;
    TopMode topMode;
    TopAnimState topAnimState;
    int topTextY;
    int bottomTextX;
    uint8_t lightX;
    uint32_t startedAt;
    uint32_t lastTopAnimAt;
    uint32_t topHoldStartedAt;
    uint32_t lastBottomScrollAt;
    uint32_t lastLightAt;
    bool finished;
    bool bottomHasEntered;
    char topText[24];
    String bottomMessage;

    void nextTopMode();
    void updateTopText(const ClockState &clock);
    void updateTopAnimation();
    void updateBottomScroll();
    void updateLightAnimation();
    void drawTop();
    void drawBottom();
    void drawCenteredTopText(const char *text);
    void drawBoldCenteredTopText(const char *text);
    void drawMosqueLight();
};

#endif
