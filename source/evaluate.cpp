#include "evaluate.hpp"

Evaluator::Evaluator(const Chess_Board& cb, const Moves_Bitboard_Matrix& matrix)
    : m_chess_board(cb), m_moves_matrix(matrix) {}

Score Evaluator::evaluate() const {
  PIECE_COLOR moving_side = m_chess_board.get_side_to_move();

  if (moving_side == PIECE_COLOR::WHITE) {
    return material_score<PIECE_COLOR::WHITE>();
  } else {
    return material_score<PIECE_COLOR::BLACK>();
  }
}
