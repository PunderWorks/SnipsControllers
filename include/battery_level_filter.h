#pragma once

#include "battery.h"

// Turns noisy one-shot VSYS ADC readings into a steady battery percent.
// One percent is only about 5 ADC counts, less than the ESP32 ADC's
// read-to-read noise, so the raw percent flickers between neighbours.
// Two stages fix that:
//   1. An exponential moving average of the raw reading, with a time
//      constant in milliseconds (so it behaves the same at any loop rate,
//      and catches up at once after a long light sleep). This also hides
//      the brief sag while the XBee transmits.
//   2. Hysteresis on the reported whole percent: it only changes once the
//      smoothed value is kHysteresisPercent past the rounding boundary
//      (0.8% away from the shown value), so a value sitting on a boundary
//      holds. The margin is small on purpose: it bounds how far the shown
//      value can lag a settled reading, and the averaging has already
//      removed nearly all the noise.
// Pure logic: SnipsController.ino does the analogRead() and passes it in.
class BatteryLevelFilter {
 public:
  static constexpr unsigned long kTimeConstantMs = 5000;
  static constexpr float kHysteresisPercent = 0.3f;

  // Feed one raw ADC reading (0-4095) taken at nowMs, and get back the
  // percent (0-100) to show and report. The first reading primes the
  // filter and is reported as-is (rounded).
  int update(int rawAdc, unsigned long nowMs);

  // The last value update() returned (0 before the first update).
  int percent() const { return reported_; }

 private:
  BatteryMonitor monitor_;
  bool primed_ = false;
  float smoothedRaw_ = 0.0f;
  unsigned long lastMs_ = 0;
  int reported_ = 0;
};
