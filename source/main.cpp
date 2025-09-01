#include <iostream>

#include "attacks.hpp"
#include "chess_board.hpp"
#include "move_generator.hpp"

/*******************************************************************************
    Only the entry point (main) function should be in this file because this
    file is excluded from testing.
*******************************************************************************/

int main(void) {
  Attacks a;

  Chess_Board cb;
  // cb.set_from_fen(
  //     "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0
  //     1");
  // cb.set_from_fen("5k2/8/6b1/8/4N3/3K4/8/8 w - - 0 1");
  // cb.set_from_fen("3k4/8/b2r4/8/2NN4/3KN3/8/8 w - - 0 1");
  // cb.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0
  // 1"); // Standard position
  // cb.set_from_fen("PPPPPPPP/8/8/1k6/6K1/8/8/pppppppp w HAha - 0 1"); // Pawn
  // promotions
  // cb.set_from_fen("8/PPPPPPPP/8/1k6/6K1/8/pppppppp/8 w HAha - 0 1");
  // cb.set_from_fen("8/8/8/3K4/2NN4/8/b7/3r3k w - - 0 1");
  cb.set_from_fen("8/3k4/8/8/8/8/3K4/8 w - - 0 1");
  cb.pretty_print();

  std::cout << "Number of bytes of Chess_Move: " << sizeof(Chess_Move)
            << std::endl;

  std::vector<Chess_Move> move_list;

  Move_Generator mg(cb);
  mg.generate_all_moves(move_list);

  for (Chess_Move m : move_list) {
    m.pretty_print();
  }

  return 0;
}
