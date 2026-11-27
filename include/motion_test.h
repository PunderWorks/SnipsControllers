#pragma once

#include "imu.h"

// State behind the Motion Test screen: the latest IMU sample, the peak
// total acceleration, and a yaw angle integrated from the gyro's Z rate.
// There's no magnetometer, so yaw is relative to where the controller
// pointed when the screen opened (or was last zeroed) and drifts slowly.
// It is the turn about the controller's own Z axis, so it only matches a
// compass heading while the controller is held roughly flat.
class MotionTestTracker {
 public:
  // A gap longer than this between samples (e.g. the gyro was just turned
  // on) is counted as this long, so a stale rate can't swing yaw far.
  static constexpr unsigned long kMaxStepMs = 200;

  // Clears everything: no sample, yaw 0, peak 0.
  void reset();

  // Feed a bias-corrected sample (imu.h's GyroBias::remove()).
  void update(const ImuSample &sample, unsigned long nowMs);

  bool hasSample() const { return hasSample_; }
  const ImuSample &latest() const { return latest_; }

  // Magnitude of the latest acceleration in g (1.0 at rest).
  float totalG() const;
  float peakG() const { return peakG_; }

  // -180..180 degrees, + counterclockwise seen from the top.
  float yawDeg() const { return yawDeg_; }

 private:
  bool hasSample_ = false;
  ImuSample latest_;
  unsigned long lastMs_ = 0;
  float peakG_ = 0.0f;
  float yawDeg_ = 0.0f;
};
