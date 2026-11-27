#include <unity.h>

#include "battery_level_filter.h"

void setUp(void) {}
void tearDown(void) {}

namespace {

// Raw ADC reading for a battery percent, the inverse of
// BatteryMonitor::percentExactFor (3.0-4.2 V, divide-by-3, 3.3 V / 4095).
// One percent is about 5 counts.
int rawFor(float percent) {
  const float batteryVolts = 3.0f + 1.2f * percent / 100.0f;
  const float pinVolts = batteryVolts / 3.0f;
  return static_cast<int>(pinVolts / 3.3f * 4095.0f + 0.5f);
}

constexpr unsigned long kTickMs = 5;  // loop_pacer's pass period

// Feeds `raw` every tick for durationMs starting at *nowMs; returns the
// last reported percent and advances *nowMs.
int hold(BatteryLevelFilter &filter, int raw, unsigned long durationMs,
         unsigned long *nowMs) {
  int percent = filter.percent();
  for (unsigned long t = 0; t < durationMs; t += kTickMs) {
    *nowMs += kTickMs;
    percent = filter.update(raw, *nowMs);
  }
  return percent;
}

}  // namespace

void test_percent_is_zero_before_first_update() {
  BatteryLevelFilter filter;
  TEST_ASSERT_EQUAL_INT(0, filter.percent());
}

void test_first_reading_is_reported_immediately() {
  BatteryLevelFilter filter;
  TEST_ASSERT_EQUAL_INT(50, filter.update(rawFor(50.0f), 1000));
  TEST_ASSERT_EQUAL_INT(50, filter.percent());
}

// The reported bug: a reading sitting near a rounding boundary flipped
// between two neighbouring percents.
void test_noise_across_a_rounding_boundary_does_not_flicker() {
  BatteryLevelFilter filter;
  unsigned long now = 0;
  filter.update(rawFor(57.5f), now);
  const int first = filter.percent();
  // +/- about 3% of ADC noise, alternating every tick, for a minute.
  for (int i = 0; i < 12000; ++i) {
    now += kTickMs;
    const int raw = rawFor(57.5f) + ((i % 2 == 0) ? 15 : -15);
    TEST_ASSERT_EQUAL_INT(first, filter.update(raw, now));
  }
}

void test_irregular_noise_holds_one_percent() {
  BatteryLevelFilter filter;
  unsigned long now = 0;
  filter.update(rawFor(57.0f), now);
  const int offsets[] = {12, -9, 3, -14, 7, 0, -4, 11, -12, 5};
  for (int i = 0; i < 12000; ++i) {
    now += kTickMs;
    TEST_ASSERT_EQUAL_INT(
        57, filter.update(rawFor(57.0f) + offsets[i % 10], now));
  }
}

void test_a_brief_transmit_sag_is_ignored() {
  BatteryLevelFilter filter;
  unsigned long now = 0;
  filter.update(rawFor(60.0f), now);
  // 50 ms dip of 10% (XBee TX burst), then back.
  TEST_ASSERT_EQUAL_INT(60, hold(filter, rawFor(50.0f), 50, &now));
  TEST_ASSERT_EQUAL_INT(60, hold(filter, rawFor(60.0f), 5000, &now));
}

void test_a_real_drop_is_followed() {
  BatteryLevelFilter filter;
  unsigned long now = 0;
  filter.update(rawFor(60.0f), now);
  TEST_ASSERT_EQUAL_INT(57, hold(filter, rawFor(57.0f), 30000, &now));
}

void test_a_real_rise_is_followed() {
  BatteryLevelFilter filter;
  unsigned long now = 0;
  filter.update(rawFor(40.0f), now);
  TEST_ASSERT_EQUAL_INT(45, hold(filter, rawFor(45.0f), 30000, &now));
}

void test_change_smaller_than_hysteresis_holds() {
  BatteryLevelFilter filter;
  unsigned long now = 0;
  filter.update(rawFor(57.0f), now);
  // About 0.65% away: past the 57/58 rounding boundary but inside the
  // hysteresis.
  TEST_ASSERT_EQUAL_INT(57, hold(filter, rawFor(57.7f), 60000, &now));
}

void test_change_past_the_hysteresis_moves() {
  BatteryLevelFilter filter;
  unsigned long now = 0;
  filter.update(rawFor(57.0f), now);
  TEST_ASSERT_EQUAL_INT(58, hold(filter, rawFor(58.2f), 60000, &now));
}

void test_large_step_is_smoothed_not_instant() {
  BatteryLevelFilter filter;
  unsigned long now = 0;
  filter.update(rawFor(80.0f), now);
  const int after = hold(filter, rawFor(20.0f), 500, &now);
  TEST_ASSERT_TRUE(after > 70);
  TEST_ASSERT_TRUE(after < 80);
}

void test_long_gap_catches_up_in_one_update() {
  BatteryLevelFilter filter;
  filter.update(rawFor(80.0f), 0);
  // A minute of light sleep between readings: the average is weighted by
  // elapsed time, so it lands within about 7% of the new value at once.
  const int percent = filter.update(rawFor(70.0f), 60000);
  TEST_ASSERT_TRUE(percent <= 71);
  TEST_ASSERT_TRUE(percent >= 70);
}

void test_same_timestamp_changes_nothing() {
  BatteryLevelFilter filter;
  filter.update(rawFor(50.0f), 1000);
  TEST_ASSERT_EQUAL_INT(50, filter.update(rawFor(10.0f), 1000));
  TEST_ASSERT_EQUAL_INT(50, filter.update(rawFor(10.0f), 1000));
}

void test_millis_wraparound_is_a_small_step() {
  BatteryLevelFilter filter;
  const unsigned long nearWrap = static_cast<unsigned long>(-1) - 2;
  filter.update(rawFor(80.0f), nearWrap);
  // 5 ms later, across the wrap: a tiny step, not a huge one.
  TEST_ASSERT_EQUAL_INT(80, filter.update(rawFor(10.0f), nearWrap + 5));
}

void test_reaches_full_and_empty() {
  BatteryLevelFilter filter;
  unsigned long now = 0;
  filter.update(rawFor(98.0f), now);
  TEST_ASSERT_EQUAL_INT(100, hold(filter, 4095, 60000, &now));
  TEST_ASSERT_EQUAL_INT(0, hold(filter, 0, 120000, &now));
}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(test_percent_is_zero_before_first_update);
  RUN_TEST(test_first_reading_is_reported_immediately);
  RUN_TEST(test_noise_across_a_rounding_boundary_does_not_flicker);
  RUN_TEST(test_irregular_noise_holds_one_percent);
  RUN_TEST(test_a_brief_transmit_sag_is_ignored);
  RUN_TEST(test_a_real_drop_is_followed);
  RUN_TEST(test_a_real_rise_is_followed);
  RUN_TEST(test_change_smaller_than_hysteresis_holds);
  RUN_TEST(test_change_past_the_hysteresis_moves);
  RUN_TEST(test_large_step_is_smoothed_not_instant);
  RUN_TEST(test_long_gap_catches_up_in_one_update);
  RUN_TEST(test_same_timestamp_changes_nothing);
  RUN_TEST(test_millis_wraparound_is_a_small_step);
  RUN_TEST(test_reaches_full_and_empty);
  return UNITY_END();
}
