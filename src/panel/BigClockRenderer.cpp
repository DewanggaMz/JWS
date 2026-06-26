#include "panel/BigClockRenderer.h"

#include "panel/DisplayConfig.h"
#include "panel/TextRenderer.h"

namespace {
const int CLOCK_X = 0;
const int CLOCK_WIDTH = 32;

const uint8_t bigDigits[10][7] = {
    {0b111, 0b101, 0b101, 0b101, 0b101, 0b101, 0b111},
    {0b010, 0b110, 0b010, 0b010, 0b010, 0b010, 0b111},
    {0b111, 0b001, 0b001, 0b111, 0b100, 0b100, 0b111},
    {0b111, 0b001, 0b001, 0b111, 0b001, 0b001, 0b111},
    {0b101, 0b101, 0b101, 0b111, 0b001, 0b001, 0b001},
    {0b111, 0b100, 0b100, 0b111, 0b001, 0b001, 0b111},
    {0b111, 0b100, 0b100, 0b111, 0b101, 0b101, 0b111},
    {0b111, 0b001, 0b001, 0b010, 0b010, 0b010, 0b010},
    {0b111, 0b101, 0b101, 0b111, 0b101, 0b101, 0b111},
    {0b111, 0b101, 0b101, 0b111, 0b001, 0b001, 0b111}
};
}

void BigClockRenderer::draw(DMD &dmd, const ClockState &clock)
{
    const uint8_t hour = clock.hour();
    const uint8_t minute = clock.minute();
    const uint8_t digits[] = {
        static_cast<uint8_t>(hour / 10),
        static_cast<uint8_t>(hour % 10),
        static_cast<uint8_t>(minute / 10),
        static_cast<uint8_t>(minute % 10)
    };

    TextRenderer::clearRegion(dmd, CLOCK_X, 0, CLOCK_WIDTH, SCREEN_HEIGHT);
    drawDigit(dmd, digits[0], 0, 1);
    drawDigit(dmd, digits[1], 7, 1);
    if (clock.colonOn) {
        drawColon(dmd, 14, 1);
    }
    drawDigit(dmd, digits[2], 18, 1);
    drawDigit(dmd, digits[3], 25, 1);
}

void BigClockRenderer::drawDigit(DMD &dmd, uint8_t digit, int x, int y)
{
    if (digit > 9) {
        return;
    }

    const int scale = 2;
    for (int row = 0; row < 7; row++) {
        for (int col = 0; col < 3; col++) {
            if ((bigDigits[digit][row] & (1 << (2 - col))) != 0) {
                drawPixelBlock(dmd, x + (col * scale), y + (row * scale), scale);
            }
        }
    }
}

void BigClockRenderer::drawColon(DMD &dmd, int x, int y)
{
    drawPixelBlock(dmd, x, y + 4, 2);
    drawPixelBlock(dmd, x, y + 10, 2);
}

void BigClockRenderer::drawPixelBlock(DMD &dmd, int x, int y, int scale)
{
    for (int dx = 0; dx < scale; dx++) {
        for (int dy = 0; dy < scale; dy++) {
            dmd.writePixel(x + dx, y + dy, GRAPHICS_NORMAL, true);
        }
    }
}
