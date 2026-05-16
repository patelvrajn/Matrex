#pragma once

#include "bitboard.hpp"

using Magics_Array = multi_array<uint64_t, NUM_OF_SQUARES_ON_CHESS_BOARD>;

struct Magic_Attacks_Info_Storage
{
    Bitboard mask {};
    uint8_t  num_of_idx_bits {};
};

using Magic_Attacks_Info_Array =
    multi_array<Magic_Attacks_Info_Storage, NUM_OF_SQUARES_ON_CHESS_BOARD>;

template <auto magics_gen_func,
          auto attacks_gen_func,
          auto attacks_info_gen_func>
class Magic_Hash_Jagged_Table
{
  public:

    constexpr Magic_Hash_Jagged_Table() = default;

    constexpr Bitboard get_attacks(const Square   s,
                                   const Bitboard occupancy) const;

  private:

    Magics_Array m_magics =
        magics_gen_func; // TODO: make magics function compile-time.
    static constexpr auto    m_attacks = attacks_gen_func();
    Magic_Attacks_Info_Array m_info    = attacks_info_gen_func();
};

template <auto magics_gen_func,
          auto attacks_gen_func,
          auto attacks_info_gen_func>
constexpr Bitboard
Magic_Hash_Jagged_Table<magics_gen_func,
                        attacks_gen_func,
                        attacks_info_gen_func>::get_attacks(const Square s,
                                                            const Bitboard
                                                                occupancy) const
{
    const uint8_t square_index = s.get_index();

    Bitboard blockers = occupancy & m_info[square_index].mask;

    uint64_t hash = blockers.get_board() * m_magics[square_index];

    uint64_t magic_index =
        hash >> ((sizeof(hash) << 3) - m_info[square_index].num_of_idx_bits);

    return m_attacks.get(square_index, magic_index);
}
