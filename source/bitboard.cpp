#include "bitboard.hpp"

#include <iostream>

#include "globals.hpp"

Bitboard::Bitboard() : m_board(0) {}

Bitboard::Bitboard(uint64_t board) : m_board(board) {}

uint64_t Bitboard::get_board() const { return m_board; }

void Bitboard::set_board(uint64_t board) { m_board = board; }

void Bitboard::unset_square(const Square& s) {
  if (get_square(s)) {                   // If square is set.
    m_board = (m_board ^ s.get_mask());  // Unset square.
  }
}

void Bitboard::set_square(const Square& s) {
  m_board = (m_board | s.get_mask());
}

uint64_t Bitboard::get_square(const Square& s) {
  return (m_board & s.get_mask());
}

// Method to count how many bits are set to 1 in the bitboard (population
// count).
uint8_t Bitboard::high_bit_count() const {
  // Counter to keep track of how many set bits we find.
  uint8_t high_bit_cnt = 0;

  // Make a temporary copy of the internal board so we don’t mutate the
  // original.
  uint64_t temp_board = m_board;

  // Loop until there are no more set bits left in temp_board.
  while (temp_board) {
    // Increment counter for each set bit we remove.
    high_bit_cnt++;

    // Clear the least significant set bit.
    // Trick: x & (x - 1) removes the lowest high bit in x.
    temp_board &= (temp_board - 1);
  }

  // Return the total number of set bits in the bitboard.
  return high_bit_cnt;
}

uint8_t Bitboard::low_bit_count() const {
  return (NUM_OF_SQUARES_ON_CHESS_BOARD - high_bit_count());
}

Bitboard Bitboard::operator|(const Bitboard& other) const {
  return Bitboard(m_board | other.m_board);
}

Bitboard Bitboard::operator&(const Bitboard& other) const {
  return Bitboard(m_board & other.m_board);
}

Bitboard Bitboard::operator^(const Bitboard& other) const {
  return Bitboard(m_board ^ other.m_board);
}

Bitboard Bitboard::operator<<(const uint8_t shift) const {
  return Bitboard(m_board << shift);
}

Bitboard Bitboard::operator>>(const uint8_t shift) const {
  return Bitboard(m_board >> shift);
}

Bitboard Bitboard::operator~() const { return Bitboard(~m_board); }

Bitboard& Bitboard::operator|=(const Bitboard& other) {
  m_board |= other.m_board;
  return *this;
}

Bitboard& Bitboard::operator&=(const Bitboard& other) {
  m_board &= other.m_board;
  return *this;
}

Bitboard& Bitboard::operator^=(const Bitboard& other) {
  m_board ^= other.m_board;
  return *this;
}

Bitboard& Bitboard::operator<<=(uint8_t shift) {
  m_board <<= shift;
  return *this;
}

Bitboard& Bitboard::operator>>=(uint8_t shift) {
  m_board >>= shift;
  return *this;
}
