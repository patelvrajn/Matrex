#pragma once

#include "chess_board.hpp"
#include "move_generator.hpp"
#include "score.hpp"

constexpr uint16_t PIECE_VALUES[] = {100, 300, 350, 500, 900};

class Evaluator {
 public:
  Evaluator(const Chess_Board& cb, const Moves_Bitboard_Matrix& matrix);

  Score evaluate() const;

  template <PIECE_COLOR moving_side>
  inline Score material_score() const;

 private:
  const Chess_Board& m_chess_board;
  const Moves_Bitboard_Matrix& m_moves_matrix;
};

template <PIECE_COLOR moving_side>
inline Score Evaluator::material_score() const {
  constexpr PIECE_COLOR opposing_side = (PIECE_COLOR)((~moving_side) & 0x1);

  Score return_value;

  for (uint8_t piece = PIECES::PAWN; piece <= PIECES::QUEEN; piece++) {
    return_value += Score::from_int(
        PIECE_VALUES[piece] *
        m_chess_board.get_piece_occupancies(moving_side, (PIECES)piece)
            .high_bit_count());
    return_value -= Score::from_int(
        PIECE_VALUES[piece] *
        m_chess_board.get_piece_occupancies(opposing_side, (PIECES)piece)
            .high_bit_count());
  }

  return return_value;
}
