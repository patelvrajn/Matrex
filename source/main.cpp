#include <iostream>

#include "bishop_magic_bitboards.hpp"
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

  return 0;
}
