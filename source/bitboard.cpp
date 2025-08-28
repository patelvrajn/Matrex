#include "bitboard.hpp"

#include <iostream>

#include "globals.hpp"

Bitboard::Bitboard() : m_board(0) {}

Bitboard::Bitboard(uint64_t board) : m_board(board) {}

void Bitboard::pretty_print() const {
  std::cout << "Bitboard: " << m_board << " (0x" << std::hex << m_board << ")"
            << std::endl;

  for (uint8_t rank = 0; rank < NUM_OF_RANKS_ON_CHESS_BOARD; rank++) {
    // Print the ranks on the left hand side of the board before the first file.
    std::cout << (NUM_OF_RANKS_ON_CHESS_BOARD - rank) << "   ";

    for (uint8_t file = 0; file < NUM_OF_FILES_ON_CHESS_BOARD; file++) {
      Square s(rank, file);

      if (get_square(s)) {
        std::cout << 1 << " ";
      } else {
        std::cout << 0 << " ";
      }
    }

    std::cout << std::endl;
  }

  std::cout << std::endl;
  std::cout << "    ";

  for (uint8_t file = 0; file < NUM_OF_FILES_ON_CHESS_BOARD; file++) {
    char file_char = 'A' + file;
    std::cout << file_char << " ";
  }

  std::cout << std::endl;
}

uint64_t Bitboard::get_board() const { return m_board; }

void Bitboard::set_board(uint64_t board) { m_board = board; }

void Bitboard::unset_square(const Square& s) { m_board &= ~(s.get_mask()); }

void Bitboard::set_square(const Square& s) {
  m_board = (m_board | s.get_mask());
}

uint64_t Bitboard::get_square(const Square& s) const {
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

int8_t Bitboard::get_index_of_high_lsb() const {
  if (m_board == 0) {
    return -1;
  }

  int8_t position = 0;
  uint64_t temp_board = m_board;

  while ((temp_board & 1) == 0) {
    temp_board >>= 1;
    position++;
  }

  return position;
}

bool Bitboard::operator==(const Bitboard& other) const {
  return (this->m_board == other.m_board);
}

bool Bitboard::operator!=(const Bitboard& other) const {
  return (this->m_board != other.m_board);
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
