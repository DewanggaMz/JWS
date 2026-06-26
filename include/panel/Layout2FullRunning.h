#ifndef LAYOUT2_FULL_RUNNING_H
#define LAYOUT2_FULL_RUNNING_H

#include <DMD32.h>
#include "PanelLayout.h"

class Layout2FullRunning : public PanelLayout {
  public:
    explicit Layout2FullRunning(DMD &display);

    void begin() override;
    void update(const ClockState &clock) override;
    void render(const ClockState &clock) override;
    bool isFinished() const override;

  private:
    DMD &dmd;
    int textX;
    uint32_t lastScrollAt;
    bool finished;
};

#endif
