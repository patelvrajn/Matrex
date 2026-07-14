#pragma once

#include <array>
#include <cstdint>
#include <stdint.h>

#include "globals.hpp"
#include "square.hpp"

constexpr uint64_t generate_between_squares_mask(const Square a, const Square b)
{
    uint64_t  mask  = 0;
    DIRECTION delta = NO_DIRECTION;

    // There are no squares in between a and b if they are the same square.
    if (a == b) { return mask; }

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
        // Move along the diagonal.
        delta = ((distance > 0) ? SOUTHEAST : NORTHWEST);
    }
    // Squares a and b are on another diagonal.
    else if ((distance % 7) == 0)
    {
        // Move along the diagonal.
        delta = ((distance > 0) ? SOUTHWEST : NORTHEAST);
    }
    else
    {
        return mask; // No diagonal or orthogonal path between the squares.
    }

    // Starting from square a travel until you get to square b while setting the
    // bits in the mask corresponding to squares between squres a and b.
    int square_index = a.get_index() + delta;
    while (square_index != b.get_index())
    {
        mask         |= Square(square_index).get_mask();
        square_index += delta;
    }

    return mask;
}

constexpr uint64_t generate_backward_squares_mask(const PIECE_COLOR c,
                                                  const Square      s)
{
    uint64_t backward_squares_mask = 0;

    const int8_t backward_rank_increment = (c == WHITE) ? -1 : 1;

    int8_t rank = s.get_rank() + backward_rank_increment;

    // As long as rank is within bounds, keep looping.
    while ((!(rank < 0)) && (!(rank >= (NUM_OF_RANKS_ON_CHESS_BOARD))))
    {
        // All squares on the rank are backward squares.
        for (int8_t file = 0; file < NUM_OF_FILES_ON_CHESS_BOARD; file++)
        {
            backward_squares_mask |= Square(rank, file).get_mask();
        }

        rank += backward_rank_increment;
    }

    return backward_squares_mask;
}

constexpr auto init_between_squares_masks()
{
    Multi_Array<uint64_t, NUM_OF_SQUARES_ON_CHESS_BOARD, NUM_OF_SQUARES_ON_CHESS_BOARD>
        between_squares_masks;

    for (uint8_t outer_square_idx = 0;
         outer_square_idx < NUM_OF_SQUARES_ON_CHESS_BOARD;
         outer_square_idx++)
    {
        for (uint8_t inner_square_idx = 0;
             inner_square_idx < NUM_OF_SQUARES_ON_CHESS_BOARD;
             inner_square_idx++)
        {
            between_squares_masks[outer_square_idx][inner_square_idx] =
                generate_between_squares_mask(Square(outer_square_idx),
                                              Square(inner_square_idx));
        }
    }

    return between_squares_masks;
}

constexpr auto init_backward_squares_masks()
{
    Multi_Array<uint64_t, NUM_OF_PLAYERS, NUM_OF_SQUARES_ON_CHESS_BOARD>
        backward_squares_masks;

    uint8_t square_index = 0;

    for (PIECE_COLOR c = PIECE_COLOR::WHITE; c <= PIECE_COLOR::BLACK; ++c)
    {
        for (auto& mask : backward_squares_masks[c])
        {
            mask = generate_backward_squares_mask(c, Square(square_index));

            square_index++;
        }
    }

    return backward_squares_masks;
}

constexpr auto init_rank_masks()
{
    Multi_Array<uint64_t, NUM_OF_SQUARES_ON_CHESS_BOARD> rank_masks;

    for (uint8_t outer_square_index = 0;
         outer_square_index < NUM_OF_SQUARES_ON_CHESS_BOARD;
         outer_square_index++)
    {
        const Square outer_square(outer_square_index);

        uint64_t rank = 0;

        for (uint8_t inner_square_index = 0;
             inner_square_index < NUM_OF_SQUARES_ON_CHESS_BOARD;
             inner_square_index++)
        {
            const Square inner_square(inner_square_index);

            if (inner_square.get_rank() == outer_square.get_rank())
            {
                rank |= inner_square.get_mask();
            }
        }

        rank_masks[outer_square_index] = rank;
    }

    return rank_masks;
}

constexpr auto init_file_masks()
{
    Multi_Array<uint64_t, NUM_OF_SQUARES_ON_CHESS_BOARD> file_masks;

    for (uint8_t outer_square_index = 0;
         outer_square_index < NUM_OF_SQUARES_ON_CHESS_BOARD;
         outer_square_index++)
    {
        const Square outer_square(outer_square_index);

        uint64_t file = 0;

        for (uint8_t inner_square_index = 0;
             inner_square_index < NUM_OF_SQUARES_ON_CHESS_BOARD;
             inner_square_index++)
        {
            const Square inner_square(inner_square_index);

            if (inner_square.get_file() == outer_square.get_file())
            {
                file |= inner_square.get_mask();
            }
        }

        file_masks[outer_square_index] = file;
    }

    return file_masks;
}

constexpr auto init_diagonal_masks()
{
    Multi_Array<uint64_t, NUM_OF_SQUARES_ON_CHESS_BOARD> diagonal_masks;

    for (uint8_t outer_square_index = 0;
         outer_square_index < NUM_OF_SQUARES_ON_CHESS_BOARD;
         outer_square_index++)
    {
        const Square outer_square(outer_square_index);

        uint64_t diag = 0;

        for (uint8_t inner_square_index = 0;
             inner_square_index < NUM_OF_SQUARES_ON_CHESS_BOARD;
             inner_square_index++)
        {
            const Square inner_square(inner_square_index);

            if (inner_square.get_diagonal() == outer_square.get_diagonal())
            {
                diag |= inner_square.get_mask();
            }
        }

        diagonal_masks[outer_square_index] = diag;
    }

    return diagonal_masks;
}

constexpr auto init_antidiagonal_masks()
{
    Multi_Array<uint64_t, NUM_OF_SQUARES_ON_CHESS_BOARD> antidiagonal_masks;

    for (uint8_t outer_square_index = 0;
         outer_square_index < NUM_OF_SQUARES_ON_CHESS_BOARD;
         outer_square_index++)
    {
        const Square outer_square(outer_square_index);

        uint64_t antidiag = 0;

        for (uint8_t inner_square_index = 0;
             inner_square_index < NUM_OF_SQUARES_ON_CHESS_BOARD;
             inner_square_index++)
        {
            const Square inner_square(inner_square_index);

            if (inner_square.get_antidiagonal()
                == outer_square.get_antidiagonal())
            {
                antidiag |= inner_square.get_mask();
            }
        }

        antidiagonal_masks[outer_square_index] = antidiag;
    }

    return antidiagonal_masks;
}

enum Bitboard_Iteration_Order
{
    LSB_TO_MSB,
    MSB_TO_LSB
};

class Bitboard
{
  public:

    template <Bitboard_Iteration_Order iteration_order = LSB_TO_MSB>
    class Iterator
    {
      public:

        Iterator(uint64_t board);
        Square    operator*() const;
        Iterator& operator++();
        bool      operator==(const Iterator& other) const;
        bool      operator!=(const Iterator& other) const;

      private:

        uint64_t m_board;
    };

    template <Bitboard_Iteration_Order iteration_order = LSB_TO_MSB>
    Iterator<iteration_order> begin() const;

    template <Bitboard_Iteration_Order iteration_order = LSB_TO_MSB>
    Iterator<iteration_order> end() const;

    constexpr Bitboard();
    constexpr Bitboard(uint64_t board);

    void pretty_print() const;

    constexpr uint64_t get_board() const;
    constexpr void     set_board(uint64_t board);

    constexpr void     unset_square(const Square s);
    constexpr void     set_square(const Square s);
    constexpr uint64_t get_square(const Square s) const;

    constexpr uint8_t high_bit_count() const;
    constexpr uint8_t low_bit_count() const;

    constexpr int8_t get_index_of_high_lsb() const;
    constexpr int8_t get_index_of_high_msb() const;

    constexpr static Bitboard get_backward_squares_mask(const Square      s,
                                                        const PIECE_COLOR side);

    constexpr static Bitboard get_between_squares_mask(const Square a,
                                                       const Square b);
    constexpr static Bitboard get_rank_mask(const Square s);
    constexpr static Bitboard get_file_mask(const Square s);
    constexpr static Bitboard get_diagonal_mask(const Square s);
    constexpr static Bitboard get_antidiagonal_mask(const Square s);
    constexpr static Bitboard get_infinite_ray(const Square a, const Square b);

    constexpr static bool is_piece_obstructed(const Square   a,
                                              const Square   b,
                                              const Bitboard occupancy);

    // Equality operators overload.
    constexpr bool operator==(const Bitboard& other) const;
    constexpr bool operator!=(const Bitboard& other) const;

    // Bitwise operators overload.
    constexpr Bitboard  operator|(const Bitboard& other) const;
    constexpr Bitboard  operator&(const Bitboard& other) const;
    constexpr Bitboard  operator^(const Bitboard& other) const;
    constexpr Bitboard  operator<<(const uint8_t shift) const;
    constexpr Bitboard  operator>>(const uint8_t shift) const;
    constexpr Bitboard  operator~() const;
    constexpr Bitboard& operator|=(const Bitboard& other);
    constexpr Bitboard& operator&=(const Bitboard& other);
    constexpr Bitboard& operator^=(const Bitboard& other);
    constexpr Bitboard& operator<<=(uint8_t shift);
    constexpr Bitboard& operator>>=(uint8_t shift);

  private:

    uint64_t m_board;

    inline static constexpr auto m_between_squares_masks =
        init_between_squares_masks();

    inline static constexpr auto m_backward_squares_masks =
        init_backward_squares_masks();

    inline static constexpr auto m_rank_masks     = init_rank_masks();
    inline static constexpr auto m_file_masks     = init_file_masks();
    inline static constexpr auto m_diagonal_masks = init_diagonal_masks();
    inline static constexpr auto m_antidiagonal_masks =
        init_antidiagonal_masks();
};

using Bitboard_Array = Multi_Array<Bitboard, NUM_OF_SQUARES_ON_CHESS_BOARD>;

template <Bitboard_Iteration_Order iteration_order>
Bitboard::Iterator<iteration_order>::Iterator(uint64_t board) : m_board(board)
{
}

template <Bitboard_Iteration_Order iteration_order>
Square Bitboard::Iterator<iteration_order>::operator*() const
{
    if constexpr (iteration_order == LSB_TO_MSB)
    {
        return Square(__builtin_ctzll(m_board));
    }

    if constexpr (iteration_order == MSB_TO_LSB)
    {
        return Square((NUM_OF_SQUARES_ON_CHESS_BOARD - 1)
                      - std::countl_zero(m_board));
    }
}

template <Bitboard_Iteration_Order iteration_order>
Bitboard::Iterator<iteration_order>&
Bitboard::Iterator<iteration_order>::operator++()
{
    if constexpr (iteration_order == LSB_TO_MSB) { m_board &= (m_board - 1); }

    if constexpr (iteration_order == MSB_TO_LSB)
    {
        const Square square((NUM_OF_SQUARES_ON_CHESS_BOARD - 1)
                            - std::countl_zero(m_board));
        m_board &= ~square.get_mask();
    }

    return *this;
}

template <Bitboard_Iteration_Order iteration_order>
bool Bitboard::Iterator<iteration_order>::operator==(
    const Iterator& other) const
{
    return (other.m_board == m_board);
}

template <Bitboard_Iteration_Order iteration_order>
bool Bitboard::Iterator<iteration_order>::operator!=(
    const Iterator& other) const
{
    return (other.m_board != m_board);
}

template <Bitboard_Iteration_Order iteration_order>
Bitboard::Iterator<iteration_order> Bitboard::begin() const
{
    return Bitboard::Iterator<iteration_order>(m_board);
}

template <Bitboard_Iteration_Order iteration_order>
Bitboard::Iterator<iteration_order> Bitboard::end() const
{
    return Bitboard::Iterator<iteration_order>(0ULL);
}

// An iterable wrapper around the Bitboard class to allow for range-based for
// loops with a specific iteration order.
template <Bitboard_Iteration_Order Order>
struct Bitboard_Iterable
{
    const Bitboard& bb;

    Bitboard_Iterable(const Bitboard& bitboard) : bb(bitboard) {}

    auto begin() const { return bb.begin<Order>(); }

    auto end() const { return bb.end<Order>(); }
};

constexpr Bitboard::Bitboard() : m_board(0) {}

constexpr Bitboard::Bitboard(uint64_t board) : m_board(board) {}

constexpr uint64_t Bitboard::get_board() const { return m_board; }

constexpr void Bitboard::set_board(uint64_t board) { m_board = board; }

constexpr void Bitboard::unset_square(const Square s)
{
    m_board &= ~(s.get_mask());
}

constexpr void Bitboard::set_square(const Square s)
{
    m_board = (m_board | s.get_mask());
}

constexpr uint64_t Bitboard::get_square(const Square s) const
{
    return (m_board & s.get_mask());
}

// Method to count how many bits are set to 1 in the bitboard.
constexpr uint8_t Bitboard::high_bit_count() const
{
    // Counter to keep track of how many set bits we find.
    uint8_t high_bit_cnt = 0;

    // Make a temporary copy of the internal board so we don’t mutate the
    // original.
    uint64_t temp_board = m_board;

    // Loop until there are no more set bits left.
    while (temp_board)
    {
        // Increment counter for each set bit we remove.
        high_bit_cnt++;

        // Clear the least significant set bit.
        // Trick: x & (x - 1) removes the lowest high bit in x.
        temp_board &= (temp_board - 1);
    }

    // Return the total number of set bits in the bitboard.
    return high_bit_cnt;
}

constexpr uint8_t Bitboard::low_bit_count() const
{
    return (NUM_OF_SQUARES_ON_CHESS_BOARD - high_bit_count());
}

constexpr int8_t Bitboard::get_index_of_high_lsb() const
{
    if (m_board == 0) { return -1; }

    return __builtin_ctzll(m_board);
}

constexpr int8_t Bitboard::get_index_of_high_msb() const
{
    if (m_board == 0) { return -1; }

    return ((NUM_OF_SQUARES_ON_CHESS_BOARD - 1) - std::countl_zero(m_board));
}

constexpr Bitboard Bitboard::get_backward_squares_mask(const Square      s,
                                                       const PIECE_COLOR side)
{
    return m_backward_squares_masks[side][s.get_index()];
}

constexpr Bitboard Bitboard::get_between_squares_mask(const Square a,
                                                      const Square b)
{
    return Bitboard(m_between_squares_masks[a.get_index()][b.get_index()]);
}

constexpr Bitboard Bitboard::get_rank_mask(const Square s)
{
    return Bitboard(m_rank_masks[s.get_index()]);
}

constexpr Bitboard Bitboard::get_file_mask(const Square s)
{
    return Bitboard(m_file_masks[s.get_index()]);
}

constexpr Bitboard Bitboard::get_diagonal_mask(const Square s)
{
    return Bitboard(m_diagonal_masks[s.get_index()]);
}

constexpr Bitboard Bitboard::get_antidiagonal_mask(const Square s)
{
    return Bitboard(m_antidiagonal_masks[s.get_index()]);
}

constexpr Bitboard Bitboard::get_infinite_ray(const Square a, const Square b)
{
    if (a.get_rank() == b.get_rank()) { return get_rank_mask(a); }

    if (a.get_file() == b.get_file()) { return get_file_mask(a); }

    if (a.get_diagonal() == b.get_diagonal()) { return get_diagonal_mask(a); }

    if (a.get_antidiagonal() == b.get_antidiagonal())
    {
        return get_antidiagonal_mask(a);
    }

    return Bitboard();
}

constexpr bool Bitboard::is_piece_obstructed(const Square   a,
                                             const Square   b,
                                             const Bitboard occupancy)
{
    return ((get_between_squares_mask(a, b) & occupancy) != Bitboard(0));
}

constexpr bool Bitboard::operator==(const Bitboard& other) const
{
    return (this->m_board == other.m_board);
}

constexpr bool Bitboard::operator!=(const Bitboard& other) const
{
    return (this->m_board != other.m_board);
}

constexpr Bitboard Bitboard::operator|(const Bitboard& other) const
{
    return Bitboard(m_board | other.m_board);
}

constexpr Bitboard Bitboard::operator&(const Bitboard& other) const
{
    return Bitboard(m_board & other.m_board);
}

constexpr Bitboard Bitboard::operator^(const Bitboard& other) const
{
    return Bitboard(m_board ^ other.m_board);
}

constexpr Bitboard Bitboard::operator<<(const uint8_t shift) const
{
    return Bitboard(m_board << shift);
}

constexpr Bitboard Bitboard::operator>>(const uint8_t shift) const
{
    return Bitboard(m_board >> shift);
}

constexpr Bitboard Bitboard::operator~() const { return Bitboard(~m_board); }

constexpr Bitboard& Bitboard::operator|=(const Bitboard& other)
{
    m_board |= other.m_board;
    return *this;
}

constexpr Bitboard& Bitboard::operator&=(const Bitboard& other)
{
    m_board &= other.m_board;
    return *this;
}

constexpr Bitboard& Bitboard::operator^=(const Bitboard& other)
{
    m_board ^= other.m_board;
    return *this;
}

constexpr Bitboard& Bitboard::operator<<=(uint8_t shift)
{
    m_board <<= shift;
    return *this;
}

constexpr Bitboard& Bitboard::operator>>=(uint8_t shift)
{
    m_board >>= shift;
    return *this;
}

constexpr Bitboard init_light_square_bitboard()
{
    Bitboard output;

    for (uint8_t from_square_idx = 0;
         from_square_idx < NUM_OF_SQUARES_ON_CHESS_BOARD;
         from_square_idx++)
    {
        Square s(from_square_idx);
        if (s.is_light_square()) { output.set_square(s); }
    }

    return output;
}

constexpr Bitboard init_dark_square_bitboard()
{
    Bitboard output;

    for (uint8_t from_square_idx = 0;
         from_square_idx < NUM_OF_SQUARES_ON_CHESS_BOARD;
         from_square_idx++)
    {
        Square s(from_square_idx);
        if (s.is_dark_square()) { output.set_square(s); }
    }

    return output;
}

constexpr Bitboard EMPTY_BITBOARD         = Bitboard(0);
constexpr Bitboard LIGHT_SQUARES_BITBOARD = init_light_square_bitboard();
constexpr Bitboard DARK_SQUARES_BITBOARD  = init_dark_square_bitboard();
