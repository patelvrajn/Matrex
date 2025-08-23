#pragma once

#include <array>

#include "globals.hpp"
#include "magic_hash_table.hpp"

class Bishop_Magic_Bitboards {
 public:
  Bishop_Magic_Bitboards();

  Bitboard get_attacks(const Square& s, Bitboard occupancy) const;

 private:
  static bool m_is_attack_tables_initialized;
  static bool m_is_magics_initialized;

  static std::array<Magic_Hash_Table, NUM_OF_SQUARES_ON_CHESS_BOARD>
      m_attack_hash_tables;

  static std::array<uint64_t, NUM_OF_SQUARES_ON_CHESS_BOARD> m_magics;

  void init_attack_hash_tables();

  void init_magics();

  Bitboard mask_bishop_attacks(const Square& s) const;

  Bitboard calculate_bishop_attacks(const Square& s,
                                    const Bitboard& blockers) const;
};
