#include "panel/Layout2FullRunning.h"

#include "panel/DisplayConfig.h"
#include "panel/TextRenderer.h"
#include "fonts/Arial14.h"

namespace {
const uint32_t SCROLL_MS = 70;
const int CHAR_SPACING = 2;
const char message[] = "LAYOUT 2  -  INFORMASI BERJALAN DI 3 PANEL P10      ";
}

Layout2FullRunning::Layout2FullRunning(DMD &display)
    : dmd(display),
      textX(SCREEN_WIDTH),
      lastScrollAt(0),
      finished(false)
{
}

void Layout2FullRunning::begin()
{
    dmd.selectFont(Arial_14);
    dmd.clearScreen(true);
    textX = SCREEN_WIDTH;
    lastScrollAt = millis();
    finished = false;
}

void Layout2FullRunning::update(const ClockState &clock)
{
    (void)clock;
    dmd.selectFont(Arial_14);

    const uint32_t now = millis();
    if (now - lastScrollAt < SCROLL_MS) {
        return;
    }
    lastScrollAt = now;

    textX--;
    if (textX < -TextRenderer::textWidth(dmd, message, CHAR_SPACING)) {
        finished = true;
    }
}

void Layout2FullRunning::render(const ClockState &clock)
{
    (void)clock;
    dmd.selectFont(Arial_14);
    TextRenderer::clearRegion(dmd, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    TextRenderer::drawBoldTextInRegion(dmd, 0, 0, SCREEN_WIDTH, textX, 1, message, CHAR_SPACING);
}

bool Layout2FullRunning::isFinished() const
{
    return finished;
}
