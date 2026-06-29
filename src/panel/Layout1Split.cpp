#include "panel/Layout1Split.h"

#include "panel/BigClockRenderer.h"
#include "panel/DisplayConfig.h"
#include "panel/TextRenderer.h"
#include "fonts/SystemFont5x7.h"

#include <stdio.h>

namespace {
const int RIGHT_X = 32;
const int RIGHT_WIDTH = SCREEN_WIDTH - RIGHT_X;
const int TOP_Y = 0;
const int TOP_HEIGHT = 8;
const int BOTTOM_Y = 8;
const int BOTTOM_HEIGHT = 8;

const uint32_t TOP_ANIM_MS = 70;
const uint32_t TOP_PRE_SCROLL_HOLD_MS = 900;
const uint32_t TOP_SCROLL_MS = 90;
const uint32_t TOP_HOLD_MS = 1800;
const int TOP_CHAR_SPACING = 1;
const int BOTTOM_CHAR_SPACING = 3;
}

Layout1Split::Layout1Split(
    DMD &display,
    const String &bottomMessage,
    uint8_t repeatTarget,
    uint16_t speedMs
)
    : dmd(display),
      topState(TOP_ANIM_IN),
      repeatTarget(repeatTarget),
      repeatCount(0),
      topMessageIndex(0),
      topTextX(RIGHT_X),
      topTextY(-SYSTEM5x7_HEIGHT),
      bottomTextX(SCREEN_WIDTH),
      lastTopAnimAt(0),
      lastTopScrollAt(0),
      lastTopHoldAt(0),
      lastBottomScrollAt(0),
      bottomScrollMs(speedMs),
      finished(false),
      bottomMessage(bottomMessage),
      currentSchedule{},
      showImsak(false),
      showSunrise(false),
      showDhuha(false),
      prayerMessageCount(0)
{
    PrayerSchedule schedule{};
    schedule.subuh = {5, 0, true};
    schedule.dzuhur = {11, 0, true};
    schedule.ashar = {14, 0, true};
    schedule.maghrib = {16, 0, true};
    schedule.isya = {18, 0, true};
    schedule.valid = true;
    setPrayerSchedule(schedule);
}

void Layout1Split::begin()
{
    dmd.selectFont(System5x7);
    dmd.clearScreen(true);
    topState = TOP_ANIM_IN;
    repeatCount = 0;
    topMessageIndex = 0;
    resetTopMessagePosition();
    topTextY = -SYSTEM5x7_HEIGHT;
    bottomTextX = SCREEN_WIDTH;
    finished = false;

    const uint32_t now = millis();
    lastTopAnimAt = now;
    lastTopScrollAt = now;
    lastTopHoldAt = now;
    lastBottomScrollAt = now;
}

void Layout1Split::update(const ClockState &clock)
{
    (void)clock;
    updateTopAnimation();
    updateBottomScroll();
}

void Layout1Split::render(const ClockState &clock)
{
    dmd.selectFont(System5x7);
    BigClockRenderer::draw(dmd, clock);
    drawRightPanel();
}

bool Layout1Split::isFinished() const
{
    return finished;
}

void Layout1Split::setMessage(
    const String &message,
    uint8_t newRepeatTarget,
    uint16_t speedMs
)
{
    bottomMessage = message;
    repeatTarget = newRepeatTarget == 0 ? 1 : newRepeatTarget;
    bottomScrollMs = speedMs;
}

void Layout1Split::resetTopMessagePosition()
{
    dmd.selectFont(System5x7);

    const char *topText = currentTopMessage();
    const int topWidth = TextRenderer::textWidth(dmd, topText, TOP_CHAR_SPACING);
    if (topWidth <= RIGHT_WIDTH) {
        topTextX = RIGHT_X + ((RIGHT_WIDTH - topWidth) / 2);
        return;
    }

    topTextX = RIGHT_X;
}

bool Layout1Split::topTextNeedsScroll()
{
    dmd.selectFont(System5x7);
    return TextRenderer::textWidth(dmd, currentTopMessage(), TOP_CHAR_SPACING) > RIGHT_WIDTH;
}

void Layout1Split::updateTopAnimation()
{
    const uint32_t now = millis();

    if (topState == TOP_ANIM_PRE_SCROLL_HOLD) {
        if (now - lastTopHoldAt >= TOP_PRE_SCROLL_HOLD_MS) {
            topState = TOP_ANIM_SCROLL;
            lastTopScrollAt = now;
        }
        return;
    }

    if (topState == TOP_ANIM_SCROLL) {
        if (now - lastTopScrollAt < TOP_SCROLL_MS) {
            return;
        }
        lastTopScrollAt = now;

        const int topWidth = TextRenderer::textWidth(dmd, currentTopMessage(), TOP_CHAR_SPACING);
        const int targetX = RIGHT_X - (topWidth - RIGHT_WIDTH);

        topTextX--;
        if (topTextX <= targetX) {
            topTextX = targetX;
            topState = TOP_ANIM_HOLD;
            lastTopHoldAt = now;
        }
        return;
    }

    if (topState == TOP_ANIM_HOLD) {
        if (now - lastTopHoldAt >= TOP_HOLD_MS) {
            topState = TOP_ANIM_OUT;
            lastTopAnimAt = now;
        }
        return;
    }

    if (now - lastTopAnimAt < TOP_ANIM_MS) {
        return;
    }
    lastTopAnimAt = now;

    if (topState == TOP_ANIM_IN) {
        topTextY++;
        if (topTextY >= 0) {
            topTextY = 0;
            if (topTextNeedsScroll()) {
                topState = TOP_ANIM_PRE_SCROLL_HOLD;
                lastTopHoldAt = now;
            } else {
                topState = TOP_ANIM_HOLD;
                lastTopHoldAt = now;
            }
        }
    } else {
        topTextY--;
        if (topTextY <= -SYSTEM5x7_HEIGHT) {
            topMessageIndex++;
            if (topMessageIndex >= prayerMessageCount) {
                repeatCount++;
                if (repeatCount >= repeatTarget) {
                    finished = true;
                    return;
                }

                topMessageIndex = 0;
            }

            resetTopMessagePosition();
            topTextY = -SYSTEM5x7_HEIGHT;
            topState = TOP_ANIM_IN;
        }
    }
}

void Layout1Split::updateBottomScroll()
{
    dmd.selectFont(System5x7);

    const uint32_t now = millis();
    if (now - lastBottomScrollAt < bottomScrollMs) {
        return;
    }
    lastBottomScrollAt = now;

    bottomTextX--;
    if (bottomTextX < RIGHT_X - TextRenderer::textWidth(dmd, bottomMessage.c_str(), BOTTOM_CHAR_SPACING)) {
        bottomTextX = SCREEN_WIDTH;
    }
}

void Layout1Split::drawRightPanel()
{
    dmd.selectFont(System5x7);

    TextRenderer::clearRegion(dmd, RIGHT_X, TOP_Y, RIGHT_WIDTH, TOP_HEIGHT);
    TextRenderer::clearRegion(dmd, RIGHT_X, BOTTOM_Y, RIGHT_WIDTH, BOTTOM_HEIGHT);

    const char *topText = currentTopMessage();
    TextRenderer::drawTextInRegion(dmd, RIGHT_X, TOP_Y, RIGHT_WIDTH, topTextX, topTextY, topText, TOP_CHAR_SPACING);
    TextRenderer::drawBoldTextInRegion(
        dmd,
        RIGHT_X,
        BOTTOM_Y,
        RIGHT_WIDTH,
        bottomTextX,
        0,
        bottomMessage.c_str(),
        BOTTOM_CHAR_SPACING
    );
}

void Layout1Split::setPrayerSchedule(const PrayerSchedule &schedule)
{
    currentSchedule = schedule;
    rebuildPrayerMessages();
    resetTopMessagePosition();
}

void Layout1Split::setPrayerDisplayConfig(
    bool newShowImsak,
    bool newShowSunrise,
    bool newShowDhuha
)
{
    showImsak = newShowImsak;
    showSunrise = newShowSunrise;
    showDhuha = newShowDhuha;
    rebuildPrayerMessages();

    topMessageIndex = 0;
    topState = TOP_ANIM_IN;
    topTextY = -SYSTEM5x7_HEIGHT;
    lastTopAnimAt = millis();
    resetTopMessagePosition();
}

const char *Layout1Split::currentTopMessage() const
{
    if (prayerMessageCount == 0 ||
        topMessageIndex >= prayerMessageCount) {
        return "";
    }

    return prayerMessages[topMessageIndex];
}

void Layout1Split::setPrayerMessage(uint8_t index, const char *label, const PrayerScheduleTime &time)
{
    if (index >= MAX_PRAYER_MESSAGES) {
        return;
    }

    if (time.valid) {
        snprintf(prayerMessages[index], sizeof(prayerMessages[index]), "%s %02u:%02u ", label, time.hour, time.minute);
    } else {
        snprintf(prayerMessages[index], sizeof(prayerMessages[index]), "%s --:--", label);
    }
}

void Layout1Split::rebuildPrayerMessages()
{
    prayerMessageCount = 0;

    if (showImsak) {
        setPrayerMessage(
            prayerMessageCount++,
            "IMSAK",
            currentSchedule.imsak
        );
    }

    setPrayerMessage(
        prayerMessageCount++,
        "SUBUH",
        currentSchedule.subuh
    );

    if (showSunrise) {
        setPrayerMessage(
            prayerMessageCount++,
            "TERBIT",
            currentSchedule.terbit
        );
    }

    if (showDhuha) {
        setPrayerMessage(
            prayerMessageCount++,
            "DHUHA",
            currentSchedule.duha
        );
    }

    setPrayerMessage(
        prayerMessageCount++,
        "DZUHUR",
        currentSchedule.dzuhur
    );
    setPrayerMessage(
        prayerMessageCount++,
        "ASHAR",
        currentSchedule.ashar
    );
    setPrayerMessage(
        prayerMessageCount++,
        "MAGHRIB",
        currentSchedule.maghrib
    );
    setPrayerMessage(
        prayerMessageCount++,
        "ISYA",
        currentSchedule.isya
    );

    if (topMessageIndex >= prayerMessageCount) {
        topMessageIndex = 0;
    }
}
