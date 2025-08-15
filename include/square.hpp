#pragma once

#include <stdint.h>

class Square {
 public:
  Square();
  Square(uint8_t index);
  Square(uint8_t r, uint8_t f);

  bool is_light_square() const;
  bool is_dark_square() const;

  uint8_t get_index() const;
  uint8_t get_rank() const;
  uint8_t get_file() const;
  uint64_t get_mask() const;

 private:
  uint8_t m_index;
  uint8_t m_rank;
  uint8_t m_file;
  uint64_t m_mask;
};
