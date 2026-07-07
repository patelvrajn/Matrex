#include "history.hpp"

Quiet_History_Table::Quiet_History_Table() { clear(); }

History_Score_Storage_Type&
Quiet_History_Table::operator[](const Chess_Move& move)
{
    return m_table[move.moving_piece][move.destination_square];
}

const History_Score_Storage_Type&
Quiet_History_Table::operator[](const Chess_Move& move) const
{
    return m_table[move.moving_piece][move.destination_square];
}

void Quiet_History_Table::clear() { m_table.fill(0); }

Quiet_Continuation_History_Table::Quiet_Continuation_History_Table() {}

Quiet_History_Table&
Quiet_Continuation_History_Table::operator[](const Chess_Move& move)
{
    return m_table[move.moving_piece][move.destination_square];
}

const Quiet_History_Table&
Quiet_Continuation_History_Table::operator[](const Chess_Move& move) const
{
    return m_table[move.moving_piece][move.destination_square];
}

void Quiet_Continuation_History_Table::clear()
{
    for (auto& outer_table : m_table)
    {
        for (auto& inner_table : outer_table) { inner_table.clear(); }
    }
}

Capture_History_Table::Capture_History_Table() { clear(); }

History_Score_Storage_Type&
Capture_History_Table::operator[](const Chess_Move& move)
{
    return m_table[move.moving_piece][move.destination_square]
                  [move.captured_piece];
}

const History_Score_Storage_Type&
Capture_History_Table::operator[](const Chess_Move& move) const
{
    return m_table[move.moving_piece][move.destination_square]
                  [move.captured_piece];
}

void Capture_History_Table::clear() { m_table.fill(0); }

Capture_Continuation_History_Table::Capture_Continuation_History_Table()
{
    clear();
}

Capture_History_Table&
Capture_Continuation_History_Table::operator[](const Chess_Move& move)
{
    return m_table[move.moving_piece][move.destination_square];
}

const Capture_History_Table&
Capture_Continuation_History_Table::operator[](const Chess_Move& move) const
{
    return m_table[move.moving_piece][move.destination_square];
}

void Capture_Continuation_History_Table::clear()
{
    for (auto& outer_table : m_table)
    {
        for (auto& inner_table : outer_table) { inner_table.clear(); }
    }
}
