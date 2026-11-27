#include "motion_test.h"

#include <cmath>

void MotionTestTracker::reset() { *this = MotionTestTracker(); }

void MotionTestTracker::update(const ImuSample &sample, unsigned long nowMs) {
  if (hasSample_) {
    unsigned long stepMs = nowMs - lastMs_;
    if (stepMs > kMaxStepMs) stepMs = kMaxStepMs;
    yawDeg_ += sample.gyroDps[2] * static_cast<float>(stepMs) / 1000.0f;
    while (yawDeg_ > 180.0f) yawDeg_ -= 360.0f;
    while (yawDeg_ <= -180.0f) yawDeg_ += 360.0f;
  }
  hasSample_ = true;
  lastMs_ = nowMs;
  latest_ = sample;
  const float total = totalG();
  if (total > peakG_) peakG_ = total;
}

float MotionTestTracker::totalG() const {
  const float *a = latest_.accelG;
  return std::sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
}
