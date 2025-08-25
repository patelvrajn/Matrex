#include "bitboard.hpp"
#include "globals.hpp"
#include "gtest/gtest.h"
#include "hello_world.hpp"

// Initialized here to avoid undefined reference on compilation.
Bitboard pawn_attacks[NUM_OF_PLAYERS][NUM_OF_SQUARES_ON_CHESS_BOARD]{};
Bitboard knight_attacks[NUM_OF_SQUARES_ON_CHESS_BOARD]{};
Bitboard king_attacks[NUM_OF_SQUARES_ON_CHESS_BOARD]{};

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  std::cout << "RUNNING TESTS ..." << std::endl;
  int ret{RUN_ALL_TESTS()};
  if (!ret) {
    std::cout << "SUCCESS!!!" << std::endl;
    return 0;
  } else {
    std::cout << "FAILED." << std::endl;
    return 1;
  }
}

TEST(hello_world_test, test_one) { ASSERT_EQ(print_hello_world(), 0); }
