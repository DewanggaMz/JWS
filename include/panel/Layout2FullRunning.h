#ifndef LAYOUT2_FULL_RUNNING_H
#define LAYOUT2_FULL_RUNNING_H

#include <DMD32.h>
#include "PanelLayout.h"

class Layout2FullRunning : public PanelLayout {
  public:
    Layout2FullRunning(DMD &display, const String &message);

    void begin() override;
    void update(const ClockState &clock) override;
    void render(const ClockState &clock) override;
    bool isFinished() const override;
    void setMessage(const String &message);

  private:
    DMD &dmd;
    int textX;
    uint32_t lastScrollAt;
    bool finished;
    String message;
};

#endif
