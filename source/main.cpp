#include <iostream>

#include "attacks.hpp"
#include "chess_board.hpp"
#include "move_generator.hpp"
#include "perft.hpp"
#include "timer.hpp"

/*******************************************************************************
    Only the entry point (main) function should be in this file because this
    file is excluded from testing.
*******************************************************************************/

int main(void) {
  Attacks a;

  Chess_Board cb;

  cb.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  cb.pretty_print();

  constexpr uint64_t PERFT_DEPTH = 6;

  // Timer t;
  // uint64_t perft_count = perft(cb, PERFT_DEPTH);
  // uint64_t time_taken = t.stop();

  // std::cout << "Perft(" << PERFT_DEPTH << "): " << perft_count << std::endl;
  // std::cout << "Time taken (ns): " << time_taken << std::endl;
  // std::cout << "NPS: " << (((double)perft_count) / ((double)time_taken /
  // 1e9))
  //           << std::endl;

  divide_perft(cb, PERFT_DEPTH);

  return 0;
}
