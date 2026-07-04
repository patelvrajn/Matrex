#include "history.hpp"

History_Table::History_Table() { clear(); }

History_Score_Storage_Type& History_Table::operator[](const Chess_Move& move)
{
    return m_table[move.moving_piece][move.destination_square];
}

const History_Score_Storage_Type&
History_Table::operator[](const Chess_Move& move) const
{
    return m_table[move.moving_piece][move.destination_square];
}

void History_Table::clear() { m_table.fill(0); }

Continuation_History_Table::Continuation_History_Table() {}

History_Table& Continuation_History_Table::operator[](const Chess_Move& move)
{
    return m_table[move.moving_piece][move.destination_square];
}

const History_Table&
Continuation_History_Table::operator[](const Chess_Move& move) const
{
    return m_table[move.moving_piece][move.destination_square];
}

void Continuation_History_Table::clear()
{
    for (auto& outer_table : m_table)
    {
        for (auto& inner_table : outer_table) { inner_table.clear(); }
    }
}
