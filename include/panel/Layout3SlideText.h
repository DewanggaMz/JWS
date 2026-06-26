#ifndef LAYOUT3_SLIDE_TEXT_H
#define LAYOUT3_SLIDE_TEXT_H

#include <DMD32.h>
#include "PanelLayout.h"

class Layout3SlideText : public PanelLayout {
  public:
    explicit Layout3SlideText(DMD &display);

    void begin() override;
    void update(const ClockState &clock) override;
    void render(const ClockState &clock) override;
    bool isFinished() const override;

  private:
    enum SlideAnimState {
        SLIDE_ANIM_IN,
        SLIDE_ANIM_HOLD,
        SLIDE_ANIM_OUT
    };

    DMD &dmd;
    SlideAnimState slideState;
    uint8_t messageIndex;
    int textX;
    uint32_t lastAnimAt;
    uint32_t lastHoldAt;
    bool finished;
};

#endif
