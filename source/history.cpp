#include "history.hpp"

History_Table::History_Table() { clear(); }

History_Score_Storage_Type
History_Table::get_history_score(const Chess_Move move) const
{
    return m_table[move.moving_piece][move.destination_square];
}

void History_Table::increment_history(const Chess_Move move)
{
    m_table[move.moving_piece][move.destination_square]++;
}

void History_Table::decrement_history(const Chess_Move move)
{
    m_table[move.moving_piece][move.destination_square]--;
}

void History_Table::update_history(const Chess_Move                 move,
                                   const History_Score_Storage_Type history)
{
    m_table[move.moving_piece][move.destination_square] = history;
}

void History_Table::clear()
{
    m_table.fill(static_cast<History_Score_Storage_Type>(0));
}
