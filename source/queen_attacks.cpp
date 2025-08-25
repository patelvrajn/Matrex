#include "queen_attacks.hpp"

#include "bishop_magic_bitboards.hpp"
#include "rook_magic_bitboards.hpp"

Bitboard get_queen_attacks(const Square& s, const Bitboard& occupancy) {
  Bishop_Magic_Bitboards bmagic;
  Rook_Magic_Bitboards rmagic;

  return (bmagic.get_attacks(s, occupancy) | rmagic.get_attacks(s, occupancy));
}
