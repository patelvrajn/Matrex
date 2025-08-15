#include "square.hpp"

Square::Square() : m_index(0), m_rank(0), m_file(0) {}

Square::Square(uint8_t index) : m_index(index) {
  m_rank = index >> 3;  // Divide by 8
  m_file = index & 7;   // Modulo 8
  m_mask = (1 << index);
}

Square::Square(uint8_t r, uint8_t f) : m_rank(r), m_file(f) {
  m_index = (r << 3) + f;  // (8 * rank) + file
  m_mask = (1 << m_index);
}

bool Square::is_light_square() const {
  // A square is light if the sum of its rank and file is even.
  // We compute parity using bitwise AND with 1 instead of modulo (% 2),
  // because (x & 1) is faster than (x % 2).
  return (((m_rank + m_file) & 1) == 0);
}

bool Square::is_dark_square() const { return (!is_light_square()); }

uint8_t Square::get_index() const { return m_index; }

uint8_t Square::get_rank() const { return m_rank; }

uint8_t Square::get_file() const { return m_file; }

uint64_t Square::get_mask() const {  // Does not return a bitboard otherwise,
                                     // there is a circular dependency.
  return m_mask;
}
