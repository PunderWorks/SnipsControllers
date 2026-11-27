#include <unity.h>

#include "input_test_screens.h"

void setUp(void) {}
void tearDown(void) {}

namespace {

StickTestReading stick(int x, int y) {
  StickTestReading r;
  r.rawX = 2048;
  r.rawY = 2048;
  r.xPercent = x;
  r.yPercent = y;
  return r;
}

// Row r of the stick grid (screen lines 3..7).
const char *gridRow(const ScreenBuffer &s, int r) { return s.line(3 + r); }

ImuSample imu(float ax, float ay, float az, float gx, float gy, float gz) {
  ImuSample s;
  s.accelG[0] = ax;
  s.accelG[1] = ay;
  s.accelG[2] = az;
  s.gyroDps[0] = gx;
  s.gyroDps[1] = gy;
  s.gyroDps[2] = gz;
  return s;
}

}  // namespace

// ---- Stick Test -------------------------------------------------------------

void test_stick_text_lines() {
  ScreenBuffer s;
  StickTestReading r;
  r.rawX = 1234;
  r.rawY = 987;
  r.xPercent = -45;
  r.yPercent = 100;
  InputTestScreens::renderStickTest(r, &s);
  TEST_ASSERT_EQUAL_STRING("Stick Test", s.line(0));
  TEST_ASSERT_EQUAL_STRING("X  -45%  raw 1234", s.line(1));
  TEST_ASSERT_EQUAL_STRING("Y +100%  raw  987", s.line(2));
}

void test_stick_centered_shows_dot_on_center_mark() {
  ScreenBuffer s;
  InputTestScreens::renderStickTest(stick(0, 0), &s);
  TEST_ASSERT_EQUAL_STRING("  . . . . . . . . .", gridRow(s, 0));
  TEST_ASSERT_EQUAL_STRING("  . . . . O . . . .", gridRow(s, 2));
  TEST_ASSERT_EQUAL_STRING("  . . . . . . . . .", gridRow(s, 4));
}

void test_stick_full_up_is_top_row() {
  ScreenBuffer s;
  InputTestScreens::renderStickTest(stick(0, 100), &s);
  TEST_ASSERT_EQUAL_STRING("  . . . . O . . . .", gridRow(s, 0));
  TEST_ASSERT_EQUAL_STRING("  . . . . + . . . .", gridRow(s, 2));
}

void test_stick_full_down_is_bottom_row() {
  ScreenBuffer s;
  InputTestScreens::renderStickTest(stick(0, -100), &s);
  TEST_ASSERT_EQUAL_STRING("  . . . . O . . . .", gridRow(s, 4));
}

// +X is physically left, so it draws on the left.
void test_stick_positive_x_is_drawn_left() {
  ScreenBuffer s;
  InputTestScreens::renderStickTest(stick(100, 0), &s);
  TEST_ASSERT_EQUAL_STRING("  O . . . + . . . .", gridRow(s, 2));
  InputTestScreens::renderStickTest(stick(-100, 0), &s);
  TEST_ASSERT_EQUAL_STRING("  . . . . + . . . O", gridRow(s, 2));
}

void test_stick_partial_push_rounds_to_nearest_cell() {
  ScreenBuffer s;
  // 40% right: 1.6 cells -> 2; 30% up: 0.6 rows -> 1.
  InputTestScreens::renderStickTest(stick(-40, 30), &s);
  TEST_ASSERT_EQUAL_STRING("  . . . . . . O . .", gridRow(s, 1));
  // 10% left: 0.4 cells -> stays on center.
  InputTestScreens::renderStickTest(stick(10, 0), &s);
  TEST_ASSERT_EQUAL_STRING("  . . . . O . . . .", gridRow(s, 2));
}

void test_stick_out_of_range_is_clamped_to_edge() {
  ScreenBuffer s;
  InputTestScreens::renderStickTest(stick(-250, 250), &s);
  TEST_ASSERT_EQUAL_STRING("  . . . . . . . . O", gridRow(s, 0));
}

// ---- Trigger Test -----------------------------------------------------------

void test_trigger_released() {
  ScreenBuffer s;
  TriggerTestReading r;
  r.raw = 812;
  r.percent = 0;
  InputTestScreens::renderTriggerTest(r, &s);
  TEST_ASSERT_EQUAL_STRING("Trigger Test", s.line(0));
  TEST_ASSERT_EQUAL_STRING("Pull   0%", s.line(1));
  TEST_ASSERT_EQUAL_STRING("Raw   812", s.line(2));
  TEST_ASSERT_EQUAL_STRING("[...................]", s.line(4));
  TEST_ASSERT_EQUAL_STRING("Bumper = exit", s.line(7));
}

void test_trigger_half_and_full() {
  ScreenBuffer s;
  TriggerTestReading r;
  r.raw = 2345;
  r.percent = 50;
  InputTestScreens::renderTriggerTest(r, &s);
  TEST_ASSERT_EQUAL_STRING("Pull  50%", s.line(1));
  TEST_ASSERT_EQUAL_STRING("[##########.........]", s.line(4));
  r.percent = 100;
  InputTestScreens::renderTriggerTest(r, &s);
  TEST_ASSERT_EQUAL_STRING("Pull 100%", s.line(1));
  TEST_ASSERT_EQUAL_STRING("[###################]", s.line(4));
}

void test_trigger_out_of_range_is_clamped() {
  ScreenBuffer s;
  TriggerTestReading r;
  r.percent = 140;
  InputTestScreens::renderTriggerTest(r, &s);
  TEST_ASSERT_EQUAL_STRING("Pull 100%", s.line(1));
  r.percent = -5;
  InputTestScreens::renderTriggerTest(r, &s);
  TEST_ASSERT_EQUAL_STRING("Pull   0%", s.line(1));
}

// ---- Motion Test ------------------------------------------------------------

void test_motion_waiting_before_first_sample() {
  ScreenBuffer s;
  MotionTestTracker t;
  InputTestScreens::renderMotionTest(t, &s);
  TEST_ASSERT_EQUAL_STRING("Motion Test", s.line(0));
  TEST_ASSERT_EQUAL_STRING("Waiting for data...", s.line(1));
  TEST_ASSERT_EQUAL_STRING("Bumper = exit", s.line(7));
}

void test_motion_flat_and_still() {
  ScreenBuffer s;
  MotionTestTracker t;
  t.update(imu(0.0f, 0.0f, 1.0f, 0, 0, 0), 0);
  InputTestScreens::renderMotionTest(t, &s);
  TEST_ASSERT_EQUAL_STRING("X+0.00 Y+0.00 Z+1.00", s.line(1));
  TEST_ASSERT_EQUAL_STRING("Total 1.00g Max 1.00g", s.line(2));
  TEST_ASSERT_EQUAL_STRING("Pitch   +0 Roll   +0", s.line(3));
  TEST_ASSERT_EQUAL_STRING("Yaw   +0 (relative)", s.line(4));
  TEST_ASSERT_EQUAL_STRING("dps X+0 Y+0 Z+0", s.line(5));
  TEST_ASSERT_EQUAL_STRING("Click = reset", s.line(6));
  TEST_ASSERT_EQUAL_STRING("Bumper = exit", s.line(7));
}

void test_motion_tilted_and_turning() {
  ScreenBuffer s;
  MotionTestTracker t;
  // Stick end raised 30 deg, turning +90 deg/s for one second.
  const ImuSample a = imu(0.0f, 0.5f, 0.866f, 12.4f, -3.6f, 90.0f);
  t.update(a, 0);
  for (unsigned long ms = 50; ms <= 1000; ms += 50) t.update(a, ms);
  InputTestScreens::renderMotionTest(t, &s);
  TEST_ASSERT_EQUAL_STRING("X+0.00 Y+0.50 Z+0.87", s.line(1));
  TEST_ASSERT_EQUAL_STRING("Pitch  +30 Roll   +0", s.line(3));
  TEST_ASSERT_EQUAL_STRING("Yaw  +90 (relative)", s.line(4));
  TEST_ASSERT_EQUAL_STRING("dps X+12 Y-4 Z+90", s.line(5));
}

void test_motion_shows_max_and_clamps_extremes() {
  ScreenBuffer s;
  MotionTestTracker t;
  t.update(imu(-0.25f, 0.0f, 1.0f, 0, 0, 0), 0);
  t.update(imu(0.0f, 12.0f, 0.0f, -2500, 2500, 0), 50);  // hard shake
  t.update(imu(-0.25f, 0.0f, 1.0f, 0, 0, 0), 100);
  InputTestScreens::renderMotionTest(t, &s);
  TEST_ASSERT_EQUAL_STRING("X-0.25 Y+0.00 Z+1.00", s.line(1));
  TEST_ASSERT_EQUAL_STRING("Total 1.03g Max 9.99g", s.line(2));

  t.update(imu(-12.0f, 12.0f, 12.0f, -2500, 2500, -2500), 150);
  InputTestScreens::renderMotionTest(t, &s);
  TEST_ASSERT_EQUAL_STRING("X-9.99 Y+9.99 Z+9.99", s.line(1));
  TEST_ASSERT_EQUAL_STRING("dps X-999 Y+999 Z-999", s.line(5));
}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(test_stick_text_lines);
  RUN_TEST(test_stick_centered_shows_dot_on_center_mark);
  RUN_TEST(test_stick_full_up_is_top_row);
  RUN_TEST(test_stick_full_down_is_bottom_row);
  RUN_TEST(test_stick_positive_x_is_drawn_left);
  RUN_TEST(test_stick_partial_push_rounds_to_nearest_cell);
  RUN_TEST(test_stick_out_of_range_is_clamped_to_edge);
  RUN_TEST(test_trigger_released);
  RUN_TEST(test_trigger_half_and_full);
  RUN_TEST(test_trigger_out_of_range_is_clamped);
  RUN_TEST(test_motion_waiting_before_first_sample);
  RUN_TEST(test_motion_flat_and_still);
  RUN_TEST(test_motion_tilted_and_turning);
  RUN_TEST(test_motion_shows_max_and_clamps_extremes);
  return UNITY_END();
}
