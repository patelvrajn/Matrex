#include "chess_move.hpp"

#include <algorithm>

Chess_Move_List::Chess_Move_List() : m_max_index(0) {}

Chess_Move* Chess_Move_List::begin() const { return (Chess_Move*)&m_list[0]; }
Chess_Move* Chess_Move_List::end() const {
  return (Chess_Move*)&m_list[m_max_index];
}

uint16_t Chess_Move_List::get_max_index() const { return m_max_index; }

Chess_Move& Chess_Move_List::operator[](uint16_t index) {
  return m_list[index];
}

// Sort moves according to their score in non-ascending order.
void Chess_Move_List::sort() {
  std::stable_sort(begin(), end(), std::greater<Chess_Move>());
}
