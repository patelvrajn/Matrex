#pragma once

#include <array>

#include "directional_ray.hpp"
#include "bishop_magic_bitboards.hpp"
#include "leaper_attacks.hpp"
#include "rook_magic_bitboards.hpp"

struct Leaper_Attack_Tables
{
    Multi_Array<Bitboard, NUM_OF_PLAYERS, NUM_OF_SQUARES_ON_CHESS_BOARD> pawn;
    Bitboard_Array                                                       knight;
    Bitboard_Array                                                       king;
};

constexpr Leaper_Attack_Tables init_leaper_attacks()
{
    Leaper_Attack_Tables leaper_table;

    for (uint8_t square_idx = 0; square_idx < NUM_OF_SQUARES_ON_CHESS_BOARD;
         square_idx++)
    {
        const Square s = Square(square_idx);

        leaper_table.pawn[PIECE_COLOR::WHITE][square_idx] =
            mask_pawn_attacks(PIECE_COLOR::WHITE, s);
        leaper_table.pawn[PIECE_COLOR::BLACK][square_idx] =
            mask_pawn_attacks(PIECE_COLOR::BLACK, s);

        leaper_table.knight[square_idx] = mask_knight_attacks(s);

        leaper_table.king[square_idx] = mask_king_attacks(s);
    }

    return leaper_table;
}

constexpr uint8_t NUM_OF_DIAGONAL_DIRECTIONS   = 4;
constexpr uint8_t NUM_OF_ORTHOGONAL_DIRECTIONS = 4;

struct Slider_Ray_Tables
{
    Multi_Array<Bitboard,
                NUM_OF_SQUARES_ON_CHESS_BOARD,
                NUM_OF_DIAGONAL_DIRECTIONS>
        bishop;
    Multi_Array<Bitboard,
                NUM_OF_SQUARES_ON_CHESS_BOARD,
                NUM_OF_ORTHOGONAL_DIRECTIONS>
        rook;
};

constexpr Slider_Ray_Tables init_slider_rays()
{
    Slider_Ray_Tables slider_table;

    for (uint8_t square_idx = 0; square_idx < NUM_OF_SQUARES_ON_CHESS_BOARD;
         square_idx++)
    {
        const Square s = Square(square_idx);

        auto ray = Directional_Ray(s, NORTHWEST).get_ray();
        ray.unset_square(s);
        slider_table.bishop[square_idx][0] = ray;

        ray = Directional_Ray(s, NORTHEAST).get_ray();
        ray.unset_square(s);
        slider_table.bishop[square_idx][1] = ray;

        ray = Directional_Ray(s, SOUTHWEST).get_ray();
        ray.unset_square(s);
        slider_table.bishop[square_idx][2] = ray;

        ray = Directional_Ray(s, SOUTHEAST).get_ray();
        ray.unset_square(s);
        slider_table.bishop[square_idx][3] = ray;

        ray = Directional_Ray(s, NORTH).get_ray();
        ray.unset_square(s);
        slider_table.rook[square_idx][0] = ray;

        ray = Directional_Ray(s, SOUTH).get_ray();
        ray.unset_square(s);
        slider_table.rook[square_idx][1] = ray;

        ray = Directional_Ray(s, WEST).get_ray();
        ray.unset_square(s);
        slider_table.rook[square_idx][2] = ray;

        ray = Directional_Ray(s, EAST).get_ray();
        ray.unset_square(s);
        slider_table.rook[square_idx][3] = ray;
    }

    return slider_table;
}

using Directional_Ray_Table = Multi_Array<Directional_Ray,
                                          NUM_OF_SQUARES_ON_CHESS_BOARD,
                                          NUM_OF_SQUARES_ON_CHESS_BOARD>;

constexpr Directional_Ray_Table init_directional_rays_table()
{
    Directional_Ray_Table table;

    for (uint8_t outer_square_idx = 0;
         outer_square_idx < NUM_OF_SQUARES_ON_CHESS_BOARD;
         outer_square_idx++)
    {
        for (uint8_t inner_square_idx = 0;
             inner_square_idx < NUM_OF_SQUARES_ON_CHESS_BOARD;
             inner_square_idx++)
        {
            table[outer_square_idx][inner_square_idx] =
                Directional_Ray(Square(outer_square_idx),
                                Square(inner_square_idx));
        }
    }

    return table;
}

// =============================================================================
// Attacks Class
//
// A class wrapping the engine's attack-generation infrastructure such as the
// calculation of attacks and rays.
// =============================================================================
class Attacks
{
  public:

    constexpr Attacks();

    constexpr Bitboard get_attacks(const PIECES      p,
                                   const Square      s,
                                   const PIECE_COLOR c,
                                   const Bitboard    occupancy) const;

    constexpr Bitboard get_pawn_attacks(const Square      s,
                                        const PIECE_COLOR c) const;
    constexpr Bitboard get_knight_attacks(const Square s) const;
    constexpr Bitboard get_king_attacks(const Square s) const;

    constexpr Bitboard get_bishop_attacks(const Square   s,
                                          const Bitboard occupancy) const;
    constexpr Bitboard get_rook_attacks(const Square   s,
                                        const Bitboard occupancy) const;
    constexpr Bitboard get_queen_attacks(const Square   s,
                                         const Bitboard occupancy) const;

    // Caution; Direction in these functions is not the direction enum, it is
    // an index for the rays of specific directions in the slider table. See
    // init_slider_rays() above.
    constexpr Bitboard get_bishop_rays(const Square  s,
                                       const uint8_t direction) const;
    constexpr Bitboard get_rook_rays(const Square  s,
                                     const uint8_t direction) const;

    constexpr static const Directional_Ray& get_directional_ray(const Square a,
                                                                const Square b);

    constexpr static Bitboard get_infinite_ray(const Square a, const Square b);

  private:

    // Slider Attack Tables (constexpr to avoid the Static Initialization Order
    // Fiasco)
    static constexpr Bishop_Magic_Bitboards m_bishop_attack_tables =
        Bishop_Magic_Bitboards();
    static constexpr Rook_Magic_Bitboards m_rook_attack_tables =
        Rook_Magic_Bitboards();

    static constexpr Slider_Ray_Tables m_slider_table = init_slider_rays();

    static constexpr Leaper_Attack_Tables m_leaper_table =
        init_leaper_attacks();

    static constexpr Directional_Ray_Table m_directional_ray_table =
        init_directional_rays_table();
};

constexpr Attacks::Attacks() {}

constexpr Bitboard Attacks::get_attacks(const PIECES      p,
                                        const Square      s,
                                        const PIECE_COLOR c,
                                        const Bitboard    occupancy) const
{
    MATREX_ASSERT((p != PIECES::NO_PIECE),
                  "Attacks Class Assertion FAILURE: Attempted to get attacks "
                  "of no piece.");

    MATREX_ASSERT(s.has_square(),
                  "Attacks Class Assertion FAILURE: Attempted to get attacks "
                  "of no square.");

    MATREX_ASSERT((c != PIECE_COLOR::NO_COLOR),
                  "Attacks Class Assertion FAILURE: Attempted to get attacks "
                  "of no color.");

    Bitboard output;

    if (p == PIECES::PAWN) { output = get_pawn_attacks(s, c); }
    else if (p == PIECES::KNIGHT) { output = get_knight_attacks(s); }
    else if (p == PIECES::KING) { output = get_king_attacks(s); }
    else if (p == PIECES::BISHOP) { output = get_bishop_attacks(s, occupancy); }
    else if (p == PIECES::ROOK) { output = get_rook_attacks(s, occupancy); }
    else if (p == PIECES::QUEEN) { output = get_queen_attacks(s, occupancy); }

    return output;
}

constexpr Bitboard Attacks::get_pawn_attacks(const Square      s,
                                             const PIECE_COLOR c) const
{
    MATREX_ASSERT(
        s.has_square(),
        "Attacks Class Assertion FAILURE: Attempted to get pawn attacks "
        "of no square.");

    MATREX_ASSERT(
        (c != PIECE_COLOR::NO_COLOR),
        "Attacks Class Assertion FAILURE: Attempted to get pawn attacks "
        "of no color.");

    return m_leaper_table.pawn[c][s.get_index()];
}

constexpr Bitboard Attacks::get_knight_attacks(const Square s) const
{
    MATREX_ASSERT(
        s.has_square(),
        "Attacks Class Assertion FAILURE: Attempted to get knight attacks "
        "of no square.");

    return m_leaper_table.knight[s.get_index()];
}

constexpr Bitboard Attacks::get_king_attacks(const Square s) const
{
    MATREX_ASSERT(
        s.has_square(),
        "Attacks Class Assertion FAILURE: Attempted to get king attacks "
        "of no square.");

    return m_leaper_table.king[s.get_index()];
}

constexpr Bitboard Attacks::get_bishop_attacks(const Square   s,
                                               const Bitboard occupancy) const
{
    MATREX_ASSERT(
        s.has_square(),
        "Attacks Class Assertion FAILURE: Attempted to get bishop attacks "
        "of no square.");

    return m_bishop_attack_tables.get_attacks(s, occupancy);
}

constexpr Bitboard Attacks::get_rook_attacks(const Square   s,
                                             const Bitboard occupancy) const
{
    MATREX_ASSERT(
        s.has_square(),
        "Attacks Class Assertion FAILURE: Attempted to get rook attacks "
        "of no square.");

    return m_rook_attack_tables.get_attacks(s, occupancy);
}

constexpr Bitboard Attacks::get_queen_attacks(const Square   s,
                                              const Bitboard occupancy) const
{
    MATREX_ASSERT(
        s.has_square(),
        "Attacks Class Assertion FAILURE: Attempted to get queen attacks "
        "of no square.");

    return (this->get_bishop_attacks(s, occupancy)
            | this->get_rook_attacks(s, occupancy));
}

constexpr Bitboard Attacks::get_bishop_rays(const Square  s,
                                            const uint8_t direction) const
{
    MATREX_ASSERT(
        s.has_square(),
        "Attacks Class Assertion FAILURE: Attempted to get bishop rays "
        "of no square.");

    MATREX_ASSERT(
        (direction >= m_slider_table.bishop[s.get_index()].size),
        "Attacks Class Assertion FAILURE: Attempted to get bishop rays "
        "of a bad direction.");

    return m_slider_table.bishop[s.get_index()][direction];
}

constexpr Bitboard Attacks::get_rook_rays(const Square  s,
                                          const uint8_t direction) const
{
    MATREX_ASSERT(s.has_square(),
                  "Attacks Class Assertion FAILURE: Attempted to get rook rays "
                  "of no square.");

    MATREX_ASSERT((direction >= m_slider_table.rook[s.get_index()].size),
                  "Attacks Class Assertion FAILURE: Attempted to get rook rays "
                  "of a bad direction.");

    return m_slider_table.rook[s.get_index()][direction];
}

constexpr const Directional_Ray& Attacks::get_directional_ray(const Square a,
                                                              const Square b)
{
    MATREX_ASSERT(
        a.has_square(),
        "Attacks Class Assertion FAILURE: Attempted to get a directional ray "
        "between no square and another square.");

    MATREX_ASSERT(
        b.has_square(),
        "Attacks Class Assertion FAILURE: Attempted to get a directional ray "
        "between no square and another square.");

    return m_directional_ray_table[a.get_index()][b.get_index()];
}

constexpr Bitboard Attacks::get_infinite_ray(const Square a, const Square b)
{
    if (a.get_rank() == b.get_rank()) { return Bitboard::get_rank_mask(a); }

    if (a.get_file() == b.get_file()) { return Bitboard::get_file_mask(a); }

    if (a.get_diagonal() == b.get_diagonal())
    {
        return Bitboard::get_diagonal_mask(a);
    }

    if (a.get_antidiagonal() == b.get_antidiagonal())
    {
        return Bitboard::get_antidiagonal_mask(a);
    }

    return Bitboard();
}
