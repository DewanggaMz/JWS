#include "panel/Layout4PrayerSchedule.h"

#include "panel/BigClockRenderer.h"
#include "panel/DisplayConfig.h"
#include "panel/TextRenderer.h"

namespace {
const int RIGHT_X = 32;
const int RIGHT_WIDTH = SCREEN_WIDTH - RIGHT_X;
const int CELL_WIDTH = RIGHT_WIDTH / 2;
const int CHAR_WIDTH = 3;
const int CHAR_SPACING = 1;
const uint32_t DISPLAY_MS = 10000;

struct PrayerTime {
    const char *text;
};

const PrayerTime prayerTimes[] = {
    {"IM 04:20"},
    {"SB 04:30"},
    {"DZ 11:55"},
    {"AS 15:15"},
    {"MG 17:50"},
    {"IS 19:05"}
};

const uint8_t GLYPH_SPACE[5] = {0b000, 0b000, 0b000, 0b000, 0b000};
const uint8_t GLYPH_COLON[5] = {0b000, 0b010, 0b000, 0b010, 0b000};
const uint8_t GLYPH_0[5] = {0b111, 0b101, 0b101, 0b101, 0b111};
const uint8_t GLYPH_1[5] = {0b010, 0b110, 0b010, 0b010, 0b111};
const uint8_t GLYPH_2[5] = {0b111, 0b001, 0b111, 0b100, 0b111};
const uint8_t GLYPH_3[5] = {0b111, 0b001, 0b111, 0b001, 0b111};
const uint8_t GLYPH_4[5] = {0b101, 0b101, 0b111, 0b001, 0b001};
const uint8_t GLYPH_5[5] = {0b111, 0b100, 0b111, 0b001, 0b111};
const uint8_t GLYPH_6[5] = {0b111, 0b100, 0b111, 0b101, 0b111};
const uint8_t GLYPH_7[5] = {0b111, 0b001, 0b010, 0b010, 0b010};
const uint8_t GLYPH_8[5] = {0b111, 0b101, 0b111, 0b101, 0b111};
const uint8_t GLYPH_9[5] = {0b111, 0b101, 0b111, 0b001, 0b111};
const uint8_t GLYPH_A[5] = {0b010, 0b101, 0b111, 0b101, 0b101};
const uint8_t GLYPH_B[5] = {0b110, 0b101, 0b110, 0b101, 0b110};
const uint8_t GLYPH_D[5] = {0b110, 0b101, 0b101, 0b101, 0b110};
const uint8_t GLYPH_G[5] = {0b111, 0b100, 0b101, 0b101, 0b111};
const uint8_t GLYPH_I[5] = {0b111, 0b010, 0b010, 0b010, 0b111};
const uint8_t GLYPH_M[5] = {0b101, 0b111, 0b111, 0b101, 0b101};
const uint8_t GLYPH_S[5] = {0b111, 0b100, 0b111, 0b001, 0b111};
const uint8_t GLYPH_Z[5] = {0b111, 0b001, 0b010, 0b100, 0b111};
}

Layout4PrayerSchedule::Layout4PrayerSchedule(DMD &display)
    : dmd(display),
      startedAt(0),
      finished(false)
{
}

void Layout4PrayerSchedule::begin()
{
    dmd.clearScreen(true);
    startedAt = millis();
    finished = false;
}

void Layout4PrayerSchedule::update(const ClockState &clock)
{
    (void)clock;
    if (millis() - startedAt >= DISPLAY_MS) {
        finished = true;
    }
}

void Layout4PrayerSchedule::render(const ClockState &clock)
{
    BigClockRenderer::draw(dmd, clock);
    drawSchedule();
}

bool Layout4PrayerSchedule::isFinished() const
{
    return finished;
}

void Layout4PrayerSchedule::drawSchedule()
{
    TextRenderer::clearRegion(dmd, RIGHT_X, 0, RIGHT_WIDTH, SCREEN_HEIGHT);

    for (uint8_t i = 0; i < 6; i++) {
        const int col = i % 2;
        const int row = i / 2;
        const int x = RIGHT_X + (col * CELL_WIDTH);
        const int y = row * 5;
        drawCellText(x, y, prayerTimes[i].text);
    }
}

void Layout4PrayerSchedule::drawCellText(int x, int y, const char *text)
{
    int cursorX = x;
    for (uint8_t i = 0; text[i] != '\0'; i++) {
        drawSmallChar(cursorX, y, text[i]);
        cursorX += CHAR_WIDTH + CHAR_SPACING;
    }
}

void Layout4PrayerSchedule::drawSmallChar(int x, int y, char c)
{
    const uint8_t *glyph = glyphFor(c);
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 3; col++) {
            if ((glyph[row] & (1 << (2 - col))) != 0) {
                dmd.writePixel(x + col, y + row, GRAPHICS_NORMAL, true);
            }
        }
    }
}

const uint8_t *Layout4PrayerSchedule::glyphFor(char c)
{
    switch (c) {
    case '0': return GLYPH_0;
    case '1': return GLYPH_1;
    case '2': return GLYPH_2;
    case '3': return GLYPH_3;
    case '4': return GLYPH_4;
    case '5': return GLYPH_5;
    case '6': return GLYPH_6;
    case '7': return GLYPH_7;
    case '8': return GLYPH_8;
    case '9': return GLYPH_9;
    case ':': return GLYPH_COLON;
    case 'A': return GLYPH_A;
    case 'B': return GLYPH_B;
    case 'D': return GLYPH_D;
    case 'G': return GLYPH_G;
    case 'I': return GLYPH_I;
    case 'M': return GLYPH_M;
    case 'S': return GLYPH_S;
    case 'Z': return GLYPH_Z;
    default: return GLYPH_SPACE;
    }
}
