#pragma once

#include <cstdint>
#include "bitboard.hpp"

// =============================================================================
// Direction Enumeration
//
// An enumeration describing directions (diagonal or orthogonal) in a chess
// board. The directions are assigned values based on the number of bits in a
// bitboard one needs to incrementally change the bit index (i.e. square index)
// by to go in that direction on the board.
// =============================================================================
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

// =============================================================================
// Directional Ray
//
// An abstraction of what a graphical vector is.
// =============================================================================
class Directional_Ray
{
  public:

    constexpr Directional_Ray();
    constexpr Directional_Ray(const Square src, const Square dst);
    constexpr Directional_Ray(const Square src, const DIRECTION d);

    constexpr Square    get_start_square() const;
    constexpr Bitboard  get_ray() const;
    constexpr DIRECTION get_direction() const;

    constexpr bool is_diagonal() const;
    constexpr bool is_orthogonal() const;

    constexpr bool is_increasing_square_index() const;
    constexpr bool is_decreasing_square_index() const;

    template <bool start_from_start_square>
    constexpr Square travel_occupied_ray(const uint8_t  index,
                                         const Bitboard occupied) const;

  private:

    Square    m_start_square;
    Bitboard  m_ray;
    DIRECTION m_direction;
};

constexpr Directional_Ray::Directional_Ray() :
    m_start_square(NO_SQUARE_OBJ),
    m_ray(EMPTY_BITBOARD),
    m_direction(DIRECTION::NO_DIRECTION)
{
}

// Creates a vector from two given squares.
constexpr Directional_Ray::Directional_Ray(const Square src, const Square dst)
{
    // Start with private members that define directional ray as having no ray.
    m_start_square = NO_SQUARE_OBJ;
    m_ray          = EMPTY_BITBOARD;
    m_direction    = DIRECTION::NO_DIRECTION;

    // If either src or dest has no square, there is no ray, return.
    if ((!src.has_square()) || (!dst.has_square())) { return; }

    // There are no squares in between src and dst if they are the same square.
    if (src == dst) { return; }

    const int8_t distance = dst.get_index() - src.get_index();

    // Squares src and dst are on the same file.
    if (src.get_file() == dst.get_file())
    {
        m_direction = ((distance > 0) ? SOUTH : NORTH); // Move along the file.
    }
    // Squares src and dst are on the same rank.
    else if (dst.get_rank() == src.get_rank())
    {
        m_direction = ((distance > 0) ? EAST : WEST); // Move along the rank.
    }
    // Squares src and dst are on a diagonal.
    else if ((distance % 9) == 0)
    {
        m_direction = ((distance > 0) ? SOUTHEAST
                                      : NORTHWEST); // Move along the diagonal.
    }
    // Squares src and dst are on another diagonal.
    else if ((distance % 7) == 0)
    {
        m_direction = ((distance > 0) ? SOUTHWEST
                                      : NORTHEAST); // Move along the diagonal.
    }
    // No diagonal or orthogonal path between the squares thus, no ray.
    else
    {
        return;
    }

    // Starting from square src travel in the direction of square dst until you
    // cannot travel any further.
    int8_t square_index = src.get_index();
    while ((Square(square_index).get_rank() < NUM_OF_RANKS_ON_CHESS_BOARD)
           && (Square(square_index).get_file() < NUM_OF_FILES_ON_CHESS_BOARD))
    {
        m_ray.set_square(Square(square_index));
        square_index += m_direction;
    }

    m_start_square = src;
}

// Creates a vector given an initial square and a direction.
constexpr Directional_Ray::Directional_Ray(const Square    src,
                                           const DIRECTION d) :
    m_start_square(src), m_ray(EMPTY_BITBOARD), m_direction(d)
{
    // Performs a "walk" in the direction specified and sets each square walked
    // on in the bitboard to form the ray.
    int8_t index = src.get_index();
    while (index >= 0 && index < NUM_OF_SQUARES_ON_CHESS_BOARD)
    {
        m_ray.set_square(Square(index));
        index += d;
    }
}

constexpr Square Directional_Ray::get_start_square() const
{
    return m_start_square;
}

constexpr Bitboard Directional_Ray::get_ray() const { return m_ray; }

constexpr DIRECTION Directional_Ray::get_direction() const
{
    return m_direction;
}

constexpr bool Directional_Ray::is_diagonal() const
{
    return ((m_direction == NORTHWEST) || (m_direction == NORTHEAST)
            || (m_direction == SOUTHWEST) || (m_direction == SOUTHEAST));
}

constexpr bool Directional_Ray::is_orthogonal() const
{
    return ((m_direction == NORTH) || (m_direction == SOUTH)
            || (m_direction == EAST) || (m_direction == WEST));
}

constexpr bool Directional_Ray::is_increasing_square_index() const
{
    return (m_direction > 0);
}

constexpr bool Directional_Ray::is_decreasing_square_index() const
{
    return (m_direction < 0);
}

// "Walks" the occupied squares of the directional ray and returns the square
// that is at index distance from the initial square. The initial square can be
// from either end of the ray.
template <bool start_from_start_square>
constexpr Square
Directional_Ray::travel_occupied_ray(const uint8_t  index,
                                     const Bitboard occupied) const
{
    const Bitboard occupied_ray = m_ray & occupied;
    uint8_t occupied_count      = 0;

    // Starts from start square.
    if constexpr (start_from_start_square)
    {
        if (is_increasing_square_index())
        {
            for (const Square square :
                 Bitboard_Iterable<LSB_TO_MSB>(occupied_ray))
            {
                if (occupied_count == index) { return square; }

                ++occupied_count;
            }
        }

        if (is_decreasing_square_index())
        {
            for (const Square square :
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
            for (const Square square :
                 Bitboard_Iterable<MSB_TO_LSB>(occupied_ray))
            {
                if (occupied_count == index) { return square; }

                ++occupied_count;
            }
        }

        if (is_decreasing_square_index())
        {
            for (const Square square :
                 Bitboard_Iterable<LSB_TO_MSB>(occupied_ray))
            {
                if (occupied_count == index) { return square; }

                ++occupied_count;
            }
        }
    }

    // There was no "walk" so we return no square.
    return NO_SQUARE_OBJ;
}
