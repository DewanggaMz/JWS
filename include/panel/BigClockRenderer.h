#ifndef BIG_CLOCK_RENDERER_H
#define BIG_CLOCK_RENDERER_H

#include <DMD32.h>
#include "PanelLayout.h"

class BigClockRenderer {
  public:
    static void draw(DMD &dmd, const ClockState &clock);

  private:
    static void drawDigit(DMD &dmd, uint8_t digit, int x, int y);
    static void drawColon(DMD &dmd, int x, int y);
    static void drawPixelBlock(DMD &dmd, int x, int y, int scale);
};

#endif
