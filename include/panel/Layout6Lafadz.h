#ifndef LAYOUT6_LAFADZ_H
#define LAYOUT6_LAFADZ_H

#include <DMD32.h>

#include "PanelLayout.h"

class Layout6Lafadz : public PanelLayout {
  public:
    explicit Layout6Lafadz(DMD &display, uint32_t holdMs = 7000);

    void begin() override;
    void update(const ClockState &clock) override;
    void render(const ClockState &clock) override;
    bool isFinished() const override;

  private:
    DMD &dmd;
    uint32_t holdMs;
    uint32_t startedAt;
    bool finished;

    void drawBitmap(
      int x,
      int y,
      const char *const rows[],
      uint8_t rowCount
    );
    void drawMuhammad();
    void drawAllah();
};

#endif
