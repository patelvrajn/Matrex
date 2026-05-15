#pragma once

#include "bitboard.hpp"

using Magics_Array = multi_array<uint64_t, NUM_OF_SQUARES_ON_CHESS_BOARD>;

template <auto attacks_generation_function, auto magics>
class Magic_Hash_Jagged_Table
{
  public:

    constexpr Magic_Hash_Jagged_Table() = default;

    constexpr Bitboard get_attacks(const Square   s,
                                   const Bitboard occupancy) const;

  private:

    Magics_Array                                         m_magics = magics;
    multi_array<uint8_t, NUM_OF_SQUARES_ON_CHESS_BOARD>  m_num_of_idx_bits;
    multi_array<Bitboard, NUM_OF_SQUARES_ON_CHESS_BOARD> m_masks;
    static constexpr auto m_attacks = attacks_generation_function();
};

template <auto attacks_generation_function, auto magics>
constexpr Bitboard
Magic_Hash_Jagged_Table<attacks_generation_function, magics>::get_attacks(
    const Square   s,
    const Bitboard occupancy) const
{
    const uint8_t square_index = s.get_index();

    Bitboard blockers = occupancy & m_masks[square_index];

    uint64_t hash = blockers.get_board() * m_magics[square_index];

    uint64_t magic_index =
        hash >> ((sizeof(hash) << 3) - m_num_of_idx_bits[square_index]);

    return m_attacks.get(square_index, magic_index);
}
