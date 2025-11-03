#include "move_ordering.hpp"

#include "move_generator.hpp"

mvv_lva_array Move_Ordering::m_mvv_lva_array =
    Move_Ordering::generate_mvv_lva_array();

Move_Ordering::Move_Ordering(const Chess_Board& cb) : m_chess_board(cb) {
  Move_Generator mg(m_chess_board);
  mg.generate_all_moves(m_move_list);
  m_side_to_move_in_check = mg.is_side_to_move_in_check();
}

Chess_Move_List& Move_Ordering::get_sorted_moves() {
  mvv_lva_scorer();
  m_move_list.sort();
  return m_move_list;
}

mvv_lva_array Move_Ordering::generate_mvv_lva_array() {
  mvv_lva_array return_value;

  for (uint8_t attacker = PIECES::PAWN; attacker <= PIECES::KING; attacker++) {
    for (uint8_t victim = PIECES::PAWN; victim <= PIECES::QUEEN; victim++) {
      return_value[attacker][victim] =
          ((MVV_LVA_ATTACKER_VALUES[victim] + NUM_OF_UNIQUE_PIECES_PER_PLAYER) -
           (MVV_LVA_ATTACKER_VALUES[attacker] / 10));
    }
  }

  return return_value;
}

void Move_Ordering::mvv_lva_scorer() {
  for (Chess_Move& move : m_move_list) {
    // For capture moves, apply MVV LVA score.
    if (move.is_capture) {
      move.score = m_mvv_lva_array[move.moving_piece][move.captured_piece];
    }
    // Enpassant is a capture but not labeled under is_capture for move
    // generator implementation reasons. The moving/attacking piece is the same
    // as the victim, both are pawns.
    if (move.is_en_passant) {
      move.score = m_mvv_lva_array[move.moving_piece][move.moving_piece];
    }
  }
}

bool Move_Ordering::is_side_to_move_in_check() const {
  return m_side_to_move_in_check;
}
