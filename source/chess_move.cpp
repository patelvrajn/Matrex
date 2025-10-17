#include "chess_move.hpp"

#include <algorithm>

Chess_Move_List::Chess_Move_List() : m_max_index(0) {}

const Chess_Move* Chess_Move_List::begin() const { return &m_list[0]; }
const Chess_Move* Chess_Move_List::end() const { return &m_list[m_max_index]; }

const Chess_Move* Chess_Move_List::find(Chess_Move move) const {
  return std::find(begin(), end(), move);
}

uint16_t Chess_Move_List::get_max_index() const { return m_max_index; }
