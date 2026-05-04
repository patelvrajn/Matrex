#include "move_ordering.hpp"

#include <limits>

mvv_lva_array Move_Ordering::m_mvv_lva_array =
    Move_Ordering::generate_mvv_lva_array();

Move_Ordering::Move_Ordering(const Chess_Board& cb,
                             const Chess_Move&  hash_move) :
    m_chess_board(cb), m_hash_move(hash_move)
{
}

Chess_Move_List& Move_Ordering::get_sorted_moves()
{
    if (m_move_list.get_max_index() != -1)
    {
        move_scorer();
        m_move_list.sort();
    }
    return m_move_list;
}

Moves_Bitboard_Matrix& Move_Ordering::get_moves_matrix()
{
    return m_moves_matrix;
}

void Move_Ordering::move_scorer()
{
    for (Chess_Move& move : m_move_list)
    {
        // For capture moves, apply MVV LVA score.
        if (move.is_capture)
        {
            move.score =
                m_mvv_lva_array[move.moving_piece][move.captured_piece];
        }
        // Enpassant is a capture but not labeled under is_capture for move
        // generator implementation reasons. The moving/attacking piece is the
        // same as the victim, both are pawns.
        if (move.is_en_passant)
        {
            move.score = m_mvv_lva_array[move.moving_piece][move.moving_piece];
        }
        if (move.is_quiet_move())
        {
            move.score = m_butterfly_history[m_chess_board.get_side_to_move()]
                             .get_history_score(move);
        }
        // For the hash move, give it the maximum score to ensure it is sorted
        // to the front. If the hash move is not found, it won't be scored (e.g.
        // if move is Chess_Move()).
        if (move.is_same_move(m_hash_move))
        {
            move.score = std::numeric_limits<Move_Score>::max();
        }
    }
}

bool Move_Ordering::is_side_to_move_in_check() const
{
    Move_Generator mg(m_chess_board);
    return mg.is_side_to_move_in_check();
}

void Move_Ordering::update_butterfly_move_history(const Chess_Move& move)
{
    if (move.is_quiet_move())
    {
        m_butterfly_history[m_chess_board.get_side_to_move()].increment_history(
            move);
    }
}

void Move_Ordering::clear_all_history_heuristics()
{
    for (History_Table& history_table : m_butterfly_history)
    {
        history_table.clear();
    }
}
