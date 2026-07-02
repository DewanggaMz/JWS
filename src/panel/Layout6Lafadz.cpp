#include "panel/Layout6Lafadz.h"

#include "panel/DisplayConfig.h"

namespace {
const uint32_t DEFAULT_HOLD_MS = 7000;

const char *const MUHAMMAD_BITMAP[] = {
  "............................",
  "....................###.....",
  "...................#...#....",
  "..###.......###....#...#....",
  ".#...#.....#...#....###.....",
  ".#...#.....#...#............",
  "..###..###..###....####.....",
  "......#...#.......#....#....",
  "......#...#.......#....#....",
  ".......###.........####.....",
  "...##........##........##...",
  "..#..#......#..#......#..#..",
  "..#..#......#..#......#..#..",
  "...##........##........##...",
  "............................",
  "............................"
};

const char *const ALLAH_BITMAP[] = {
  "........................",
  "...........##...........",
  "...........##.....##....",
  ".....##....##.....##....",
  ".....##....##.....##....",
  ".....##....##.....##....",
  ".....##....##.....##....",
  "...####..####...####....",
  "..#....##....#.#....#...",
  "..#....##....#.#....#...",
  "...####..####...####....",
  "...................##...",
  ".................###....",
  "...............###......",
  "........................",
  "........................"
};

template <size_t N>
uint8_t rowCount(const char *const (&)[N])
{
  return static_cast<uint8_t>(N);
}
}

Layout6Lafadz::Layout6Lafadz(DMD &display, uint32_t durationMs)
  : dmd(display),
    holdMs(durationMs == 0 ? DEFAULT_HOLD_MS : durationMs),
    startedAt(0),
    finished(false)
{
}

void Layout6Lafadz::begin()
{
  dmd.clearScreen(true);
  startedAt = millis();
  finished = false;
}

void Layout6Lafadz::update(const ClockState &clock)
{
  (void)clock;
  if (millis() - startedAt >= holdMs) {
    finished = true;
  }
}

void Layout6Lafadz::render(const ClockState &clock)
{
  (void)clock;
  dmd.clearScreen(true);
  drawMuhammad();
  drawAllah();
}

bool Layout6Lafadz::isFinished() const
{
  return finished;
}

void Layout6Lafadz::drawBitmap(
  int x,
  int y,
  const char *const rows[],
  uint8_t rowCount
)
{
  for (uint8_t row = 0; row < rowCount; row++) {
    const char *pixels = rows[row];
    for (uint8_t col = 0; pixels[col] != '\0'; col++) {
      if (pixels[col] != '.' && pixels[col] != ' ') {
        dmd.writePixel(x + col, y + row, GRAPHICS_NORMAL, true);
      }
    }
  }
}

void Layout6Lafadz::drawMuhammad()
{
  drawBitmap(2, 0, MUHAMMAD_BITMAP, rowCount(MUHAMMAD_BITMAP));
}

void Layout6Lafadz::drawAllah()
{
  const int bitmapWidth = 24;
  const int rightPanelX = SCREEN_WIDTH - PANEL_WIDTH;
  const int x = rightPanelX + ((PANEL_WIDTH - bitmapWidth) / 2);
  drawBitmap(x, 0, ALLAH_BITMAP, rowCount(ALLAH_BITMAP));
}
