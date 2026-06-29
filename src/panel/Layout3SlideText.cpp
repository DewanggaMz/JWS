#include "panel/Layout3SlideText.h"

#include "panel/DisplayConfig.h"
#include "panel/TextRenderer.h"
#include "fonts/Arial14.h"

namespace {
const uint32_t ANIM_MS = 10;
const uint32_t HOLD_MS = 1600;
const int CHAR_SPACING = 2;
}

Layout3SlideText::Layout3SlideText(
    DMD &display,
    const String initialMessages[],
    uint8_t initialMessageCount
)
    : dmd(display),
      slideState(SLIDE_ANIM_IN),
      messageIndex(0),
      textX(-SCREEN_WIDTH),
      lastAnimAt(0),
      lastHoldAt(0),
      finished(false),
      messageCount(min(initialMessageCount, static_cast<uint8_t>(12)))
{
    if (messageCount == 0) {
        messages[0] = " ";
        messageCount = 1;
        return;
    }

    for (uint8_t i = 0; i < messageCount; i++) {
        messages[i] = initialMessages[i];
    }
}

void Layout3SlideText::begin()
{
    dmd.selectFont(Arial_14);
    dmd.clearScreen(true);
    slideState = SLIDE_ANIM_IN;
    messageIndex = 0;
    textX = -TextRenderer::textWidth(dmd, messages[messageIndex].c_str(), CHAR_SPACING);
    finished = false;

    const uint32_t now = millis();
    lastAnimAt = now;
    lastHoldAt = now;
}

void Layout3SlideText::update(const ClockState &clock)
{
    (void)clock;
    dmd.selectFont(Arial_14);

    const char *text = messages[messageIndex].c_str();
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

            textX = -TextRenderer::textWidth(dmd, messages[messageIndex].c_str(), CHAR_SPACING);
            slideState = SLIDE_ANIM_IN;
        }
    }
}

void Layout3SlideText::render(const ClockState &clock)
{
    (void)clock;
    dmd.selectFont(Arial_14);
    TextRenderer::clearRegion(dmd, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    TextRenderer::drawBoldTextInRegion(
        dmd,
        0,
        0,
        SCREEN_WIDTH,
        textX,
        1,
        messages[messageIndex].c_str(),
        CHAR_SPACING
    );
}

bool Layout3SlideText::isFinished() const
{
    return finished;
}

void Layout3SlideText::setMessages(
    const String newMessages[],
    uint8_t newMessageCount
)
{
    messageCount = min(newMessageCount, static_cast<uint8_t>(12));
    if (messageCount == 0) {
        messages[0] = " ";
        messageCount = 1;
        return;
    }

    for (uint8_t i = 0; i < messageCount; i++) {
        messages[i] = newMessages[i];
    }

    messageIndex = 0;
    slideState = SLIDE_ANIM_IN;
    textX = -SCREEN_WIDTH;
    finished = false;
    lastAnimAt = millis();
    lastHoldAt = lastAnimAt;
}
