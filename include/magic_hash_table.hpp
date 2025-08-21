#pragma once

#include "bitboard.hpp"

class Magic_Hash_Table {
 public:
  Magic_Hash_Table();

  Magic_Hash_Table(uint64_t magic, uint64_t num_of_idx_bits, Bitboard mask,
                   Bitboard* attacks);

  Bitboard get_attacks(const Bitboard& occupancy) const;

 private:
  uint64_t m_magic;
  uint64_t m_num_of_idx_bits;
  Bitboard m_mask;
  Bitboard* m_attacks;
};
