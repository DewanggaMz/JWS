#include "panel/PanelAnimations.h"

PanelAnimations::PanelAnimations(DMD &display)
    : dmd(display),
      layoutCount(0),
      activeLayoutIndex(0),
      clock{0, false},
      lastClockAt(0)
{
    for (uint8_t i = 0; i < MAX_LAYOUTS; i++) {
        layouts[i] = nullptr;
    }
}

bool PanelAnimations::addLayout(PanelLayout &layout)
{
    if (layoutCount >= MAX_LAYOUTS) {
        return false;
    }

    layouts[layoutCount] = &layout;
    layoutCount++;
    return true;
}

void PanelAnimations::begin(uint8_t hour, uint8_t minute, uint8_t second)
{
    clock.seconds = (hour * 3600UL) + (minute * 60UL) + second;
    clock.colonOn = false;
    lastClockAt = millis();

    if (layoutCount > 0) {
        startLayout(0);
    }
}

void PanelAnimations::update()
{
    updateClock();
    if (layoutCount == 0) {
        return;
    }

    PanelLayout *activeLayout = layouts[activeLayoutIndex];
    activeLayout->update(clock);
    if (activeLayout->isFinished()) {
        nextLayout();
    }
}

void PanelAnimations::render()
{
    if (layoutCount == 0) {
        return;
    }

    layouts[activeLayoutIndex]->render(clock);
}

void PanelAnimations::updateClock()
{
    const uint32_t now = millis();
    if (now - lastClockAt < CLOCK_MS) {
        return;
    }

    lastClockAt += CLOCK_MS;
    clock.seconds++;
    clock.colonOn = !clock.colonOn;
}

void PanelAnimations::startLayout(uint8_t index)
{
    activeLayoutIndex = index;
    dmd.clearScreen(true);
    layouts[activeLayoutIndex]->begin();
}

void PanelAnimations::nextLayout()
{
    uint8_t nextIndex = activeLayoutIndex + 1;
    if (nextIndex >= layoutCount) {
        nextIndex = 0;
    }

    startLayout(nextIndex);
}
