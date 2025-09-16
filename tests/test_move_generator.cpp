#include "gtest/gtest.h"
#include "move_generator.hpp"
#include "perft.hpp"

TEST(move_generator_tests, standard_position) {
  Chess_Board cb;
  cb.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

  ASSERT_EQ(perft(cb, 1), 20);
  ASSERT_EQ(perft(cb, 2), 400);
  ASSERT_EQ(perft(cb, 3), 8902);
  ASSERT_EQ(perft(cb, 4), 197281);
  ASSERT_EQ(perft(cb, 5), 4865609);
  ASSERT_EQ(perft(cb, 6), 119060324);
}
