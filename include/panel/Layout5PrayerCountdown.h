#pragma once

#include <DMD32.h>

#include "PanelLayout.h"
#include "prayer_schedule.h"

class Layout5PrayerCountdown : public PanelLayout {
  public:
    Layout5PrayerCountdown(
      DMD &display,
      const PrayerSchedule &schedule,
      uint16_t speedMs = 65
    );

    void begin() override;
    void update(const ClockState &clock) override;
    void render(const ClockState &clock) override;
    bool isFinished() const override;
    void setPrayerSchedule(const PrayerSchedule &schedule);
    void setRunningSpeed(uint16_t speedMs);

  private:
    enum AnimationState {
      COUNTDOWN_SLIDE_IN,
      COUNTDOWN_HOLD,
      RUNNING_MESSAGE
    };

    DMD &dmd;
    PrayerSchedule schedule;
    AnimationState state;
    bool targetSelected;
    bool finished;
    uint8_t messageRepeatCount;
    uint32_t targetSecond;
    int countdownX;
    int runningX;
    uint32_t lastAnimationAt;
    uint32_t holdStartedAt;
    uint32_t lastScrollAt;
    uint16_t runningScrollMs;
    String prayerName;
    String runningMessage;
    char countdownText[12];

    bool selectNextPrayer(const ClockState &clock);
    void updateCountdown(const ClockState &clock);
    void updateSlideAnimation();
    void updateRunningMessage();
    void drawCountdown();
    void drawRunningMessage();
};
