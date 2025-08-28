#include <iostream>

#include "attacks.hpp"
#include "chess_board.hpp"

/*******************************************************************************
    Only the entry point (main) function should be in this file because this
    file is excluded from testing.
*******************************************************************************/

int main(void) {
  Attacks a;

  Chess_Board cb;
  cb.set_from_fen(
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
  cb.pretty_print();

  return 0;
}
