#include "chess_move.hpp"

Chess_Move_List::Chess_Move_List() : m_max_index(0) {}

const Chess_Move* Chess_Move_List::begin() const { return &m_list[0]; }
const Chess_Move* Chess_Move_List::end() const { return &m_list[m_max_index]; }
