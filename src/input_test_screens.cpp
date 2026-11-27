#include "input_test_screens.h"

#include <cmath>
#include <cstdio>

namespace {

using Line = char[ScreenBuffer::kMaxLineLength + 1];

int clampInt(int value, int low, int high) {
  if (value < low) return low;
  if (value > high) return high;
  return value;
}

// n / d rounded half away from zero, for d > 0.
int roundDiv(int n, int d) {
  return n >= 0 ? (n + d / 2) / d : -((-n + d / 2) / d);
}

// Hundredths of a g, clamped so the text stays four digits wide.
int hundredths(float g) {
  return clampInt(static_cast<int>(std::lround(g * 100.0f)), -999, 999);
}

// "+1.00" / "-0.25": sign always shown, two decimals.
void formatSignedG(float g, char *out, size_t capacity) {
  const int h = hundredths(g);
  const int magnitude = h < 0 ? -h : h;
  std::snprintf(out, capacity, "%c%d.%02d", h < 0 ? '-' : '+',
                magnitude / 100, magnitude % 100);
}

// "1.00": for values that are never negative.
void formatG(float g, char *out, size_t capacity) {
  const int h = clampInt(hundredths(g), 0, 999);
  std::snprintf(out, capacity, "%d.%02d", h / 100, h % 100);
}

int roundedDps(float dps) {
  return clampInt(static_cast<int>(std::lround(dps)), -999, 999);
}

}  // namespace

namespace InputTestScreens {

void renderStickTest(const StickTestReading &reading, ScreenBuffer *screen) {
  screen->clear();
  screen->setLine(0, "Stick Test");
  Line line;
  std::snprintf(line, sizeof(line), "X %+4d%%  raw %4d", reading.xPercent,
                reading.rawX);
  screen->setLine(1, line);
  std::snprintf(line, sizeof(line), "Y %+4d%%  raw %4d", reading.yPercent,
                reading.rawY);
  screen->setLine(2, line);

  constexpr int kCenterCol = kGridColumns / 2;
  constexpr int kCenterRow = kGridRows / 2;
  const int x = clampInt(reading.xPercent, -100, 100);
  const int y = clampInt(reading.yPercent, -100, 100);
  // + X is physically left, so it moves the dot toward column 0.
  const int col = kCenterCol - roundDiv(x * kCenterCol, 100);
  const int row = kCenterRow - roundDiv(y * kCenterRow, 100);

  for (int r = 0; r < kGridRows; ++r) {
    // Two spaces of margin, then each cell followed by a space.
    Line grid = "  ";
    int pos = 2;
    for (int c = 0; c < kGridColumns; ++c) {
      char cell = '.';
      if (r == kCenterRow && c == kCenterCol) cell = '+';
      if (r == row && c == col) cell = 'O';
      grid[pos++] = cell;
      if (c + 1 < kGridColumns) grid[pos++] = ' ';
    }
    grid[pos] = '\0';
    screen->setLine(3 + r, grid);
  }
}

void renderTriggerTest(const TriggerTestReading &reading,
                       ScreenBuffer *screen) {
  screen->clear();
  screen->setLine(0, "Trigger Test");
  Line line;
  const int percent = clampInt(reading.percent, 0, 100);
  std::snprintf(line, sizeof(line), "Pull %3d%%", percent);
  screen->setLine(1, line);
  std::snprintf(line, sizeof(line), "Raw  %4d", reading.raw);
  screen->setLine(2, line);

  const int filled = roundDiv(percent * kBarCells, 100);
  Line bar;
  int pos = 0;
  bar[pos++] = '[';
  for (int i = 0; i < kBarCells; ++i) bar[pos++] = i < filled ? '#' : '.';
  bar[pos++] = ']';
  bar[pos] = '\0';
  screen->setLine(4, bar);
  screen->setLine(7, "Bumper = exit");
}

void renderMotionTest(const MotionTestTracker &motion, ScreenBuffer *screen) {
  screen->clear();
  screen->setLine(0, "Motion Test");
  if (!motion.hasSample()) {
    screen->setLine(1, "Waiting for data...");
    screen->setLine(7, "Bumper = exit");
    return;
  }

  const ImuSample &s = motion.latest();
  Line line;
  char x[8], y[8], z[8];
  formatSignedG(s.accelG[0], x, sizeof(x));
  formatSignedG(s.accelG[1], y, sizeof(y));
  formatSignedG(s.accelG[2], z, sizeof(z));
  std::snprintf(line, sizeof(line), "X%s Y%s Z%s", x, y, z);
  screen->setLine(1, line);

  char total[8], peak[8];
  formatG(motion.totalG(), total, sizeof(total));
  formatG(motion.peakG(), peak, sizeof(peak));
  std::snprintf(line, sizeof(line), "Total %sg Max %sg", total, peak);
  screen->setLine(2, line);

  std::snprintf(line, sizeof(line), "Pitch %+4d Roll %+4d", imuPitchDeg(s),
                imuRollDeg(s));
  screen->setLine(3, line);

  std::snprintf(line, sizeof(line), "Yaw %+4d (relative)",
                static_cast<int>(std::lround(motion.yawDeg())));
  screen->setLine(4, line);

  std::snprintf(line, sizeof(line), "dps X%+d Y%+d Z%+d",
                roundedDps(s.gyroDps[0]), roundedDps(s.gyroDps[1]),
                roundedDps(s.gyroDps[2]));
  screen->setLine(5, line);

  screen->setLine(6, "Click = reset");
  screen->setLine(7, "Bumper = exit");
}

}  // namespace InputTestScreens
