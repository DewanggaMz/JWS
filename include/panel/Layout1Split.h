#pragma once

#include <DMD32.h>
#include "PanelLayout.h"
#include "prayer_schedule.h"

class Layout1Split : public PanelLayout {
  public:
    Layout1Split(
      DMD &display,
      const String &bottomMessage,
      uint8_t repeatTarget = 3,
      uint16_t speedMs = 65
    );

    void begin() override;
    void update(const ClockState &clock) override;
    void render(const ClockState &clock) override;
    bool isFinished() const override;
    void setPrayerSchedule(const PrayerSchedule &schedule);
    void setPrayerDisplayConfig(
      bool showImsak,
      bool showSunrise,
      bool showDhuha
    );
    void setMessage(
      const String &message,
      uint8_t repeatTarget,
      uint16_t speedMs
    );

  private:
    static const uint8_t MAX_PRAYER_MESSAGES = 8;

    enum TopAnimState {
        TOP_ANIM_IN,
        TOP_ANIM_PRE_SCROLL_HOLD,
        TOP_ANIM_SCROLL,
        TOP_ANIM_HOLD,
        TOP_ANIM_OUT
    };

    DMD &dmd;
    TopAnimState topState;
    uint8_t repeatTarget;
    uint8_t repeatCount;
    uint8_t topMessageIndex;
    int topTextX;
    int topTextY;
    int bottomTextX;
    uint32_t lastTopAnimAt;
    uint32_t lastTopScrollAt;
    uint32_t lastTopHoldAt;
    uint32_t lastBottomScrollAt;
    uint16_t bottomScrollMs;
    bool finished;
    String bottomMessage;
    PrayerSchedule currentSchedule;
    bool showImsak;
    bool showSunrise;
    bool showDhuha;
    uint8_t prayerMessageCount;
    char prayerMessages[MAX_PRAYER_MESSAGES][16];

    const char *currentTopMessage() const;
    void setPrayerMessage(uint8_t index, const char *label, const PrayerScheduleTime &time);
    void rebuildPrayerMessages();
    void resetTopMessagePosition();
    bool topTextNeedsScroll();
    void updateTopAnimation();
    void updateBottomScroll();
    void drawRightPanel();
};

