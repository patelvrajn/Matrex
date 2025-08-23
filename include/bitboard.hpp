#pragma once

#include <stdint.h>

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
};
