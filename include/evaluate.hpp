#pragma once

#include "chess_board.hpp"
#include "move_generator.hpp"
#include "score.hpp"

template <typename T>
class Evaluation_Weights {
 public:
  Evaluation_Weights(
      multi_array<T, (NUM_OF_UNIQUE_PIECES_PER_PLAYER - 1)> material_weights,
      T diagonal_mobility_weight, T orthogonal_mobility_weight,
      T knight_movement_mobility_weight, T multi_movement_mobility_weight,
      T backwards_movement_mobility_weight)
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
  multi_array<T, (NUM_OF_UNIQUE_PIECES_PER_PLAYER - 1)> material;

  // Mobility weights.
  T diagonal_mobility;
  T orthogonal_mobility;
  T knight_movement_mobility;
  T multi_movement_mobility;
  T backwards_movement_mobility;

  T& operator[](std::size_t index);
  const T& operator[](std::size_t index) const;

 private:
  Reference_Array<T, multi_array<T, (NUM_OF_UNIQUE_PIECES_PER_PLAYER - 1)>, T,
                  T, T, T, T>
      m_weight_ref_array;
};

template <typename T>
T& Evaluation_Weights<T>::operator[](std::size_t index) {
  if (index >= m_weight_ref_array.size) {
    throw std::out_of_range("Index out of range in Evaluation_Weights");
  }
  return m_weight_ref_array.get_array()[index].value().get();
}

template <typename T>
const T& Evaluation_Weights<T>::operator[](std::size_t index) const {
  if (index >= m_weight_ref_array.size) {
    throw std::out_of_range("Index out of range in Evaluation_Weights");
  }
  return m_weight_ref_array.get_array()[index].value().get();
}

constexpr double NON_LINEAR_RESPONSE_EPSILON = 1e-8;
constexpr double NON_LINEAR_RESPONSE_T = 1e6;

class Non_Linear_Response {
 public:
  Non_Linear_Response(double h_plus_parameter, double h_minus_parameter,
                      double z_parameter, double k_parameter,
                      double q_plus_parameter, double q_minus_parameter,
                      double r_plus_parameter, double r_minus_parameter,
                      double g_plus_parameter, double g_minus_parameter);

  double value(double F) const;

  double partial_derivative_h_plus(double F) const;
  double partial_derivative_h_minus(double F) const;
  double partial_derivative_z(double F) const;
  double partial_derivative_k(double F) const;
  double partial_derivative_q_plus(double F) const;
  double partial_derivative_q_minus(double F) const;
  double partial_derivative_r_plus(double F) const;
  double partial_derivative_r_minus(double F) const;
  double partial_derivative_g_plus(double F) const;
  double partial_derivative_g_minus(double F) const;
  double partial_derivative_u(double F) const;

 private:
  double m_h_plus_parameter;
  double m_h_minus_parameter;
  double m_z_parameter;
  double m_k_parameter;
  double m_q_plus_parameter;
  double m_q_minus_parameter;
  double m_r_plus_parameter;
  double m_r_minus_parameter;
  double m_g_plus_parameter;
  double m_g_minus_parameter;

  double calculate_u(double F) const;

  double calculate_function_M(double x) const;
  double calculate_function_G(double F) const;
  double calculate_function_H(double F) const;
  double calculate_function_S(double F) const;
  double calculate_function_P_plus(double F) const;
  double calculate_function_P_minus(double F) const;
  double calculate_function_P(double F) const;
  double calculate_function_B_plus(double F) const;
  double calculate_function_B_minus(double F) const;
  double calculate_function_B(double F) const;
};

template <typename T>
class Evaluator {
 public:
  Evaluator(const Evaluation_Weights<T>& weights, const Chess_Board& cb,
            const Moves_Bitboard_Matrix& moving_side_matrix,
            const Moves_Bitboard_Matrix& opposing_side_matrix);

  Score evaluate() const;

  template <PIECE_COLOR moving_side>
  inline Score material_score() const;

 private:
  const Evaluation_Weights<T>& m_weights;
  const Chess_Board& m_chess_board;
  const Moves_Bitboard_Matrix& m_moving_side_matrix;
  const Moves_Bitboard_Matrix& m_opposing_side_matrix;
};

template <typename T>
Evaluator<T>::Evaluator(const Evaluation_Weights<T>& weights,
                        const Chess_Board& cb,
                        const Moves_Bitboard_Matrix& moving_side_matrix,
                        const Moves_Bitboard_Matrix& opposing_side_matrix)
    : m_weights(weights),
      m_chess_board(cb),
      m_moving_side_matrix(moving_side_matrix),
      m_opposing_side_matrix(opposing_side_matrix) {}

template <typename T>
Score Evaluator<T>::evaluate() const {
  PIECE_COLOR moving_side = m_chess_board.get_side_to_move();

  if (moving_side == PIECE_COLOR::WHITE) {
    return material_score<PIECE_COLOR::WHITE>();
  } else {
    return material_score<PIECE_COLOR::BLACK>();
  }
}

template <typename T>
template <PIECE_COLOR moving_side>
inline Score Evaluator<T>::material_score() const {
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
