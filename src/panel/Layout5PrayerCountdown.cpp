#include "panel/Layout5PrayerCountdown.h"

#include <stdio.h>

#include "fonts/SystemFont5x7.h"
#include "panel/DisplayConfig.h"
#include "panel/TextRenderer.h"

namespace {

const uint32_t SLIDE_ANIMATION_MS = 20;
const uint32_t COUNTDOWN_HOLD_MS = 500;
const uint32_t RUNNING_SCROLL_MS = 65;
const int SLIDE_STEP = 2;
const int COUNTDOWN_CHAR_SPACING = 2;
const int MESSAGE_CHAR_SPACING = 2;
const int TOP_Y = 0;
const int TOP_HEIGHT = 8;
const int BOTTOM_Y = 8;
const int BOTTOM_HEIGHT = 8;

struct PrayerTarget {
  const char *name;
  const PrayerScheduleTime *time;
};

}

Layout5PrayerCountdown::Layout5PrayerCountdown(
  DMD &display,
  const PrayerSchedule &initialSchedule
)
  : dmd(display),
    schedule(initialSchedule),
    state(COUNTDOWN_SLIDE_IN),
    targetSelected(false),
    finished(false),
    targetSecond(0),
    countdownX(-SCREEN_WIDTH),
    runningX(SCREEN_WIDTH),
    lastAnimationAt(0),
    holdStartedAt(0),
    lastScrollAt(0),
    prayerName(""),
    runningMessage("")
{
  snprintf(countdownText, sizeof(countdownText), "00:00:00");
}

void Layout5PrayerCountdown::begin()
{
  dmd.selectFont(System5x7);
  dmd.clearScreen(true);

  state = COUNTDOWN_SLIDE_IN;
  targetSelected = false;
  finished = false;
  targetSecond = 0;
  countdownX = -SCREEN_WIDTH;
  runningX = SCREEN_WIDTH;
  prayerName = "";
  runningMessage = "";
  snprintf(countdownText, sizeof(countdownText), "00:00:00");

  const uint32_t now = millis();
  lastAnimationAt = now;
  holdStartedAt = now;
  lastScrollAt = now;
}

void Layout5PrayerCountdown::update(const ClockState &clock)
{
  if (finished) {
    return;
  }

  if (!targetSelected) {
    if (!selectNextPrayer(clock)) {
      finished = true;
      return;
    }
    targetSelected = true;
  }

  updateCountdown(clock);

  if (state == COUNTDOWN_SLIDE_IN) {
    updateSlideAnimation();
    return;
  }

  if (state == COUNTDOWN_HOLD) {
    if (millis() - holdStartedAt >= COUNTDOWN_HOLD_MS) {
      state = RUNNING_MESSAGE;
      runningX = SCREEN_WIDTH;
      lastScrollAt = millis();
    }
    return;
  }

  updateRunningMessage();
}

void Layout5PrayerCountdown::render(const ClockState &clock)
{
  (void)clock;
  dmd.selectFont(System5x7);
  TextRenderer::clearRegion(
    dmd,
    0,
    TOP_Y,
    SCREEN_WIDTH,
    TOP_HEIGHT
  );
  TextRenderer::clearRegion(
    dmd,
    0,
    BOTTOM_Y,
    SCREEN_WIDTH,
    BOTTOM_HEIGHT
  );

  drawCountdown();
  if (state == RUNNING_MESSAGE) {
    drawRunningMessage();
  }
}

bool Layout5PrayerCountdown::isFinished() const
{
  return finished;
}

void Layout5PrayerCountdown::setPrayerSchedule(
  const PrayerSchedule &newSchedule
)
{
  schedule = newSchedule;
  targetSelected = false;
  state = COUNTDOWN_SLIDE_IN;
  finished = false;
  countdownX = -SCREEN_WIDTH;
  runningX = SCREEN_WIDTH;
  const uint32_t now = millis();
  lastAnimationAt = now;
  holdStartedAt = now;
  lastScrollAt = now;
}

bool Layout5PrayerCountdown::selectNextPrayer(
  const ClockState &clock
)
{
  PrayerTarget prayers[] = {
    {"SUBUH", &schedule.subuh},
    {"DZUHUR", &schedule.dzuhur},
    {"ASHAR", &schedule.ashar},
    {"MAGHRIB", &schedule.maghrib},
    {"ISYA", &schedule.isya}
  };
  const size_t prayerCount =
    sizeof(prayers) / sizeof(prayers[0]);
  const uint32_t currentSecond = clock.seconds % 86400UL;

  for (size_t i = 0; i < prayerCount; i++) {
    if (!prayers[i].time->valid) {
      continue;
    }

    const uint32_t prayerSecond =
      (static_cast<uint32_t>(prayers[i].time->hour) * 3600UL) +
      (static_cast<uint32_t>(prayers[i].time->minute) * 60UL);
    if (prayerSecond >= currentSecond) {
      targetSecond = prayerSecond;
      prayerName = prayers[i].name;
      runningMessage = "MEMASUKI WAKTU SHOLAT " + prayerName;
      return true;
    }
  }

  if (!schedule.subuh.valid) {
    return false;
  }

  targetSecond =
    86400UL +
    (static_cast<uint32_t>(schedule.subuh.hour) * 3600UL) +
    (static_cast<uint32_t>(schedule.subuh.minute) * 60UL);
  prayerName = "SUBUH";
  runningMessage = "MEMASUKI WAKTU " + prayerName;
  return true;
}

void Layout5PrayerCountdown::updateCountdown(
  const ClockState &clock
)
{
  uint32_t currentSecond = clock.seconds % 86400UL;
  if (targetSecond >= 86400UL &&
      currentSecond < schedule.isya.hour * 3600UL) {
    currentSecond += 86400UL;
  }

  const uint32_t remaining =
    targetSecond > currentSecond
      ? targetSecond - currentSecond
      : 0;
  const uint8_t hours = remaining / 3600UL;
  const uint8_t minutes = (remaining % 3600UL) / 60UL;
  const uint8_t seconds = remaining % 60UL;
  snprintf(
    countdownText,
    sizeof(countdownText),
    "%02u:%02u:%02u",
    hours,
    minutes,
    seconds
  );
}

void Layout5PrayerCountdown::updateSlideAnimation()
{
  const uint32_t now = millis();
  if (now - lastAnimationAt < SLIDE_ANIMATION_MS) {
    return;
  }
  lastAnimationAt = now;

  dmd.selectFont(System5x7);
  const int textWidth = TextRenderer::textWidth(
    dmd,
    countdownText,
    COUNTDOWN_CHAR_SPACING
  );
  const int targetX = max(0, (SCREEN_WIDTH - textWidth) / 2);
  countdownX += SLIDE_STEP;
  if (countdownX >= targetX) {
    countdownX = targetX;
    state = COUNTDOWN_HOLD;
    holdStartedAt = now;
  }
}

void Layout5PrayerCountdown::updateRunningMessage()
{
  const uint32_t now = millis();
  if (now - lastScrollAt < RUNNING_SCROLL_MS) {
    return;
  }
  lastScrollAt = now;

  runningX--;
  dmd.selectFont(System5x7);
  const int textWidth = TextRenderer::textWidth(
    dmd,
    runningMessage.c_str(),
    MESSAGE_CHAR_SPACING
  );
  if (runningX < -textWidth) {
    finished = true;
  }
}

void Layout5PrayerCountdown::drawCountdown()
{
  TextRenderer::drawBoldTextInRegion(
    dmd,
    0,
    TOP_Y,
    SCREEN_WIDTH,
    countdownX,
    0,
    countdownText,
    COUNTDOWN_CHAR_SPACING
  );
}

void Layout5PrayerCountdown::drawRunningMessage()
{
  TextRenderer::drawBoldTextInRegion(
    dmd,
    0,
    BOTTOM_Y,
    SCREEN_WIDTH,
    runningX,
    0,
    runningMessage.c_str(),
    MESSAGE_CHAR_SPACING
  );
}
