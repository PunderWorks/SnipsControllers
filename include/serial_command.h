#pragma once

#include <cstddef>

// Bring-up commands typed into the USB serial monitor, one per line. Pure:
// loop() feeds it whatever bytes arrived, and acts on what it returns.
enum class SerialCommand {
  kNone,
  // "hang": stop feeding the loop watchdog, to check that the watchdog
  // reboots the board. A hung V2 board can't be switched off (RXD0's
  // pull-up holds the power latch), so the watchdog is the only way out.
  kHang,
};

class SerialCommandReader {
 public:
  static constexpr size_t kMaxLineLength = 15;

  // Feeds one received byte. Returns the command when `c` ends a line
  // ('\r' or '\n') that names one; kNone otherwise. Matching ignores case.
  // A line longer than kMaxLineLength is dropped whole.
  SerialCommand feed(char c);

 private:
  char line_[kMaxLineLength + 1] = {};
  size_t length_ = 0;
  bool overflowed_ = false;
};
