#include <unity.h>

#include "serial_command.h"

void setUp(void) {}
void tearDown(void) {}

namespace {

// Feeds `text` and returns the last non-kNone command, or kNone.
SerialCommand feedAll(SerialCommandReader *reader, const char *text) {
  SerialCommand last = SerialCommand::kNone;
  for (const char *p = text; *p != '\0'; ++p) {
    const SerialCommand command = reader->feed(*p);
    if (command != SerialCommand::kNone) last = command;
  }
  return last;
}

}  // namespace

void test_hang_line_returns_hang() {
  SerialCommandReader reader;
  TEST_ASSERT_TRUE(feedAll(&reader, "hang\n") == SerialCommand::kHang);
}

void test_command_only_fires_at_end_of_line() {
  SerialCommandReader reader;
  TEST_ASSERT_TRUE(feedAll(&reader, "hang") == SerialCommand::kNone);
  TEST_ASSERT_TRUE(reader.feed('\r') == SerialCommand::kHang);
}

void test_crlf_fires_once() {
  SerialCommandReader reader;
  TEST_ASSERT_TRUE(reader.feed('h') == SerialCommand::kNone);
  feedAll(&reader, "ang");
  TEST_ASSERT_TRUE(reader.feed('\r') == SerialCommand::kHang);
  TEST_ASSERT_TRUE(reader.feed('\n') == SerialCommand::kNone);
}

void test_matching_ignores_case() {
  SerialCommandReader reader;
  TEST_ASSERT_TRUE(feedAll(&reader, "HaNg\n") == SerialCommand::kHang);
}

void test_other_lines_are_ignored() {
  SerialCommandReader reader;
  TEST_ASSERT_TRUE(feedAll(&reader, "hangs\n") == SerialCommand::kNone);
  TEST_ASSERT_TRUE(feedAll(&reader, "han\n") == SerialCommand::kNone);
  TEST_ASSERT_TRUE(feedAll(&reader, "\n\n") == SerialCommand::kNone);
  // The reader recovers for the next line.
  TEST_ASSERT_TRUE(feedAll(&reader, "hang\n") == SerialCommand::kHang);
}

void test_overlong_line_is_dropped_whole() {
  SerialCommandReader reader;
  // Its first kMaxLineLength characters are not "hang", and its tail must
  // not be read as the start of a line.
  TEST_ASSERT_TRUE(feedAll(&reader, "xxxxxxxxxxxxxxxhang\n") ==
                   SerialCommand::kNone);
  TEST_ASSERT_TRUE(feedAll(&reader, "hangxxxxxxxxxxxxxxxx\n") ==
                   SerialCommand::kNone);
  TEST_ASSERT_TRUE(feedAll(&reader, "hang\n") == SerialCommand::kHang);
}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_hang_line_returns_hang);
  RUN_TEST(test_command_only_fires_at_end_of_line);
  RUN_TEST(test_crlf_fires_once);
  RUN_TEST(test_matching_ignores_case);
  RUN_TEST(test_other_lines_are_ignored);
  RUN_TEST(test_overlong_line_is_dropped_whole);
  return UNITY_END();
}
