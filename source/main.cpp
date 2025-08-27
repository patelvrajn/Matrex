#include <iostream>

#include "bishop_magic_bitboards.hpp"
#include "chess_board.hpp"
#include "leaper_attacks.hpp"
#include "rook_magic_bitboards.hpp"

/*******************************************************************************
    Only the entry point (main) function should be in this file because this
    file is excluded from testing.
*******************************************************************************/

Bitboard pawn_attacks[NUM_OF_PLAYERS][NUM_OF_SQUARES_ON_CHESS_BOARD]{};
Bitboard knight_attacks[NUM_OF_SQUARES_ON_CHESS_BOARD]{};
Bitboard king_attacks[NUM_OF_SQUARES_ON_CHESS_BOARD]{};

int main(void) {
  init_leaper_attacks();

  Bishop_Magic_Bitboards bmagic;
  Rook_Magic_Bitboards rmagic;

  Chess_Board cb;
  cb.set_from_fen(
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
  cb.pretty_print();

  return 0;
}
