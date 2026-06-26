#ifndef LAYOUT4_PRAYER_SCHEDULE_H
#define LAYOUT4_PRAYER_SCHEDULE_H

#include <DMD32.h>
#include "PanelLayout.h"

class Layout4PrayerSchedule : public PanelLayout {
  public:
    explicit Layout4PrayerSchedule(DMD &display);

    void begin() override;
    void update(const ClockState &clock) override;
    void render(const ClockState &clock) override;
    bool isFinished() const override;

  private:
    DMD &dmd;
    uint32_t startedAt;
    bool finished;

    void drawSchedule();
    void drawCellText(int x, int y, const char *text);
    void drawSmallChar(int x, int y, char c);
    const uint8_t *glyphFor(char c);
};

#endif
