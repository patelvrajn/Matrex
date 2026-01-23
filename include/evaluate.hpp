#pragma once

#include "chess_board.hpp"
#include "move_generator.hpp"
#include "score.hpp"

class Evaluation_Weights {
 public:
  Evaluation_Weights(
      multi_array<uint16_t, (NUM_OF_UNIQUE_PIECES_PER_PLAYER - 1)>
          material_weights,
      uint16_t diagonal_mobility_weight, uint16_t orthogonal_mobility_weight,
      uint16_t knight_movement_mobility_weight,
      uint16_t multi_movement_mobility_weight,
      uint16_t backwards_movement_mobility_weight)
      : material(material_weights),
        diagonal_mobility(diagonal_mobility_weight),
        orthogonal_mobility(orthogonal_mobility_weight),
        knight_movement_mobility(knight_movement_mobility_weight),
        multi_movement_mobility(multi_movement_mobility_weight),
        backwards_movement_mobility(backwards_movement_mobility_weight),
        m_weight_ref_array(material, diagonal_mobility, orthogonal_mobility,
                           knight_movement_mobility, multi_movement_mobility,
                           backwards_movement_mobility) {}

  // Material weights.
  multi_array<uint16_t, (NUM_OF_UNIQUE_PIECES_PER_PLAYER - 1)> material;

  // Mobility weights.
  uint16_t diagonal_mobility;
  uint16_t orthogonal_mobility;
  uint16_t knight_movement_mobility;
  uint16_t multi_movement_mobility;
  uint16_t backwards_movement_mobility;

  uint16_t& operator[](std::size_t index);
  const uint16_t& operator[](std::size_t index) const;

 private:
  Reference_Array<multi_array<uint16_t, (NUM_OF_UNIQUE_PIECES_PER_PLAYER - 1)>,
                  uint16_t, uint16_t, uint16_t, uint16_t, uint16_t>
      m_weight_ref_array;
};

class Non_Linear_Response {
 public:
  Non_Linear_Response(uint64_t S_Parameter, uint64_t M_Parameter);
  uint64_t value(uint64_t x);
  uint64_t derivative(uint64_t x);

 private:
  uint64_t m_S_parameter;
  uint64_t m_M_parameter;
};

class Evaluator {
 public:
  Evaluator(const Evaluation_Weights& weights, const Chess_Board& cb,
            const Moves_Bitboard_Matrix& moving_side_matrix,
            const Moves_Bitboard_Matrix& opposing_side_matrix);

  Score evaluate() const;

  template <PIECE_COLOR moving_side>
  inline Score material_score() const;

 private:
  const Evaluation_Weights& m_weights;
  const Chess_Board& m_chess_board;
  const Moves_Bitboard_Matrix& m_moving_side_matrix;
  const Moves_Bitboard_Matrix& m_opposing_side_matrix;
};

template <PIECE_COLOR moving_side>
inline Score Evaluator::material_score() const {
  constexpr PIECE_COLOR opposing_side = (PIECE_COLOR)((~moving_side) & 0x1);

  Score return_value;

  for (uint8_t piece = PIECES::PAWN; piece <= PIECES::QUEEN; piece++) {
    return_value += Score::from_int(
        m_weights.material[piece] *
        m_chess_board.get_piece_occupancies(moving_side, (PIECES)piece)
            .high_bit_count());
    return_value -= Score::from_int(
        m_weights.material[piece] *
        m_chess_board.get_piece_occupancies(opposing_side, (PIECES)piece)
            .high_bit_count());
  }

  return return_value;
}
