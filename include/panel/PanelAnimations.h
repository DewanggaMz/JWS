#ifndef PANEL_ANIMATIONS_H
#define PANEL_ANIMATIONS_H

#include <Arduino.h>
#include <DMD32.h>
#include "PanelLayout.h"

class PanelAnimations {
  public:
    explicit PanelAnimations(DMD &display);

    bool addLayout(PanelLayout &layout);
    void begin(uint8_t hour, uint8_t minute, uint8_t second);
    void update();
    void render();

  private:
    static const uint8_t MAX_LAYOUTS = 8;
    static const uint32_t CLOCK_MS = 1000;

    DMD &dmd;
    PanelLayout *layouts[MAX_LAYOUTS];
    uint8_t layoutCount;
    uint8_t activeLayoutIndex;
    ClockState clock;
    uint32_t lastClockAt;

    void updateClock();
    void startLayout(uint8_t index);
    void nextLayout();
};

#endif
