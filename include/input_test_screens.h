#pragma once

#include "motion_test.h"
#include "screen.h"

// Text for the live Stick, Trigger and Motion Test menu screens
// (bring-up/support tools, like Button Test). Pure: each takes the
// readings to show and fills a ScreenBuffer.

struct StickTestReading {
  int rawX = 0;
  int rawY = 0;
  int xPercent = 0;  // calibrated, -100..100, + = physically left
  int yPercent = 0;  // calibrated, -100..100, + = up
};

struct TriggerTestReading {
  int raw = 0;
  int percent = 0;  // calibrated, 0..100
};

namespace InputTestScreens {

// Stick position grid: kGridColumns x kGridRows dots, '+' at center and
// 'O' at the stick's position, drawn the way the stick physically moves.
constexpr int kGridColumns = 9;
constexpr int kGridRows = 5;

// Trigger bar width in cells, between the [ ] ends.
constexpr int kBarCells = 19;

void renderStickTest(const StickTestReading &reading, ScreenBuffer *screen);
void renderTriggerTest(const TriggerTestReading &reading,
                       ScreenBuffer *screen);
// Shows "Waiting for data..." until the first IMU sample arrives (or for
// good, if the IMU isn't responding).
void renderMotionTest(const MotionTestTracker &motion, ScreenBuffer *screen);

}  // namespace InputTestScreens
