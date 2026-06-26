#ifndef PANEL_LAYOUT_H
#define PANEL_LAYOUT_H

#include <Arduino.h>

struct ClockState {
    uint32_t seconds;
    bool colonOn;

    uint8_t hour() const
    {
        return (seconds % 86400UL) / 3600UL;
    }

    uint8_t minute() const
    {
        return ((seconds % 86400UL) % 3600UL) / 60UL;
    }
};

class PanelLayout {
  public:
    virtual void begin() = 0;
    virtual void update(const ClockState &clock) = 0;
    virtual void render(const ClockState &clock) = 0;
    virtual bool isFinished() const = 0;
};

#endif
