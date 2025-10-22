#pragma once

#include "chess_board.hpp"
#include "score.hpp"

constexpr uint16_t PIECE_VALUES[] = {100, 300, 350, 500, 900};

class Evaluator {
 public:
  Evaluator(const Chess_Board& cb);

  Score evaluate();

  template <PIECE_COLOR moving_side>
  inline Score material_score();

 private:
  const Chess_Board& m_chess_board;
};

template <PIECE_COLOR moving_side>
inline Score Evaluator::material_score() {
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
