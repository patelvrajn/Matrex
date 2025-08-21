#include "magic_hash_table.hpp"

Magic_Hash_Table::Magic_Hash_Table()
    : m_magic(0),
      m_num_of_idx_bits(0),
      m_mask(Bitboard(0)),
      m_attacks(nullptr) {}

Magic_Hash_Table::Magic_Hash_Table(uint64_t magic, uint64_t num_of_idx_bits,
                                   Bitboard mask, Bitboard* attacks)
    : m_magic(magic),
      m_num_of_idx_bits(num_of_idx_bits),
      m_mask(mask),
      m_attacks(attacks) {}

Bitboard Magic_Hash_Table::get_attacks(const Bitboard& occupancy) const {
  Bitboard blockers = occupancy & m_mask;

  uint64_t hash = blockers.get_board() * m_magic;

  uint64_t magic_index = hash >> ((sizeof(hash) << 3) - m_num_of_idx_bits);

  return m_attacks[magic_index];
}
