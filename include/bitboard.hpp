#pragma once

#include <array>
#include <stdint.h>

#include "globals.hpp"
#include "square.hpp"

consteval uint64_t generate_between_squares_mask(const Square& a,
                                                 const Square& b)
{
    uint64_t mask  = 0;
    int8_t   delta = 0;

    // There are no squares in between a and b if they are the same square.
    if (a == b) { return mask; }

    const int8_t distance = b.get_index() - a.get_index();

    if ((distance & 7) == 0)
    { // Squares a and b are on the same file.

        delta = ((distance > 0) ? 8 : -8); // Move along the file.
    }
    else if (b.get_rank() == a.get_rank())
    { // Squares a and b are on the same rank.

        delta = ((distance > 0) ? 1 : -1); // Move along the rank.
    }
    else if ((distance % 9) == 0)
    { // Squares a and b are on a diagonal.

        delta = ((distance > 0) ? 9 : -9); // Move along the diagonal.
    }
    else if ((distance % 7) == 0)
    { // Squares a and b are on another diagonal.

        delta = ((distance > 0) ? 7 : -7); // Move along the diagonal.
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

consteval std::array<std::array<uint64_t, NUM_OF_SQUARES_ON_CHESS_BOARD>,
                     NUM_OF_SQUARES_ON_CHESS_BOARD>
init_between_squares_masks()
{
    std::array<std::array<uint64_t, NUM_OF_SQUARES_ON_CHESS_BOARD>,
               NUM_OF_SQUARES_ON_CHESS_BOARD>
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

consteval std::array<uint64_t, NUM_OF_SQUARES_ON_CHESS_BOARD> init_rank_masks()
{
    std::array<uint64_t, NUM_OF_SQUARES_ON_CHESS_BOARD> rank_masks;

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

consteval std::array<uint64_t, NUM_OF_SQUARES_ON_CHESS_BOARD> init_file_masks()
{
    std::array<uint64_t, NUM_OF_SQUARES_ON_CHESS_BOARD> file_masks;

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

consteval std::array<uint64_t, NUM_OF_SQUARES_ON_CHESS_BOARD>
init_diagonal_masks()
{
    std::array<uint64_t, NUM_OF_SQUARES_ON_CHESS_BOARD> diagonal_masks;

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

consteval std::array<uint64_t, NUM_OF_SQUARES_ON_CHESS_BOARD>
init_antidiagonal_masks()
{
    std::array<uint64_t, NUM_OF_SQUARES_ON_CHESS_BOARD> antidiagonal_masks;

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

class Bitboard
{
  public:

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

    Iterator begin() const;
    Iterator end() const;

    constexpr Bitboard();
    constexpr Bitboard(uint64_t board);

    void pretty_print() const;

    constexpr uint64_t get_board() const;
    constexpr void     set_board(uint64_t board);

    constexpr void     unset_square(const Square& s);
    constexpr void     set_square(const Square& s);
    constexpr uint64_t get_square(const Square& s) const;

    constexpr uint8_t high_bit_count() const;
    constexpr uint8_t low_bit_count() const;

    constexpr int8_t get_index_of_high_lsb() const;
    constexpr int8_t get_index_of_high_msb() const;

    constexpr Bitboard get_backward_squares_mask(const Square& s,
                                                 PIECE_COLOR   side) const;

    constexpr static Bitboard get_between_squares_mask(const Square& a,
                                                       const Square& b);
    constexpr static Bitboard get_rank_mask(const Square& s);
    constexpr static Bitboard get_file_mask(const Square& s);
    constexpr static Bitboard get_diagonal_mask(const Square& s);
    constexpr static Bitboard get_antidiagonal_mask(const Square& s);
    constexpr static Bitboard get_infinite_ray(const Square& a,
                                               const Square& b);

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

    inline static constexpr std::array<
        std::array<uint64_t, NUM_OF_SQUARES_ON_CHESS_BOARD>,
        NUM_OF_SQUARES_ON_CHESS_BOARD>
        m_between_squares_masks = init_between_squares_masks();
    inline static constexpr std::array<uint64_t, NUM_OF_SQUARES_ON_CHESS_BOARD>
        m_rank_masks = init_rank_masks();
    inline static constexpr std::array<uint64_t, NUM_OF_SQUARES_ON_CHESS_BOARD>
        m_file_masks = init_file_masks();
    inline static constexpr std::array<uint64_t, NUM_OF_SQUARES_ON_CHESS_BOARD>
        m_diagonal_masks = init_diagonal_masks();
    inline static constexpr std::array<uint64_t, NUM_OF_SQUARES_ON_CHESS_BOARD>
        m_antidiagonal_masks = init_antidiagonal_masks();
};

using Bitboard_Array = multi_array<Bitboard, NUM_OF_SQUARES_ON_CHESS_BOARD>;

constexpr Bitboard::Bitboard() : m_board(0) {}

constexpr Bitboard::Bitboard(uint64_t board) : m_board(board) {}

constexpr uint64_t Bitboard::get_board() const { return m_board; }

constexpr void Bitboard::set_board(uint64_t board) { m_board = board; }

constexpr void Bitboard::unset_square(const Square& s)
{
    m_board &= ~(s.get_mask());
}

constexpr void Bitboard::set_square(const Square& s)
{
    m_board = (m_board | s.get_mask());
}

constexpr uint64_t Bitboard::get_square(const Square& s) const
{
    return (m_board & s.get_mask());
}

// Method to count how many bits are set to 1 in the bitboard (population
// count).
constexpr uint8_t Bitboard::high_bit_count() const
{
    // Counter to keep track of how many set bits we find.
    uint8_t high_bit_cnt = 0;

    // Make a temporary copy of the internal board so we don’t mutate the
    // original.
    uint64_t temp_board = m_board;

    // Loop until there are no more set bits left in temp_board.
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

constexpr Bitboard Bitboard::get_backward_squares_mask(const Square& s,
                                                       PIECE_COLOR   side) const
{
    Bitboard backward_squares_mask = *this;

    for (const Square& square : *this)
    {
        int8_t rank_diff = s.get_rank() - square.get_rank();

        bool is_backward_move =
            (side == WHITE) ? (rank_diff < 0) : (rank_diff > 0);

        if (!is_backward_move)
        { // Unset all forward squares.
            backward_squares_mask.unset_square(square);
        }
    }

    return backward_squares_mask;
}

constexpr Bitboard Bitboard::get_between_squares_mask(const Square& a,
                                                      const Square& b)
{
    return Bitboard(m_between_squares_masks[a.get_index()][b.get_index()]);
}

constexpr Bitboard Bitboard::get_rank_mask(const Square& s)
{
    return Bitboard(m_rank_masks[s.get_index()]);
}

constexpr Bitboard Bitboard::get_file_mask(const Square& s)
{
    return Bitboard(m_file_masks[s.get_index()]);
}

constexpr Bitboard Bitboard::get_diagonal_mask(const Square& s)
{
    return Bitboard(m_diagonal_masks[s.get_index()]);
}

constexpr Bitboard Bitboard::get_antidiagonal_mask(const Square& s)
{
    return Bitboard(m_antidiagonal_masks[s.get_index()]);
}

constexpr Bitboard Bitboard::get_infinite_ray(const Square& a, const Square& b)
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

constexpr Bitboard EMPTY_BITBOARD = Bitboard(0);
