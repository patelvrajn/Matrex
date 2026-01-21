#include "evaluate.hpp"

#include <cmath>

Non_Linear_Response::Non_Linear_Response(uint64_t S_Parameter,
                                         uint64_t M_Parameter)
    : m_S_parameter(S_Parameter), m_M_parameter(M_Parameter) {}

uint64_t Non_Linear_Response::value(uint64_t x) {
  const double x_squared = (x * x);

  const double a = x;
  const double b = std::pow((1.0L + x_squared), -0.4L);
  const double c = std::tanh(x_squared / (m_M_parameter * m_M_parameter));

  return static_cast<uint64_t>(static_cast<double>(m_S_parameter) * a * b * c);
}

uint64_t Non_Linear_Response::derivative(uint64_t x) {
  const double x_squared = (x * x);
  const double M_squared_inv = 1 / (m_M_parameter * m_M_parameter);

  const double a = x;
  const double b = std::pow((1.0L + x_squared), -0.4L);
  const double c = std::tanh(x_squared * M_squared_inv);

  constexpr double a_prime = 1.0L;
  const double b_prime = -0.8L * x * std::pow((1.0L + x_squared), -1.4L);
  const double c_prime = (2.0L * x * M_squared_inv) * (1.0L - (c * c));

  return static_cast<uint64_t>(
      static_cast<double>(m_S_parameter) *
      ((a_prime * b * c) + (a * b_prime * c) + (a * b * c_prime)));
}

Evaluator::Evaluator(const Chess_Board& cb,
                     const Moves_Bitboard_Matrix& moving_side_matrix,
                     const Moves_Bitboard_Matrix& opposing_side_matrix)
    : m_chess_board(cb),
      m_moving_side_matrix(moving_side_matrix),
      m_opposing_side_matrix(opposing_side_matrix) {}

Score Evaluator::evaluate() const {
  PIECE_COLOR moving_side = m_chess_board.get_side_to_move();

  if (moving_side == PIECE_COLOR::WHITE) {
    return material_score<PIECE_COLOR::WHITE>();
  } else {
    return material_score<PIECE_COLOR::BLACK>();
  }
}
