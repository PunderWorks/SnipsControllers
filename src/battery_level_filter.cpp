#include "battery_level_filter.h"

int BatteryLevelFilter::update(int rawAdc, unsigned long nowMs) {
  const float raw = static_cast<float>(rawAdc);
  if (!primed_) {
    primed_ = true;
    smoothedRaw_ = raw;
    lastMs_ = nowMs;
    reported_ = monitor_.percentFor(rawAdc);
    return reported_;
  }

  // Unsigned subtraction stays correct across millis() wraparound.
  const unsigned long elapsedMs = nowMs - lastMs_;
  lastMs_ = nowMs;
  const float alpha = static_cast<float>(elapsedMs) /
                      static_cast<float>(kTimeConstantMs + elapsedMs);
  smoothedRaw_ += alpha * (raw - smoothedRaw_);

  const float exact = monitor_.percentExactFor(smoothedRaw_);
  const float distance = exact - static_cast<float>(reported_);
  if (distance >= 0.5f + kHysteresisPercent ||
      distance <= -(0.5f + kHysteresisPercent)) {
    reported_ = static_cast<int>(exact + 0.5f);
  }
  return reported_;
}
