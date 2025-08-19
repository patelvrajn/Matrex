#include "square.hpp"

#include <iostream>

Square::Square() : m_index(0) {}

Square::Square(uint8_t index) : m_index(index) {}

Square::Square(uint8_t r, uint8_t f) {
  m_index = (r << 3) + f;  // (8 * rank) + file
}

bool Square::is_light_square() const {
  // A square is light if the sum of its rank and file is even.
  // We compute parity using bitwise AND with 1 instead of modulo (% 2),
  // because (x & 1) is faster than (x % 2).
  return (((get_rank() + get_file()) & 1) == 0);
}

bool Square::is_dark_square() const { return (!is_light_square()); }

uint8_t Square::get_index() const { return m_index; }

uint8_t Square::get_rank() const {
  return (m_index >> 3);  // Divide by 8
}

uint8_t Square::get_file() const {
  return (m_index & 7);  // Modulo 8
}

uint64_t Square::get_mask() const {  // Does not return a bitboard otherwise,
                                     // there is a circular dependency.
  uint64_t temp = 1;
  return (temp << m_index);
}
