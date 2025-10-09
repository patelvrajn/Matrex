#include "gtest/gtest.h"
#include "tests/epd.hpp"

TEST(move_generator_tests, standard_perft_test_suite) {
  epd_perft_test("assets/standard_perft_suite.epd");
}

TEST(move_generator_tests, frc_perft_test_suite) {
  epd_perft_test("assets/frc_perft_suite.epd");
}
