#pragma once

#include <stdint.h>

#include <array>

#include "globals.hpp"
#include "square.hpp"

class Bitboard {
 public:
  Bitboard();
  Bitboard(uint64_t board);

  void pretty_print() const;

  uint64_t get_board() const;
  void set_board(uint64_t board);

  void unset_square(const Square& s);
  void set_square(const Square& s);
  uint64_t get_square(const Square& s) const;

  uint8_t high_bit_count() const;
  uint8_t low_bit_count() const;

  int8_t get_index_of_high_lsb() const;
  int8_t get_index_of_high_msb() const;

  uint64_t get_between_squares_mask(const Square& a, const Square& b) const;

  // Equality operators overload.
  bool operator==(const Bitboard& other) const;
  bool operator!=(const Bitboard& other) const;

  // Bitwise operators overload.
  Bitboard operator|(const Bitboard& other) const;
  Bitboard operator&(const Bitboard& other) const;
  Bitboard operator^(const Bitboard& other) const;
  Bitboard operator<<(const uint8_t shift) const;
  Bitboard operator>>(const uint8_t shift) const;
  Bitboard operator~() const;
  Bitboard& operator|=(const Bitboard& other);
  Bitboard& operator&=(const Bitboard& other);
  Bitboard& operator^=(const Bitboard& other);
  Bitboard& operator<<=(uint8_t shift);
  Bitboard& operator>>=(uint8_t shift);

 private:
  uint64_t m_board;

  static bool m_is_between_squares_masks_initialized;

  static std::array<std::array<uint64_t, NUM_OF_SQUARES_ON_CHESS_BOARD>,
                    NUM_OF_SQUARES_ON_CHESS_BOARD>
      m_between_squares_masks;

  uint64_t generate_between_squares_mask(const Square& a,
                                         const Square& b) const;

  void init_between_squares_masks();
};

inline Bitboard::Bitboard(uint64_t board) : m_board(board) {
  if (!m_is_between_squares_masks_initialized) {
    init_between_squares_masks();
  }
}

inline bool Bitboard::operator==(const Bitboard& other) const {
  return (this->m_board == other.m_board);
}

inline bool Bitboard::operator!=(const Bitboard& other) const {
  return (this->m_board != other.m_board);
}

inline Bitboard Bitboard::operator|(const Bitboard& other) const {
  return Bitboard(m_board | other.m_board);
}

inline Bitboard Bitboard::operator&(const Bitboard& other) const {
  return Bitboard(m_board & other.m_board);
}

inline Bitboard Bitboard::operator^(const Bitboard& other) const {
  return Bitboard(m_board ^ other.m_board);
}

inline Bitboard Bitboard::operator<<(const uint8_t shift) const {
  return Bitboard(m_board << shift);
}

inline Bitboard Bitboard::operator>>(const uint8_t shift) const {
  return Bitboard(m_board >> shift);
}

inline Bitboard Bitboard::operator~() const { return Bitboard(~m_board); }

inline Bitboard& Bitboard::operator|=(const Bitboard& other) {
  m_board |= other.m_board;
  return *this;
}

inline Bitboard& Bitboard::operator&=(const Bitboard& other) {
  m_board &= other.m_board;
  return *this;
}

inline Bitboard& Bitboard::operator^=(const Bitboard& other) {
  m_board ^= other.m_board;
  return *this;
}

inline Bitboard& Bitboard::operator<<=(uint8_t shift) {
  m_board <<= shift;
  return *this;
}

inline Bitboard& Bitboard::operator>>=(uint8_t shift) {
  m_board >>= shift;
  return *this;
}
