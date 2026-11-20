#include "serial_command.h"

#include <cctype>
#include <cstring>

SerialCommand SerialCommandReader::feed(char c) {
  if (c != '\r' && c != '\n') {
    if (length_ < kMaxLineLength) {
      line_[length_++] =
          static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    } else {
      overflowed_ = true;
    }
    return SerialCommand::kNone;
  }

  line_[length_] = '\0';
  const bool hang = !overflowed_ && std::strcmp(line_, "hang") == 0;
  length_ = 0;
  overflowed_ = false;
  return hang ? SerialCommand::kHang : SerialCommand::kNone;
}
