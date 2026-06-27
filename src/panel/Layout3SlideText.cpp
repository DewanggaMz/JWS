#include "panel/Layout3SlideText.h"

#include "panel/DisplayConfig.h"
#include "panel/TextRenderer.h"
#include "fonts/Arial14.h"

namespace {
const uint32_t ANIM_MS = 10;
const uint32_t HOLD_MS = 1600;
const int CHAR_SPACING = 2;

const char *messages[] = {
    "10 MENIT",
    "LAGI",
    "MEMASUKI",
    "WAKTU",
    "SHOLAT",
    "MAGHRIB"
};

const uint8_t messageCount = sizeof(messages) / sizeof(messages[0]);
}

Layout3SlideText::Layout3SlideText(DMD &display)
    : dmd(display),
      slideState(SLIDE_ANIM_IN),
      messageIndex(0),
      textX(-SCREEN_WIDTH),
      lastAnimAt(0),
      lastHoldAt(0),
      finished(false)
{
}

void Layout3SlideText::begin()
{
    dmd.selectFont(Arial_14);
    dmd.clearScreen(true);
    slideState = SLIDE_ANIM_IN;
    messageIndex = 0;
    textX = -TextRenderer::textWidth(dmd, messages[messageIndex], CHAR_SPACING);
    finished = false;

    const uint32_t now = millis();
    lastAnimAt = now;
    lastHoldAt = now;
}

void Layout3SlideText::update(const ClockState &clock)
{
    (void)clock;
    dmd.selectFont(Arial_14);

    const char *text = messages[messageIndex];
    const int targetX = max(0, (SCREEN_WIDTH - TextRenderer::textWidth(dmd, text, CHAR_SPACING)) / 2);
    const uint32_t now = millis();

    if (slideState == SLIDE_ANIM_HOLD) {
        if (now - lastHoldAt >= HOLD_MS) {
            slideState = SLIDE_ANIM_OUT;
            lastAnimAt = now;
        }
        return;
    }

    if (now - lastAnimAt < ANIM_MS) {
        return;
    }
    lastAnimAt = now;

    if (slideState == SLIDE_ANIM_IN) {
        textX += 2;
        if (textX >= targetX) {
            textX = targetX;
            slideState = SLIDE_ANIM_HOLD;
            lastHoldAt = now;
        }
    } else {
        textX += 2;
        if (textX > SCREEN_WIDTH) {
            messageIndex++;
            if (messageIndex >= messageCount) {
                finished = true;
                return;
            }

            textX = -TextRenderer::textWidth(dmd, messages[messageIndex], CHAR_SPACING);
            slideState = SLIDE_ANIM_IN;
        }
    }
}

void Layout3SlideText::render(const ClockState &clock)
{
    (void)clock;
    dmd.selectFont(Arial_14);
    TextRenderer::clearRegion(dmd, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    TextRenderer::drawBoldTextInRegion(dmd, 0, 0, SCREEN_WIDTH, textX, 1, messages[messageIndex], CHAR_SPACING);
}

bool Layout3SlideText::isFinished() const
{
    return finished;
}
