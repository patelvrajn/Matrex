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

constexpr Leaper_Attack_Tables init_leaper_attacks()
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

constexpr Slider_Ray_Tables init_slider_rays()
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

enum DIRECTION : int8_t
{
    NORTHWEST    = -9,
    NORTH        = -8,
    NORTHEAST    = -7,
    WEST         = -1,
    NO_DIRECTION = 0,
    EAST         = 1,
    SOUTHWEST    = 7,
    SOUTH        = 8,
    SOUTHEAST    = 9
};

struct Directional_Ray
{
    Square    start_square = Square(ESQUARE::NO_SQUARE);
    Bitboard  ray          = 0;
    DIRECTION direction    = NO_DIRECTION;

    constexpr bool is_diagonal() const
    {
        return ((direction == NORTHWEST) || (direction == NORTHEAST)
                || (direction == SOUTHWEST) || (direction == SOUTHEAST));
    }

    constexpr bool is_orthogonal() const
    {
        return ((direction == NORTH) || (direction == SOUTH)
                || (direction == EAST) || (direction == WEST));
    }

    constexpr bool is_increasing_square_index() const
    {
        return (direction > 0);
    }

    constexpr bool is_decreasing_square_index() const
    {
        return (direction < 0);
    }

    template <bool start_from_start_square>
    constexpr Square travel_occupied_ray(uint8_t index, Bitboard occupied) const
    {
        Bitboard occupied_ray   = ray & occupied;
        uint8_t  occupied_count = 0;

        // Starts from start square.
        if constexpr (start_from_start_square)
        {
            if (is_increasing_square_index())
            {
                for (const Square& square :
                     Bitboard_Iterable<LSB_TO_MSB>(occupied_ray))
                {
                    if (occupied_count == index) { return square; }

                    ++occupied_count;
                }
            }

            if (is_decreasing_square_index())
            {
                for (const Square& square :
                     Bitboard_Iterable<MSB_TO_LSB>(occupied_ray))
                {
                    if (occupied_count == index) { return square; }

                    ++occupied_count;
                }
            }
        }

        // Starts from end of ray.
        if constexpr (!start_from_start_square)
        {
            if (is_increasing_square_index())
            {
                for (const Square& square :
                     Bitboard_Iterable<MSB_TO_LSB>(occupied_ray))
                {
                    if (occupied_count == index) { return square; }

                    ++occupied_count;
                }
            }

            if (is_decreasing_square_index())
            {
                for (const Square& square :
                     Bitboard_Iterable<LSB_TO_MSB>(occupied_ray))
                {
                    if (occupied_count == index) { return square; }

                    ++occupied_count;
                }
            }
        }

        return Square(ESQUARE::NO_SQUARE);
    }
};

using Directional_Ray_Table = multi_array<Directional_Ray,
                                          NUM_OF_SQUARES_ON_CHESS_BOARD,
                                          NUM_OF_SQUARES_ON_CHESS_BOARD>;

constexpr Directional_Ray generate_directional_ray(const Square a,
                                                   const Square b)
{
    Bitboard  mask  = 0;
    DIRECTION delta = NO_DIRECTION;

    // There are no squares in between a and b if they are the same square.
    if (a == b) { return Directional_Ray(); }

    const int8_t distance = b.get_index() - a.get_index();

    // Squares a and b are on the same file.
    if (a.get_file() == b.get_file())
    {
        delta = ((distance > 0) ? SOUTH : NORTH); // Move along the file.
    }
    // Squares a and b are on the same rank.
    else if (b.get_rank() == a.get_rank())
    {
        delta = ((distance > 0) ? EAST : WEST); // Move along the rank.
    }
    // Squares a and b are on a diagonal.
    else if ((distance % 9) == 0)
    {
        delta = ((distance > 0) ? SOUTHEAST
                                : NORTHWEST); // Move along the diagonal.
    }
    // Squares a and b are on another diagonal.
    else if ((distance % 7) == 0)
    {
        delta = ((distance > 0) ? SOUTHWEST
                                : NORTHEAST); // Move along the diagonal.
    }
    // No diagonal or orthogonal path between the squares.
    else
    {
        return Directional_Ray();
    }

    // Starting from square a travel in the direction of square b until you
    // cannot travel any further.
    int square_index = a.get_index();
    while ((Square(square_index).get_rank() < NUM_OF_RANKS_ON_CHESS_BOARD)
           && (Square(square_index).get_file() < NUM_OF_FILES_ON_CHESS_BOARD))
    {
        mask.set_square(Square(square_index));
        square_index += delta;
    }

    return {.start_square = a, .ray = mask, .direction = delta};
}

constexpr Directional_Ray_Table init_between_directional_rays_table()
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
                generate_directional_ray(Square(outer_square_idx),
                                         Square(inner_square_idx));
        }
    }

    return table;
}

class Attacks
{
  public:

    constexpr Attacks();

    constexpr Bitboard get_attacks(const PIECES      p,
                                   const Square      s,
                                   const PIECE_COLOR c,
                                   const Bitboard    occupancy) const;

    constexpr Bitboard get_pawn_attacks(const Square& s, PIECE_COLOR c) const;
    constexpr Bitboard get_knight_attacks(const Square& s) const;
    constexpr Bitboard get_king_attacks(const Square& s) const;

    constexpr Bitboard get_bishop_attacks(const Square&   s,
                                          const Bitboard& occupancy) const;
    constexpr Bitboard get_rook_attacks(const Square&   s,
                                        const Bitboard& occupancy) const;
    constexpr Bitboard get_queen_attacks(const Square&   s,
                                         const Bitboard& occupancy) const;

    constexpr Bitboard get_bishop_rays(const Square& s,
                                       uint8_t       direction) const;
    constexpr Bitboard get_rook_rays(const Square& s, uint8_t direction) const;

    constexpr static const Directional_Ray& get_directional_ray(const Square a,
                                                                const Square b);

  private:

    // Slider Attack Tables (constexpr to avoid the Static Initialization Order
    // Fiasco)
    const inline static constexpr Bishop_Magic_Bitboards
        m_bishop_attack_tables = Bishop_Magic_Bitboards();
    const inline static constexpr Rook_Magic_Bitboards m_rook_attack_tables =
        Rook_Magic_Bitboards();

    // Slider Rays Table
    static constexpr Slider_Ray_Tables m_slider_table = init_slider_rays();

    // Leaper Attack Tables
    static constexpr Leaper_Attack_Tables m_leaper_table =
        init_leaper_attacks();

    // Directional Ray Table
    inline static constexpr Directional_Ray_Table m_directional_ray_table =
        init_between_directional_rays_table();
};

constexpr Attacks::Attacks() {}

constexpr Bitboard Attacks::get_attacks(const PIECES      p,
                                        const Square      s,
                                        const PIECE_COLOR c,
                                        const Bitboard    occupancy) const
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
                                               const Bitboard& occupancy) const
{
    return m_bishop_attack_tables.get_attacks(s, occupancy);
}

constexpr Bitboard Attacks::get_rook_attacks(const Square&   s,
                                             const Bitboard& occupancy) const
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

constexpr const Directional_Ray& Attacks::get_directional_ray(const Square a,
                                                              const Square b)
{
    return m_directional_ray_table[a.get_index()][b.get_index()];
}

constexpr Bitboard Attacks::get_queen_attacks(const Square&   s,
                                              const Bitboard& occupancy) const
{
    return (this->get_bishop_attacks(s, occupancy)
            | this->get_rook_attacks(s, occupancy));
}
