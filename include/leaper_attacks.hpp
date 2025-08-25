#pragma once

#include "bitboard.hpp"
#include "globals.hpp"

extern Bitboard pawn_attacks[NUM_OF_PLAYERS][NUM_OF_SQUARES_ON_CHESS_BOARD];
extern Bitboard knight_attacks[NUM_OF_SQUARES_ON_CHESS_BOARD];
extern Bitboard king_attacks[NUM_OF_SQUARES_ON_CHESS_BOARD];

const Bitboard NOT_A_FILE(18374403900871474942ULL);
const Bitboard NOT_H_FILE(9187201950435737471ULL);
const Bitboard NOT_AB_FILES(18229723555195321596ULL);
const Bitboard NOT_GH_FILES(4557430888798830399ULL);

void init_leaper_attacks();

Bitboard mask_pawn_attacks(PIECE_COLOR c, const Square& s);
Bitboard mask_knight_attacks(const Square& s);
Bitboard mask_king_attacks(const Square& s);
