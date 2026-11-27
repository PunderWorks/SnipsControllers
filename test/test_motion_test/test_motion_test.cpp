#include <unity.h>

#include "motion_test.h"

void setUp(void) {}
void tearDown(void) {}

namespace {
ImuSample sample(float ax, float ay, float az, float zDps) {
  ImuSample s;
  s.accelG[0] = ax;
  s.accelG[1] = ay;
  s.accelG[2] = az;
  s.gyroDps[2] = zDps;
  return s;
}
}  // namespace

void test_starts_empty() {
  MotionTestTracker t;
  TEST_ASSERT_FALSE(t.hasSample());
  TEST_ASSERT_EQUAL_FLOAT(0.0f, t.yawDeg());
  TEST_ASSERT_EQUAL_FLOAT(0.0f, t.peakG());
}

void test_first_sample_sets_reading_but_not_yaw() {
  MotionTestTracker t;
  t.update(sample(0, 0, 1, 90), 5000);
  TEST_ASSERT_TRUE(t.hasSample());
  TEST_ASSERT_EQUAL_FLOAT(0.0f, t.yawDeg());
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, t.totalG());
  TEST_ASSERT_EQUAL_FLOAT(90.0f, t.latest().gyroDps[2]);
}

void test_yaw_integrates_z_rate() {
  MotionTestTracker t;
  t.update(sample(0, 0, 1, 45), 0);
  for (unsigned long ms = 50; ms <= 2000; ms += 50) {
    t.update(sample(0, 0, 1, 45), ms);
  }
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 90.0f, t.yawDeg());
}

void test_yaw_turns_negative_clockwise() {
  MotionTestTracker t;
  t.update(sample(0, 0, 1, -30), 0);
  t.update(sample(0, 0, 1, -30), 100);
  TEST_ASSERT_FLOAT_WITHIN(0.01f, -3.0f, t.yawDeg());
}

void test_yaw_wraps_at_180() {
  MotionTestTracker t;
  t.update(sample(0, 0, 1, 900), 0);
  t.update(sample(0, 0, 1, 900), 200);  // +180: stays at 180
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 180.0f, t.yawDeg());
  t.update(sample(0, 0, 1, 900), 300);  // +90 more: 270 -> -90
  TEST_ASSERT_FLOAT_WITHIN(0.01f, -90.0f, t.yawDeg());
  for (unsigned long ms = 400; ms <= 900; ms += 100) {
    t.update(sample(0, 0, 1, -900), ms);  // -540: -630 -> +90
  }
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 90.0f, t.yawDeg());
}

void test_long_gap_is_capped() {
  MotionTestTracker t;
  t.update(sample(0, 0, 1, 100), 0);
  t.update(sample(0, 0, 1, 100), 10000);  // counted as 200 ms
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 20.0f, t.yawDeg());
}

void test_peak_holds_the_largest_total() {
  MotionTestTracker t;
  t.update(sample(0, 0, 1, 0), 0);
  t.update(sample(0, 3, 4, 0), 50);
  t.update(sample(0, 0, 1, 0), 100);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, t.totalG());
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 5.0f, t.peakG());
}

void test_reset_clears_everything() {
  MotionTestTracker t;
  t.update(sample(0, 3, 4, 50), 0);
  t.update(sample(0, 3, 4, 50), 100);
  t.reset();
  TEST_ASSERT_FALSE(t.hasSample());
  TEST_ASSERT_EQUAL_FLOAT(0.0f, t.yawDeg());
  TEST_ASSERT_EQUAL_FLOAT(0.0f, t.peakG());
  // The next sample is a fresh start, not a step from the old time.
  t.update(sample(0, 0, 1, 50), 5000);
  TEST_ASSERT_EQUAL_FLOAT(0.0f, t.yawDeg());
}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(test_starts_empty);
  RUN_TEST(test_first_sample_sets_reading_but_not_yaw);
  RUN_TEST(test_yaw_integrates_z_rate);
  RUN_TEST(test_yaw_turns_negative_clockwise);
  RUN_TEST(test_yaw_wraps_at_180);
  RUN_TEST(test_long_gap_is_capped);
  RUN_TEST(test_peak_holds_the_largest_total);
  RUN_TEST(test_reset_clears_everything);
  return UNITY_END();
}
