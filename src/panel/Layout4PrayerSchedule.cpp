#include "panel/Layout4PrayerSchedule.h"

#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#include "datetime/date_and_time.h"
#include "fonts/SystemFont5x7.h"
#include "panel/DisplayConfig.h"
#include "panel/TextRenderer.h"

namespace {
const int TOP_Y = 0;
const int TOP_HEIGHT = 8;
const int BOTTOM_Y = 8;
const int BOTTOM_HEIGHT = 8;
const int TOP_CHAR_SPACING = 2;
const int BOTTOM_CHAR_SPACING = 3;
const uint32_t TOP_ANIM_MS = 70;
const uint32_t TOP_HOLD_MS = 4000;
const uint32_t BOTTOM_SCROLL_MS = 55;
const uint32_t LIGHT_MS = 45;

// const char bottomMessage[] =
//   "      12 JUMADIL AKHIR 1448 H      "
//   "      JAGA KEBERHISAN DAN KEKHUSYUKAN IBADAH      "
//   "      MASJID BAITUROHMAH-GUMUKMOJO      ";
const char bottomMessage[] =
  "      12 JUMADIL AKHIR 1448 H      ";

void uppercaseCopy(char *target, size_t targetSize, const char *source)
{
  if (targetSize == 0) {
    return;
  }

  size_t i = 0;
  for (; source[i] != '\0' && i < targetSize - 1; i++) {
    char c = source[i];
    if (c >= 'a' && c <= 'z') {
      c = c - 'a' + 'A';
    }
    target[i] = c;
  }
  target[i] = '\0';
}
}

Layout4PrayerSchedule::Layout4PrayerSchedule(DMD &display)
    : dmd(display),
      topMode(TOP_TIME),
      topAnimState(TOP_ANIM_IN),
      topTextY(-SYSTEM5x7_HEIGHT),
      bottomTextX(SCREEN_WIDTH),
      lightX(0),
      startedAt(0),
      lastTopAnimAt(0),
      topHoldStartedAt(0),
      lastBottomScrollAt(0),
      lastLightAt(0),
      finished(false),
      bottomHasEntered(false)
{
  topText[0] = '\0';
}

void Layout4PrayerSchedule::begin()
{
  dmd.selectFont(System5x7);
  dmd.clearScreen(true);

  topMode = TOP_TIME;
  topAnimState = TOP_ANIM_IN;
  topTextY = -SYSTEM5x7_HEIGHT;
  bottomTextX = SCREEN_WIDTH;
  lightX = 0;
  finished = false;
  bottomHasEntered = false;

  const uint32_t now = millis();
  startedAt = now;
  lastTopAnimAt = now;
  topHoldStartedAt = now;
  lastBottomScrollAt = now;
  lastLightAt = now;
  updateTopText(ClockState{0, false});
}

void Layout4PrayerSchedule::update(const ClockState &clock)
{
  updateTopText(clock);
  updateTopAnimation();
  updateBottomScroll();
  updateLightAnimation();
}

void Layout4PrayerSchedule::render(const ClockState &clock)
{
  (void)clock;
  dmd.selectFont(System5x7);
  drawTop();
  drawBottom();
  drawMosqueLight();
}

bool Layout4PrayerSchedule::isFinished() const
{
  return finished;
}

void Layout4PrayerSchedule::nextTopMode()
{
  if (topMode == TOP_TIME) {
    topMode = TOP_DAY;
  } else if (topMode == TOP_DAY) {
    topMode = TOP_DATE;
  } else {
    topMode = TOP_TIME;
  }

  topAnimState = TOP_ANIM_IN;
  topTextY = -SYSTEM5x7_HEIGHT;
  lastTopAnimAt = millis();
}

void Layout4PrayerSchedule::updateTopText(const ClockState &clock)
{
  if (topMode == TOP_TIME) {
    snprintf(
      topText,
      sizeof(topText),
      "%02u:%02u:%02u",
      clock.hour(),
      clock.minute(),
      static_cast<unsigned int>(clock.seconds % 60UL)
    );
    return;
  }

  Date today = dayNow();
  if (topMode == TOP_DAY) {
    uppercaseCopy(topText, sizeof(topText), today.dayName);
    return;
  }

  char dayName[8];
  uppercaseCopy(dayName, sizeof(dayName), today.dayName);
  snprintf(topText, sizeof(topText), "%02u/%02u/%04u", today.day, today.month, today.year);
}

void Layout4PrayerSchedule::updateTopAnimation()
{
  const uint32_t now = millis();

  if (topAnimState == TOP_ANIM_HOLD) {
    if (now - topHoldStartedAt >= TOP_HOLD_MS) {
      topAnimState = TOP_ANIM_OUT;
      lastTopAnimAt = now;
    }
    return;
  }

  if (now - lastTopAnimAt < TOP_ANIM_MS) {
    return;
  }
  lastTopAnimAt = now;

  if (topAnimState == TOP_ANIM_IN) {
    topTextY++;
    if (topTextY >= 0) {
      topTextY = 0;
      topAnimState = TOP_ANIM_HOLD;
      topHoldStartedAt = now;
    }
    return;
  }

  topTextY--;
  if (topTextY <= -SYSTEM5x7_HEIGHT) {
    nextTopMode();
  }
}

void Layout4PrayerSchedule::updateBottomScroll()
{
  const uint32_t now = millis();
  if (now - lastBottomScrollAt < BOTTOM_SCROLL_MS) {
    return;
  }

  lastBottomScrollAt = now;
  bottomTextX--;

  dmd.selectFont(System5x7);
  const int textWidth = TextRenderer::textWidth(dmd, bottomMessage, BOTTOM_CHAR_SPACING);
  if (bottomTextX <= 0) {
    bottomHasEntered = true;
  }

  if (bottomTextX < -textWidth) {
    if (bottomHasEntered) {
      finished = true;
      return;
    }

    bottomTextX = SCREEN_WIDTH;
  }
}

void Layout4PrayerSchedule::updateLightAnimation()
{
  const uint32_t now = millis();
  if (now - lastLightAt < LIGHT_MS) {
    return;
  }

  lastLightAt = now;
  lightX++;
  if (lightX >= SCREEN_WIDTH) {
    lightX = 0;
  }
}

void Layout4PrayerSchedule::drawTop()
{
  TextRenderer::clearRegion(dmd, 0, TOP_Y, SCREEN_WIDTH, TOP_HEIGHT);
  if (topMode == TOP_TIME) {
    drawBoldCenteredTopText(topText);
  } else {
    drawCenteredTopText(topText);
  }

  dmd.writePixel(0, 7, GRAPHICS_NORMAL, true);
  dmd.writePixel(SCREEN_WIDTH - 1, 7, GRAPHICS_NORMAL, true);
}

void Layout4PrayerSchedule::drawBottom()
{
  TextRenderer::clearRegion(dmd, 0, BOTTOM_Y, SCREEN_WIDTH, BOTTOM_HEIGHT);
  TextRenderer::drawBoldTextInRegion(
    dmd,
    0,
    BOTTOM_Y,
    SCREEN_WIDTH,
    bottomTextX,
    1,
    bottomMessage,
    BOTTOM_CHAR_SPACING
  );
}

void Layout4PrayerSchedule::drawCenteredTopText(const char *text)
{
  const int textWidth = TextRenderer::textWidth(dmd, text, TOP_CHAR_SPACING);
  const int x = textWidth < SCREEN_WIDTH ? (SCREEN_WIDTH - textWidth) / 2 : 0;
  TextRenderer::drawTextInRegion(dmd, 0, TOP_Y, SCREEN_WIDTH, x, topTextY, text, TOP_CHAR_SPACING);
}

void Layout4PrayerSchedule::drawBoldCenteredTopText(const char *text)
{
  const int textWidth = TextRenderer::textWidth(dmd, text, TOP_CHAR_SPACING);
  const int x = textWidth < SCREEN_WIDTH - 1 ? (SCREEN_WIDTH - textWidth - 1) / 2 : 0;
  TextRenderer::drawBoldTextInRegion(dmd, 0, TOP_Y, SCREEN_WIDTH, x, topTextY, text, TOP_CHAR_SPACING);
}

void Layout4PrayerSchedule::drawMosqueLight()
{
  for (int i = 0; i < SCREEN_WIDTH; i += 6) {
    dmd.writePixel(i, 7, GRAPHICS_NORMAL, true);
  }

  dmd.writePixel(lightX, 7, GRAPHICS_NORMAL, true);
  if (lightX > 0) {
    dmd.writePixel(lightX - 1, 7, GRAPHICS_NORMAL, true);
  }
  if (lightX + 1 < SCREEN_WIDTH) {
    dmd.writePixel(lightX + 1, 7, GRAPHICS_NORMAL, true);
  }
}
