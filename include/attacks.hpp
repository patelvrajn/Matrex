#pragma once

#include <array>

#include "bishop_magic_bitboards.hpp"
#include "leaper_attacks.hpp"
#include "rook_magic_bitboards.hpp"

struct Leaper_Attack_Tables
{
    multi_array<Bitboard, NUM_OF_PLAYERS, NUM_OF_SQUARES_ON_CHESS_BOARD> pawn;
    Bitboard_Array                                                       knight;
    Bitboard_Array                                                       king;
};

constexpr uint8_t NUM_OF_BISHOP_RAY_DIRECTIONS = 4;
constexpr uint8_t NUM_OF_ROOK_RAY_DIRECTIONS   = 4;

struct Slider_Ray_Tables
{
    multi_array<Bitboard,
                NUM_OF_SQUARES_ON_CHESS_BOARD,
                NUM_OF_BISHOP_RAY_DIRECTIONS>
        bishop;
    multi_array<Bitboard,
                NUM_OF_SQUARES_ON_CHESS_BOARD,
                NUM_OF_ROOK_RAY_DIRECTIONS>
        rook;
};

consteval Leaper_Attack_Tables init_leaper_attacks()
{
    Leaper_Attack_Tables leaper_table;

    for (uint8_t square_idx = 0; square_idx < NUM_OF_SQUARES_ON_CHESS_BOARD;
         square_idx++)
    {
        leaper_table.pawn[PIECE_COLOR::WHITE][square_idx] =
            mask_pawn_attacks(PIECE_COLOR::WHITE, Square(square_idx));
        leaper_table.pawn[PIECE_COLOR::BLACK][square_idx] =
            mask_pawn_attacks(PIECE_COLOR::BLACK, Square(square_idx));

        leaper_table.knight[square_idx] =
            mask_knight_attacks(Square(square_idx));

        leaper_table.king[square_idx] = mask_king_attacks(Square(square_idx));
    }

    return leaper_table;
}

constexpr Bitboard generate_slider_rays_on_square(Square s,
                                                  int8_t rank_direction,
                                                  int8_t file_direction)
{
    Bitboard ray;

    int8_t f = s.get_file() + file_direction;
    int8_t r = s.get_rank() + rank_direction;

    while (f >= 0 && f < NUM_OF_FILES_ON_CHESS_BOARD && r >= 0
           && r < NUM_OF_RANKS_ON_CHESS_BOARD)
    {
        ray.set_square(Square(r, f));
        f += file_direction;
        r += rank_direction;
    }

    return ray;
}

consteval Slider_Ray_Tables init_slider_rays()
{
    Slider_Ray_Tables slider_table;

    for (uint8_t square_idx = 0; square_idx < NUM_OF_SQUARES_ON_CHESS_BOARD;
         square_idx++)
    {
        slider_table.bishop[square_idx][0] =
            generate_slider_rays_on_square(Square(square_idx), 1, 1);
        slider_table.bishop[square_idx][1] =
            generate_slider_rays_on_square(Square(square_idx), -1, -1);
        slider_table.bishop[square_idx][2] =
            generate_slider_rays_on_square(Square(square_idx), -1, 1);
        slider_table.bishop[square_idx][3] =
            generate_slider_rays_on_square(Square(square_idx), 1, -1);

        slider_table.rook[square_idx][0] =
            generate_slider_rays_on_square(Square(square_idx), 1, 0);
        slider_table.rook[square_idx][1] =
            generate_slider_rays_on_square(Square(square_idx), -1, 0);
        slider_table.rook[square_idx][2] =
            generate_slider_rays_on_square(Square(square_idx), 0, 1);
        slider_table.rook[square_idx][3] =
            generate_slider_rays_on_square(Square(square_idx), 0, -1);
    }

    return slider_table;
}

class Attacks
{
  public:

    constexpr Attacks();

    constexpr Bitboard get_attacks(const PIECES      p,
                                   const Square      s,
                                   const PIECE_COLOR c,
                                   const Bitboard    occupancy);

    constexpr Bitboard get_pawn_attacks(const Square& s, PIECE_COLOR c) const;
    constexpr Bitboard get_knight_attacks(const Square& s) const;
    constexpr Bitboard get_king_attacks(const Square& s) const;

    constexpr Bitboard get_bishop_attacks(const Square&   s,
                                          const Bitboard& occupancy);
    constexpr Bitboard get_rook_attacks(const Square&   s,
                                        const Bitboard& occupancy);
    constexpr Bitboard get_queen_attacks(const Square&   s,
                                         const Bitboard& occupancy);

    constexpr Bitboard get_bishop_rays(const Square& s,
                                       uint8_t       direction) const;
    constexpr Bitboard get_rook_rays(const Square& s, uint8_t direction) const;

  private:

    // Slider Attack Tables (Construct on First Use Idiom to avoid the Static
    // Initialization Order Fiasco)
    const inline static constinit Bishop_Magic_Bitboards
        m_bishop_attack_tables = Bishop_Magic_Bitboards();
    const inline static constinit Rook_Magic_Bitboards m_rook_attack_tables =
        Rook_Magic_Bitboards();

    // Slider Rays Table
    static constexpr Slider_Ray_Tables m_slider_table = init_slider_rays();

    // Leaper Attack Tables
    static constexpr Leaper_Attack_Tables m_leaper_table =
        init_leaper_attacks();
};

constexpr Attacks::Attacks() {}

constexpr Bitboard Attacks::get_attacks(const PIECES      p,
                                        const Square      s,
                                        const PIECE_COLOR c,
                                        const Bitboard    occupancy)
{
    Bitboard output;

    if (p == PIECES::PAWN) { output = get_pawn_attacks(s, c); }
    else if (p == PIECES::KNIGHT) { output = get_knight_attacks(s); }
    else if (p == PIECES::KING) { output = get_king_attacks(s); }
    else if (p == PIECES::BISHOP) { output = get_bishop_attacks(s, occupancy); }
    else if (p == PIECES::ROOK) { output = get_rook_attacks(s, occupancy); }
    else if (p == PIECES::QUEEN) { output = get_queen_attacks(s, occupancy); }

    return output;
}

constexpr Bitboard Attacks::get_pawn_attacks(const Square& s,
                                             PIECE_COLOR   c) const
{
    return m_leaper_table.pawn[c][s.get_index()];
}

constexpr Bitboard Attacks::get_knight_attacks(const Square& s) const
{
    return m_leaper_table.knight[s.get_index()];
}

constexpr Bitboard Attacks::get_king_attacks(const Square& s) const
{
    return m_leaper_table.king[s.get_index()];
}

constexpr Bitboard Attacks::get_bishop_attacks(const Square&   s,
                                               const Bitboard& occupancy)
{
    return m_bishop_attack_tables.get_attacks(s, occupancy);
}

constexpr Bitboard Attacks::get_rook_attacks(const Square&   s,
                                             const Bitboard& occupancy)
{
    return m_rook_attack_tables.get_attacks(s, occupancy);
}

constexpr Bitboard Attacks::get_bishop_rays(const Square& s,
                                            uint8_t       direction) const
{
    return m_slider_table.bishop[s.get_index()][direction];
}

constexpr Bitboard Attacks::get_rook_rays(const Square& s,
                                          uint8_t       direction) const
{
    return m_slider_table.rook[s.get_index()][direction];
}

constexpr Bitboard Attacks::get_queen_attacks(const Square&   s,
                                              const Bitboard& occupancy)
{
    return (this->get_bishop_attacks(s, occupancy)
            | this->get_rook_attacks(s, occupancy));
}
