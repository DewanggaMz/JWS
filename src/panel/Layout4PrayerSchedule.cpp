#include "panel/Layout4PrayerSchedule.h"

#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#include "datetime/date_and_time.h"
#include "datetime/hijriah.h"
#include "datetime/pasaran.h"
#include "fonts/SystemFont5x7.h"
#include "panel/DisplayConfig.h"
#include "panel/TextRenderer.h"


namespace {
const int TOP_Y = 0;
const int TOP_HEIGHT = 8;
const int BOTTOM_Y = 8;
const int BOTTOM_HEIGHT = 8;
const int TOP_CHAR_SPACING = 2;
const int TOP_CHAR_LIGHT_SPACING = 1;
const int BOTTOM_CHAR_SPACING = 3;
const uint32_t TOP_ANIM_MS = 70;
const uint32_t TOP_HOLD_MS = 4000;
const uint32_t LIGHT_MS = 45;

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

Layout4PrayerSchedule::Layout4PrayerSchedule(
  DMD &display,
  const String &runningMessage,
  bool showPasaran,
  bool showHijriDate,
  uint8_t repeatTarget,
  int hijriCorrection,
  uint16_t speedMs
)
    : dmd(display),
      topMode(TOP_TIME),
      topAnimState(TOP_ANIM_IN),
      bottomAnimState(
        showHijriDate ? BOTTOM_HIJRI_RUNNING : BOTTOM_RUNNING
      ),
      topTextY(-SYSTEM5x7_HEIGHT),
      bottomTextX(SCREEN_WIDTH),
      lightX(0),
      lastTopAnimAt(0),
      topHoldStartedAt(0),
      lastBottomAnimAt(0),
      lastLightAt(0),
      bottomScrollMs(speedMs),
      finished(false),
      showPasaran(showPasaran),
      showHijriDate(showHijriDate),
      repeatTarget(repeatTarget == 0 ? 1 : repeatTarget),
      repeatCount(0),
      hijriCorrection(hijriCorrection),
      runningMessage(runningMessage)
{
  topText[0] = '\0';
  dayText[0] = '\0';
  dateText[0] = '\0';
  hijriText[0] = '\0';
}

void Layout4PrayerSchedule::begin()
{
  dmd.selectFont(System5x7);
  dmd.clearScreen(true);

  finished = false;
  repeatCount = 0;

  const uint32_t now = millis();
  updateDateCache();
  resetAnimationCycle(now);
  updateTopText(ClockState{0, false});
}

void Layout4PrayerSchedule::update(const ClockState &clock)
{
  updateTopText(clock);
  updateTopAnimation();
  updateBottomAnimation();
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

void Layout4PrayerSchedule::setConfiguration(
  const String &newRunningMessage,
  bool newShowPasaran,
  bool newShowHijriDate,
  uint8_t newRepeatTarget,
  int newHijriCorrection,
  uint16_t speedMs
)
{
  runningMessage = newRunningMessage;
  showPasaran = newShowPasaran;
  showHijriDate = newShowHijriDate;
  repeatTarget = newRepeatTarget == 0 ? 1 : newRepeatTarget;
  hijriCorrection = newHijriCorrection;
  bottomScrollMs = speedMs;
  updateDateCache();
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

  if (topMode == TOP_DAY) {
    updateDateCache();
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

  if (topMode == TOP_DAY) {
    snprintf(topText, sizeof(topText), "%s", dayText);
    return;
  }

  snprintf(topText, sizeof(topText), "%s", dateText);
}

void Layout4PrayerSchedule::updateDateCache()
{
  Date today = dayNow();
  char dayName[8];
  uppercaseCopy(dayName, sizeof(dayName), today.dayName);
  if (showPasaran) {
    const String pasaran =
      getPasaran(today.day, today.month, today.year);
    char pasaranJawa[8];
    uppercaseCopy(
      pasaranJawa,
      sizeof(pasaranJawa),
      pasaran.c_str()
    );
    snprintf(
      dayText,
      sizeof(dayText),
      "%s %s",
      dayName,
      pasaranJawa
    );
  } else {
    snprintf(dayText, sizeof(dayText), "%s", dayName);
  }

  snprintf(
    dateText,
    sizeof(dateText),
    "%02u / %02u / %04u",
    today.day,
    today.month,
    today.year
  );

  HijriDate hijri = HijriModule::getHijriDate(
    today.year,
    today.month,
    today.day,
    hijriCorrection
  );

  char hijriDayName[20];
  uppercaseCopy(
    hijriDayName,
    sizeof(hijriDayName),
    getHijriMonthName(hijri.month)
  );
  snprintf(
    hijriText,
    sizeof(hijriText),
    "%u %s %u H",
    static_cast<unsigned int>(hijri.day),
    hijriDayName,
    static_cast<unsigned int>(hijri.year)
  );
}

void Layout4PrayerSchedule::resetAnimationCycle(uint32_t now)
{
  topMode = TOP_TIME;
  topAnimState = TOP_ANIM_IN;
  bottomAnimState =
    showHijriDate ? BOTTOM_HIJRI_RUNNING : BOTTOM_RUNNING;
  topTextY = -SYSTEM5x7_HEIGHT;
  bottomTextX = SCREEN_WIDTH;
  lightX = 0;
  lastTopAnimAt = now;
  topHoldStartedAt = now;
  lastBottomAnimAt = now;
  lastLightAt = now;
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

void Layout4PrayerSchedule::updateBottomAnimation()
{
  const uint32_t now = millis();
  if (now - lastBottomAnimAt < bottomScrollMs) {
    return;
  }
  lastBottomAnimAt = now;

  dmd.selectFont(System5x7);
  bottomTextX--;
  const char *text =
    bottomAnimState == BOTTOM_HIJRI_RUNNING
      ? hijriText
      : runningMessage.c_str();
  const int textWidth = TextRenderer::textWidth(
    dmd,
    text,
    BOTTOM_CHAR_SPACING
  );

  if (bottomTextX >= -textWidth) {
    return;
  }

  if (bottomAnimState == BOTTOM_HIJRI_RUNNING) {
    bottomAnimState = BOTTOM_RUNNING;
    bottomTextX = SCREEN_WIDTH;
  } else {
    repeatCount++;
    if (repeatCount >= repeatTarget) {
      finished = true;
      return;
    }

    updateDateCache();
    resetAnimationCycle(now);
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
  const char *text =
    bottomAnimState == BOTTOM_HIJRI_RUNNING
      ? hijriText
      : runningMessage.c_str();

  TextRenderer::drawBoldTextInRegion(
    dmd,
    0,
    BOTTOM_Y,
    SCREEN_WIDTH,
    bottomTextX,
    1,
    text,
    BOTTOM_CHAR_SPACING
  );
}

void Layout4PrayerSchedule::drawCenteredTopText(const char *text)
{
  const int textWidth = TextRenderer::textWidth(dmd, text, TOP_CHAR_LIGHT_SPACING);
  const int x = textWidth < SCREEN_WIDTH ? (SCREEN_WIDTH - textWidth) / 2 : 0;
  TextRenderer::drawTextInRegion(dmd, 0, TOP_Y, SCREEN_WIDTH, x, topTextY, text, TOP_CHAR_LIGHT_SPACING);
}

void Layout4PrayerSchedule::drawBoldCenteredTopText(const char *text)
{
  const int textWidth = TextRenderer::textWidth(dmd, text, TOP_CHAR_SPACING);
  const int x = textWidth < SCREEN_WIDTH - 1 ? (SCREEN_WIDTH - textWidth - 1) / 2 : 0;
  TextRenderer::drawBoldTextInRegion(dmd, 0, TOP_Y, SCREEN_WIDTH, x, topTextY, text, TOP_CHAR_SPACING);
}

void Layout4PrayerSchedule::drawMosqueLight()
{
  for (int i = 0; i < SCREEN_WIDTH; i += 12) {
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
