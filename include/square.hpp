#pragma once

#include <stdint.h>
#include <string>

// Enumeration mapping chess square to bit index on the bitboard.
enum ESQUARE
{ // E for ENUM
    A8,
    B8,
    C8,
    D8,
    E8,
    F8,
    G8,
    H8,
    A7,
    B7,
    C7,
    D7,
    E7,
    F7,
    G7,
    H7,
    A6,
    B6,
    C6,
    D6,
    E6,
    F6,
    G6,
    H6,
    A5,
    B5,
    C5,
    D5,
    E5,
    F5,
    G5,
    H5,
    A4,
    B4,
    C4,
    D4,
    E4,
    F4,
    G4,
    H4,
    A3,
    B3,
    C3,
    D3,
    E3,
    F3,
    G3,
    H3,
    A2,
    B2,
    C2,
    D2,
    E2,
    F2,
    G2,
    H2,
    A1,
    B1,
    C1,
    D1,
    E1,
    F1,
    G1,
    H1,
    NO_SQUARE
};

constexpr std::string SQUARE_STRINGS[] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8", "a7", "b7",       "c7",
    "d7", "e7", "f7", "g7", "h7", "a6", "b6", "c6", "d6", "e6",       "f6",
    "g6", "h6", "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",       "a4",
    "b4", "c4", "d4", "e4", "f4", "g4", "h4", "a3", "b3", "c3",       "d3",
    "e3", "f3", "g3", "h3", "a2", "b2", "c2", "d2", "e2", "f2",       "g2",
    "h2", "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", "NO_SQUARE"};

class Square
{
  public:

    constexpr Square();
    constexpr Square(uint8_t index);
    constexpr Square(uint8_t r, uint8_t f);

    constexpr bool is_light_square() const;
    constexpr bool is_dark_square() const;

    constexpr uint8_t  get_index() const;
    constexpr uint8_t  get_rank() const;
    constexpr uint8_t  get_file() const;
    constexpr uint64_t get_mask() const;
    constexpr uint8_t  get_diagonal() const;
    constexpr uint8_t  get_antidiagonal() const;

    constexpr bool operator==(const Square& other) const;

  private:

    uint8_t m_index;
};

constexpr Square::Square() : m_index(0) {}

constexpr Square::Square(uint8_t index) : m_index(index) {}

constexpr Square::Square(uint8_t r, uint8_t f)
{
    m_index = (r << 3) + f; // (8 * rank) + file
}

constexpr bool Square::is_light_square() const
{
    // A square is light if the sum of its rank and file is even.
    // We compute parity using bitwise AND with 1 instead of modulo (% 2),
    // because (x & 1) is faster than (x % 2).
    return (((get_rank() + get_file()) & 1) == 0);
}

constexpr bool Square::is_dark_square() const { return (!is_light_square()); }

constexpr uint8_t Square::get_index() const { return m_index; }

constexpr uint8_t Square::get_rank() const
{
    return (m_index >> 3); // Divide by 8
}

constexpr uint8_t Square::get_file() const
{
    return (m_index & 7); // Modulo 8
}

constexpr uint64_t Square::get_mask() const
{ // Does not return a bitboard otherwise,
  // there is a circular dependency.
    uint64_t temp = 1;
    return (temp << m_index);
}

constexpr uint8_t Square::get_diagonal() const
{
    return 7
         + (this->get_rank()
            - this->get_file()); // 7 is to normalize (-7 to 7) to (0 to 14)
}

constexpr uint8_t Square::get_antidiagonal() const
{
    return (this->get_rank() + this->get_file());
}

constexpr bool Square::operator==(const Square& other) const
{
    return (this->m_index == other.m_index);
}
