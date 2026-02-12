#pragma once

#include <cmath>

#include "chess_board.hpp"
#include "move_generator.hpp"
#include "score.hpp"

template <typename T>
struct NLR_Parameters {  // NLR = Non-Linear Response
  T h_plus;
  T h_minus;
  T z;
  T k;
  T q_plus;
  T q_minus;
  T r_plus;
  T r_minus;
  T g_plus;
  T g_minus;
};

#define NLR_ARRAY_FIELDS(arr, idx)                                           \
  (arr)[(idx)].h_plus, (arr)[(idx)].h_minus, (arr)[(idx)].z, (arr)[(idx)].k, \
      (arr)[(idx)].q_plus, (arr)[(idx)].q_minus, (arr)[(idx)].r_plus,        \
      (arr)[(idx)].r_minus, (arr)[(idx)].g_plus, (arr)[(idx)].g_minus

#define NLR_FIELDS(x)                                                         \
  (x).h_plus, (x).h_minus, (x).z, (x).k, (x).q_plus, (x).q_minus, (x).r_plus, \
      (x).r_minus, (x).g_plus, (x).g_minus

template <typename T>
class Evaluation_Weights {
  using Evaluation_Weights_Reference_Array = Reference_Array<
      T, multi_array<T, (NUM_OF_UNIQUE_PIECES_PER_PLAYER - 1)>, T, T, T, T, T,
      T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,
      T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,
      T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,
      T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,
      T, T, T, T, T, T, T, T, T, T>;

 public:
  Evaluation_Weights()
      : material{},
        material_NLR_parameters{},
        piece_mobility_NLR_parameters{},
        diagonal_mobility(T{}),
        orthogonal_mobility(T{}),
        knight_movement_mobility(T{}),
        multi_movement_mobility(T{}),
        backwards_movement_mobility(T{}),
        m_weight_ref_array(
            material, NLR_ARRAY_FIELDS(material_NLR_parameters, PIECES::PAWN),
            NLR_ARRAY_FIELDS(material_NLR_parameters, PIECES::KNIGHT),
            NLR_ARRAY_FIELDS(material_NLR_parameters, PIECES::BISHOP),
            NLR_ARRAY_FIELDS(material_NLR_parameters, PIECES::ROOK),
            NLR_ARRAY_FIELDS(material_NLR_parameters, PIECES::QUEEN),
            NLR_ARRAY_FIELDS(piece_mobility_NLR_parameters, PIECES::PAWN),
            NLR_ARRAY_FIELDS(piece_mobility_NLR_parameters, PIECES::KNIGHT),
            NLR_ARRAY_FIELDS(piece_mobility_NLR_parameters, PIECES::BISHOP),
            NLR_ARRAY_FIELDS(piece_mobility_NLR_parameters, PIECES::ROOK),
            NLR_ARRAY_FIELDS(piece_mobility_NLR_parameters, PIECES::QUEEN),
            NLR_ARRAY_FIELDS(piece_mobility_NLR_parameters, PIECES::KING),
            diagonal_mobility, orthogonal_mobility, knight_movement_mobility,
            multi_movement_mobility, backwards_movement_mobility) {}

  Evaluation_Weights(
      multi_array<T, (NUM_OF_UNIQUE_PIECES_PER_PLAYER - 1)> material_weights,
      multi_array<NLR_Parameters<T>, (NUM_OF_UNIQUE_PIECES_PER_PLAYER - 1)>
          material_NLR_weights,
      multi_array<NLR_Parameters<T>, NUM_OF_UNIQUE_PIECES_PER_PLAYER>
          piece_mobility_NLR_weights,
      T diagonal_mobility_weight, T orthogonal_mobility_weight,
      T knight_movement_mobility_weight, T multi_movement_mobility_weight,
      T backwards_movement_mobility_weight)
      : material(material_weights),
        material_NLR_parameters(material_NLR_weights),
        piece_mobility_NLR_parameters(piece_mobility_NLR_weights),
        diagonal_mobility(diagonal_mobility_weight),
        orthogonal_mobility(orthogonal_mobility_weight),
        knight_movement_mobility(knight_movement_mobility_weight),
        multi_movement_mobility(multi_movement_mobility_weight),
        backwards_movement_mobility(backwards_movement_mobility_weight),
        m_weight_ref_array(
            material, NLR_ARRAY_FIELDS(material_NLR_parameters, PIECES::PAWN),
            NLR_ARRAY_FIELDS(material_NLR_parameters, PIECES::KNIGHT),
            NLR_ARRAY_FIELDS(material_NLR_parameters, PIECES::BISHOP),
            NLR_ARRAY_FIELDS(material_NLR_parameters, PIECES::ROOK),
            NLR_ARRAY_FIELDS(material_NLR_parameters, PIECES::QUEEN),
            NLR_ARRAY_FIELDS(piece_mobility_NLR_parameters, PIECES::PAWN),
            NLR_ARRAY_FIELDS(piece_mobility_NLR_parameters, PIECES::KNIGHT),
            NLR_ARRAY_FIELDS(piece_mobility_NLR_parameters, PIECES::BISHOP),
            NLR_ARRAY_FIELDS(piece_mobility_NLR_parameters, PIECES::ROOK),
            NLR_ARRAY_FIELDS(piece_mobility_NLR_parameters, PIECES::QUEEN),
            NLR_ARRAY_FIELDS(piece_mobility_NLR_parameters, PIECES::KING),
            diagonal_mobility, orthogonal_mobility, knight_movement_mobility,
            multi_movement_mobility, backwards_movement_mobility) {}

  Evaluation_Weights(Evaluation_Weights_Reference_Array ref_array)
      : Evaluation_Weights()  // default construct first
  {
    // Copy the values from ref_array to m_weight_ref_array not the references.
    for (std::size_t i = 0; i < m_weight_ref_array.size; ++i) {
      m_weight_ref_array.get_array()[i].value().get() =
          ref_array.get_array()[i].value().get();
    }
  }

  // Material weights.
  multi_array<NLR_Parameters<T>, (NUM_OF_UNIQUE_PIECES_PER_PLAYER - 1)>
      material_NLR_parameters;
  multi_array<T, (NUM_OF_UNIQUE_PIECES_PER_PLAYER - 1)> material;

  // Mobility weights.
  multi_array<NLR_Parameters<T>, NUM_OF_UNIQUE_PIECES_PER_PLAYER>
      piece_mobility_NLR_parameters;
  T diagonal_mobility;
  T orthogonal_mobility;
  T knight_movement_mobility;
  T multi_movement_mobility;
  T backwards_movement_mobility;

  T& operator[](std::size_t index);
  const T& operator[](std::size_t index) const;

  // Arithmetic operators with another evaluation weights.
  Evaluation_Weights operator+(const Evaluation_Weights& other) const;
  Evaluation_Weights operator-(const Evaluation_Weights& other) const;
  Evaluation_Weights operator/(const Evaluation_Weights& other) const;
  Evaluation_Weights operator*(const Evaluation_Weights& other) const;

  // Arithmetic operators with T.
  Evaluation_Weights operator+(T value) const;
  Evaluation_Weights operator-(T value) const;
  Evaluation_Weights operator*(T value) const;
  Evaluation_Weights operator/(T value) const;

  template <typename U>
  std::size_t get_index_of(const U& ref) const;

  std::size_t get_size() const;

 private:
  Evaluation_Weights_Reference_Array m_weight_ref_array;
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

template <typename T>
Evaluation_Weights<T> Evaluation_Weights<T>::operator+(
    const Evaluation_Weights& other) const {
  Evaluation_Weights result;

  for (std::size_t i = 0; i < m_weight_ref_array.size; ++i) {
    result[i] = m_weight_ref_array.get_array()[i].value().get() +
                other.m_weight_ref_array.get_array()[i].value().get();
  }

  return result;
}

template <typename T>
Evaluation_Weights<T> Evaluation_Weights<T>::operator-(
    const Evaluation_Weights& other) const {
  Evaluation_Weights result;

  for (std::size_t i = 0; i < m_weight_ref_array.size; ++i) {
    result[i] = m_weight_ref_array.get_array()[i].value().get() -
                other.m_weight_ref_array.get_array()[i].value().get();
  }

  return result;
}

template <typename T>
Evaluation_Weights<T> Evaluation_Weights<T>::operator/(
    const Evaluation_Weights& other) const {
  Evaluation_Weights result;

  for (std::size_t i = 0; i < m_weight_ref_array.size; ++i) {
    result[i] = m_weight_ref_array.get_array()[i].value().get() /
                other.m_weight_ref_array.get_array()[i].value().get();
  }

  return result;
}

template <typename T>
Evaluation_Weights<T> Evaluation_Weights<T>::operator*(
    const Evaluation_Weights& other) const {
  Evaluation_Weights result;

  for (std::size_t i = 0; i < m_weight_ref_array.size; ++i) {
    result[i] = m_weight_ref_array.get_array()[i].value().get() *
                other.m_weight_ref_array.get_array()[i].value().get();
  }

  return result;
}

template <typename T>
Evaluation_Weights<T> Evaluation_Weights<T>::operator+(T value) const {
  Evaluation_Weights result;

  for (std::size_t i = 0; i < m_weight_ref_array.size; ++i) {
    result[i] = m_weight_ref_array.get_array()[i].value().get() + value;
  }

  return result;
}

template <typename T>
Evaluation_Weights<T> Evaluation_Weights<T>::operator-(T value) const {
  Evaluation_Weights result;

  for (std::size_t i = 0; i < m_weight_ref_array.size; ++i) {
    result[i] = m_weight_ref_array.get_array()[i].value().get() - value;
  }

  return result;
}

template <typename T>
Evaluation_Weights<T> Evaluation_Weights<T>::operator*(T value) const {
  Evaluation_Weights result;

  for (std::size_t i = 0; i < m_weight_ref_array.size; ++i) {
    result[i] = m_weight_ref_array.get_array()[i].value().get() * value;
  }

  return result;
}

template <typename T>
Evaluation_Weights<T> Evaluation_Weights<T>::operator/(T value) const {
  Evaluation_Weights result;

  for (std::size_t i = 0; i < m_weight_ref_array.size; ++i) {
    result[i] = m_weight_ref_array.get_array()[i].value().get() / value;
  }

  return result;
}

// Example use case: weights.get_index_of(weights.diagonal_mobility);
template <typename T>
template <typename U>
std::size_t Evaluation_Weights<T>::get_index_of(const U& ref) const {
  return m_weight_ref_array.get_index_of(ref);
}

template <typename T>
std::size_t Evaluation_Weights<T>::get_size() const {
  return m_weight_ref_array.size;
}

constexpr double NON_LINEAR_RESPONSE_EPSILON = 1e-8;
constexpr double NON_LINEAR_RESPONSE_T = 1e6;

class Non_Linear_Response {
 public:
  template <typename T>
  Non_Linear_Response(NLR_Parameters<T> params);

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
  Evaluation_Weights<T> derivative_evaluate() const;

  template <PIECE_COLOR moving_side>
  inline Score material_score() const;

  template <PIECE_COLOR moving_side>
  inline Evaluation_Weights<T> derivative_material_score() const;

  template <PIECE_COLOR moving_side>
  inline Score mobility_score() const;

  template <PIECE_COLOR moving_side>
  inline Evaluation_Weights<T> derivative_mobility_score() const;

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

  Score material;
  Score mobility;

  if (moving_side == PIECE_COLOR::WHITE) {
    material = material_score<PIECE_COLOR::WHITE>();
    mobility = mobility_score<PIECE_COLOR::WHITE>();
  } else {
    material = material_score<PIECE_COLOR::BLACK>();
    mobility = mobility_score<PIECE_COLOR::BLACK>();
  }

  Score evaluation = material + mobility;

  return evaluation;
}

template <typename T>
Evaluation_Weights<T> Evaluator<T>::derivative_evaluate() const {
  PIECE_COLOR moving_side = m_chess_board.get_side_to_move();

  Evaluation_Weights<T> material;
  Evaluation_Weights<T> mobility;

  if (moving_side == PIECE_COLOR::WHITE) {
    material = derivative_material_score<PIECE_COLOR::WHITE>();
    mobility = derivative_mobility_score<PIECE_COLOR::WHITE>();
  } else {
    material = derivative_material_score<PIECE_COLOR::BLACK>();
    mobility = derivative_mobility_score<PIECE_COLOR::BLACK>();
  }

  Evaluation_Weights<T> evaluation = material + mobility;

  return evaluation;
}

template <typename T>
template <PIECE_COLOR moving_side>
inline Score Evaluator<T>::material_score() const {
  constexpr PIECE_COLOR opposing_side = (PIECE_COLOR)((~moving_side) & 0x1);

  double return_value;

  for (uint8_t piece = PIECES::PAWN; piece <= PIECES::QUEEN; piece++) {
    T material_difference = 0;
    material_difference +=
        (m_weights.material[piece] *
         m_chess_board.get_piece_occupancies(moving_side, (PIECES)piece)
             .high_bit_count());
    material_difference -=
        (m_weights.material[piece] *
         m_chess_board.get_piece_occupancies(opposing_side, (PIECES)piece)
             .high_bit_count());
    double non_linear_material =
        Non_Linear_Response(m_weights.material_NLR_parameters[piece])
            .value(material_difference);
    return_value += non_linear_material;
  }

  return Score(return_value);
}

template <typename T>
template <PIECE_COLOR moving_side>
inline Evaluation_Weights<T> Evaluator<T>::derivative_material_score() const {
  constexpr PIECE_COLOR opposing_side = (PIECE_COLOR)((~moving_side) & 0x1);

  Evaluation_Weights<T> grad = m_weights;

  for (uint8_t piece = PIECES::PAWN; piece <= PIECES::QUEEN; ++piece) {
    const int c_plus =
        (int)m_chess_board.get_piece_occupancies(moving_side, (PIECES)piece)
            .high_bit_count();

    const int c_minus =
        (int)m_chess_board.get_piece_occupancies(opposing_side, (PIECES)piece)
            .high_bit_count();

    const int delta = c_plus - c_minus;
    const T w = m_weights.material[piece];
    const T F = w * (T)delta;

    Non_Linear_Response nlr(m_weights.material_NLR_parameters[piece]);

    const T dYdF = (T)nlr.partial_derivative_u((double)F);

    // 1) Gradient w.r.t. linear material weight w_p
    {
      const std::size_t idx_w =
          m_weights.get_index_of(m_weights.material[piece]);
      grad[idx_w] += dYdF * (T)delta;
    }

    // 2) Gradients w.r.t. this piece's NLR parameters
    {
      const auto& p = m_weights.material_NLR_parameters[piece];

      grad[m_weights.get_index_of(p.h_plus)] +=
          (T)nlr.partial_derivative_h_plus((double)F);
      grad[m_weights.get_index_of(p.h_minus)] +=
          (T)nlr.partial_derivative_h_minus((double)F);
      grad[m_weights.get_index_of(p.z)] +=
          (T)nlr.partial_derivative_z((double)F);
      grad[m_weights.get_index_of(p.k)] +=
          (T)nlr.partial_derivative_k((double)F);
      grad[m_weights.get_index_of(p.q_plus)] +=
          (T)nlr.partial_derivative_q_plus((double)F);
      grad[m_weights.get_index_of(p.q_minus)] +=
          (T)nlr.partial_derivative_q_minus((double)F);
      grad[m_weights.get_index_of(p.r_plus)] +=
          (T)nlr.partial_derivative_r_plus((double)F);
      grad[m_weights.get_index_of(p.r_minus)] +=
          (T)nlr.partial_derivative_r_minus((double)F);
      grad[m_weights.get_index_of(p.g_plus)] +=
          (T)nlr.partial_derivative_g_plus((double)F);
      grad[m_weights.get_index_of(p.g_minus)] +=
          (T)nlr.partial_derivative_g_minus((double)F);
    }
  }

  return grad;
}

template <typename T>
template <PIECE_COLOR moving_side>
inline Score Evaluator<T>::mobility_score() const {
  constexpr PIECE_COLOR opposing_side = (PIECE_COLOR)((~moving_side) & 0x1);

  Attacks a;
  double mobility = 0;

  for (uint8_t piece = PIECES::PAWN; piece <= PIECES::KING; piece++) {
    std::vector<Moves_Bitboard> moving_side_moves_bitboards;
    m_moving_side_matrix.get_piece_moves_bitboards(moving_side, (PIECES)piece,
                                                   moving_side_moves_bitboards);
    T moving_side_piece_mobility = 0;
    for (Moves_Bitboard& mb : moving_side_moves_bitboards) {
      const Bitboard diagonal_movements =
          mb.bitboard &
          (a.get_bishop_attacks(mb.square,
                                m_chess_board.get_both_color_occupancies()));
      const Bitboard orthogonal_movements =
          mb.bitboard &
          (a.get_rook_attacks(mb.square,
                              m_chess_board.get_both_color_occupancies()));
      const Bitboard backward_movements =
          mb.bitboard.get_backward_squares_mask(mb.square, moving_side);

      const T diagonal_mobility =
          diagonal_movements.high_bit_count() * m_weights.diagonal_mobility;
      const T orthogonal_mobility =
          orthogonal_movements.high_bit_count() * m_weights.orthogonal_mobility;
      const T backward_mobility = backward_movements.high_bit_count() *
                                  m_weights.backwards_movement_mobility;
      const T multi_movement_mobility =
          ((diagonal_movements.high_bit_count() > 0) &&
           (orthogonal_movements.high_bit_count() > 0)) *
          m_weights.multi_movement_mobility;
      const T knight_movements_mobility = mb.bitboard.high_bit_count() *
                                          (piece == PIECES::KNIGHT) *
                                          m_weights.knight_movement_mobility;

      moving_side_piece_mobility +=
          (diagonal_mobility + orthogonal_mobility + backward_mobility +
           multi_movement_mobility + knight_movements_mobility);
    }
    mobility +=
        Non_Linear_Response(m_weights.piece_mobility_NLR_parameters[piece])
            .value(moving_side_piece_mobility);
  }

  return Score(mobility);
}

template <typename T>
template <PIECE_COLOR moving_side>
inline Evaluation_Weights<T> Evaluator<T>::derivative_mobility_score() const {
  constexpr PIECE_COLOR opposing_side = (PIECE_COLOR)((~moving_side) & 0x1);

  Attacks a;
  Evaluation_Weights<T> derivative_weights;

  struct Counts {
    uint16_t diagonal_movement = 0;
    uint16_t orthogonal_movement = 0;
    uint16_t knight_movement = 0;
    uint16_t multi_movement = 0;
    uint16_t backwards_movement = 0;
  };

  Counts piece_movement_counts[NUM_OF_UNIQUE_PIECES_PER_PLAYER];

  for (uint8_t piece = PIECES::PAWN; piece <= PIECES::KING; piece++) {
    std::vector<Moves_Bitboard> moving_side_moves_bitboards;
    m_moving_side_matrix.get_piece_moves_bitboards(moving_side, (PIECES)piece,
                                                   moving_side_moves_bitboards);
    Score moving_side_piece_mobility_score = Score::from_int(0);
    for (Moves_Bitboard& mb : moving_side_moves_bitboards) {
      const uint16_t diagonal_movements =
          (mb.bitboard &
           (a.get_bishop_attacks(mb.square,
                                 m_chess_board.get_both_color_occupancies())))
              .high_bit_count();
      const uint16_t orthogonal_movements =
          (mb.bitboard &
           (a.get_rook_attacks(mb.square,
                               m_chess_board.get_both_color_occupancies())))
              .high_bit_count();
      const uint16_t backward_movements =
          mb.bitboard.get_backward_squares_mask(mb.square, moving_side)
              .high_bit_count();
      const uint16_t multi_movement_mobility_score =
          ((diagonal_movements > 0) && (orthogonal_movements > 0));
      const uint16_t knight_movements =
          mb.bitboard.high_bit_count() * (piece == PIECES::KNIGHT);

      piece_movement_counts[piece].diagonal_movement += diagonal_movements;
      piece_movement_counts[piece].orthogonal_movement += orthogonal_movements;
      piece_movement_counts[piece].backwards_movement += backward_movements;
      piece_movement_counts[piece].multi_movement +=
          multi_movement_mobility_score;
      piece_movement_counts[piece].knight_movement += knight_movements;
    }
  }

  T f_k = piece_movement_counts[PIECES::KING].diagonal_movement *
              m_weights.diagonal_mobility +
          piece_movement_counts[PIECES::KING].orthogonal_movement *
              m_weights.orthogonal_mobility +
          piece_movement_counts[PIECES::KING].multi_movement *
              m_weights.multi_movement_mobility +
          piece_movement_counts[PIECES::KING].backwards_movement *
              m_weights.backwards_movement_mobility;
  T f_q = piece_movement_counts[PIECES::QUEEN].diagonal_movement *
              m_weights.diagonal_mobility +
          piece_movement_counts[PIECES::QUEEN].orthogonal_movement *
              m_weights.orthogonal_mobility +
          piece_movement_counts[PIECES::QUEEN].multi_movement *
              m_weights.multi_movement_mobility +
          piece_movement_counts[PIECES::QUEEN].backwards_movement *
              m_weights.backwards_movement_mobility;
  T f_r = piece_movement_counts[PIECES::ROOK].orthogonal_movement *
              m_weights.orthogonal_mobility +
          piece_movement_counts[PIECES::ROOK].backwards_movement *
              m_weights.backwards_movement_mobility;
  T f_b = piece_movement_counts[PIECES::BISHOP].diagonal_movement *
              m_weights.diagonal_mobility +
          piece_movement_counts[PIECES::BISHOP].backwards_movement *
              m_weights.backwards_movement_mobility;
  T f_n = piece_movement_counts[PIECES::KNIGHT].knight_movement *
              m_weights.knight_movement_mobility +
          piece_movement_counts[PIECES::KNIGHT].backwards_movement *
              m_weights.backwards_movement_mobility;
  T f_p = piece_movement_counts[PIECES::PAWN].diagonal_movement *
              m_weights.diagonal_mobility +
          piece_movement_counts[PIECES::PAWN].orthogonal_movement *
              m_weights.orthogonal_mobility +
          piece_movement_counts[PIECES::PAWN].multi_movement *
              m_weights.multi_movement_mobility;

  Non_Linear_Response nlr_king(
      m_weights.piece_mobility_NLR_parameters[PIECES::KING]);
  Non_Linear_Response nlr_queen(
      m_weights.piece_mobility_NLR_parameters[PIECES::QUEEN]);
  Non_Linear_Response nlr_rook(
      m_weights.piece_mobility_NLR_parameters[PIECES::ROOK]);
  Non_Linear_Response nlr_bishop(
      m_weights.piece_mobility_NLR_parameters[PIECES::BISHOP]);
  Non_Linear_Response nlr_knight(
      m_weights.piece_mobility_NLR_parameters[PIECES::KNIGHT]);
  Non_Linear_Response nlr_pawn(
      m_weights.piece_mobility_NLR_parameters[PIECES::PAWN]);

  for (uint64_t i = 0; i < derivative_weights.get_size(); i++) {
    if (i == m_weights.get_index_of(m_weights.diagonal_mobility)) {
      derivative_weights[i] =
          nlr_king.partial_derivative_u(f_k) *
              piece_movement_counts[PIECES::KING].diagonal_movement +
          nlr_queen.partial_derivative_u(f_q) *
              piece_movement_counts[PIECES::QUEEN].diagonal_movement +
          nlr_bishop.partial_derivative_u(f_b) *
              piece_movement_counts[PIECES::BISHOP].diagonal_movement +
          nlr_pawn.partial_derivative_u(f_p) *
              piece_movement_counts[PIECES::PAWN].diagonal_movement;
    } else if (i == m_weights.get_index_of(m_weights.orthogonal_mobility)) {
      derivative_weights[i] =
          nlr_king.partial_derivative_u(f_k) *
              piece_movement_counts[PIECES::KING].orthogonal_movement +
          nlr_queen.partial_derivative_u(f_q) *
              piece_movement_counts[PIECES::QUEEN].orthogonal_movement +
          nlr_rook.partial_derivative_u(f_r) *
              piece_movement_counts[PIECES::ROOK].orthogonal_movement +
          nlr_pawn.partial_derivative_u(f_p) *
              piece_movement_counts[PIECES::PAWN].orthogonal_movement;
    } else if (i ==
               m_weights.get_index_of(m_weights.knight_movement_mobility)) {
      derivative_weights[i] =
          nlr_knight.partial_derivative_u(f_n) *
          piece_movement_counts[PIECES::KNIGHT].knight_movement;
    } else if (i == m_weights.get_index_of(m_weights.multi_movement_mobility)) {
      derivative_weights[i] =
          nlr_king.partial_derivative_u(f_k) *
              piece_movement_counts[PIECES::KING].multi_movement +
          nlr_queen.partial_derivative_u(f_q) *
              piece_movement_counts[PIECES::QUEEN].multi_movement +
          nlr_pawn.partial_derivative_u(f_p) *
              piece_movement_counts[PIECES::PAWN].multi_movement;
    } else if (i ==
               m_weights.get_index_of(m_weights.backwards_movement_mobility)) {
      derivative_weights[i] =
          nlr_king.partial_derivative_u(f_k) *
              piece_movement_counts[PIECES::KING].backwards_movement +
          nlr_queen.partial_derivative_u(f_q) *
              piece_movement_counts[PIECES::QUEEN].backwards_movement +
          nlr_rook.partial_derivative_u(f_r) *
              piece_movement_counts[PIECES::ROOK].backwards_movement +
          nlr_bishop.partial_derivative_u(f_b) *
              piece_movement_counts[PIECES::BISHOP].backwards_movement +
          nlr_knight.partial_derivative_u(f_n) *
              piece_movement_counts[PIECES::KNIGHT].backwards_movement;
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::KING]
                            .h_plus)) {
      derivative_weights[i] = nlr_king.partial_derivative_h_plus(f_k);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::KING]
                            .h_minus)) {
      derivative_weights[i] = nlr_king.partial_derivative_h_minus(f_k);
    } else if (i ==
               m_weights.get_index_of(
                   m_weights.piece_mobility_NLR_parameters[PIECES::KING].z)) {
      derivative_weights[i] = nlr_king.partial_derivative_z(f_k);
    } else if (i ==
               m_weights.get_index_of(
                   m_weights.piece_mobility_NLR_parameters[PIECES::KING].k)) {
      derivative_weights[i] = nlr_king.partial_derivative_k(f_k);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::KING]
                            .q_plus)) {
      derivative_weights[i] = nlr_king.partial_derivative_q_plus(f_k);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::KING]
                            .q_minus)) {
      derivative_weights[i] = nlr_king.partial_derivative_q_minus(f_k);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::KING]
                            .r_plus)) {
      derivative_weights[i] = nlr_king.partial_derivative_r_plus(f_k);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::KING]
                            .r_minus)) {
      derivative_weights[i] = nlr_king.partial_derivative_r_minus(f_k);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::KING]
                            .g_plus)) {
      derivative_weights[i] = nlr_king.partial_derivative_g_plus(f_k);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::KING]
                            .g_minus)) {
      derivative_weights[i] = nlr_king.partial_derivative_g_minus(f_k);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::QUEEN]
                            .h_plus)) {
      derivative_weights[i] = nlr_queen.partial_derivative_h_plus(f_q);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::QUEEN]
                            .h_minus)) {
      derivative_weights[i] = nlr_queen.partial_derivative_h_minus(f_q);
    } else if (i ==
               m_weights.get_index_of(
                   m_weights.piece_mobility_NLR_parameters[PIECES::QUEEN].z)) {
      derivative_weights[i] = nlr_queen.partial_derivative_z(f_q);
    } else if (i ==
               m_weights.get_index_of(
                   m_weights.piece_mobility_NLR_parameters[PIECES::QUEEN].k)) {
      derivative_weights[i] = nlr_queen.partial_derivative_k(f_q);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::QUEEN]
                            .q_plus)) {
      derivative_weights[i] = nlr_queen.partial_derivative_q_plus(f_q);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::QUEEN]
                            .q_minus)) {
      derivative_weights[i] = nlr_queen.partial_derivative_q_minus(f_q);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::QUEEN]
                            .r_plus)) {
      derivative_weights[i] = nlr_queen.partial_derivative_r_plus(f_q);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::QUEEN]
                            .r_minus)) {
      derivative_weights[i] = nlr_queen.partial_derivative_r_minus(f_q);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::QUEEN]
                            .g_plus)) {
      derivative_weights[i] = nlr_queen.partial_derivative_g_plus(f_q);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::QUEEN]
                            .g_minus)) {
      derivative_weights[i] = nlr_queen.partial_derivative_g_minus(f_q);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::ROOK]
                            .h_plus)) {
      derivative_weights[i] = nlr_rook.partial_derivative_h_plus(f_r);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::ROOK]
                            .h_minus)) {
      derivative_weights[i] = nlr_rook.partial_derivative_h_minus(f_r);
    } else if (i ==
               m_weights.get_index_of(
                   m_weights.piece_mobility_NLR_parameters[PIECES::ROOK].z)) {
      derivative_weights[i] = nlr_rook.partial_derivative_z(f_r);
    } else if (i ==
               m_weights.get_index_of(
                   m_weights.piece_mobility_NLR_parameters[PIECES::ROOK].k)) {
      derivative_weights[i] = nlr_rook.partial_derivative_k(f_r);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::ROOK]
                            .q_plus)) {
      derivative_weights[i] = nlr_rook.partial_derivative_q_plus(f_r);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::ROOK]
                            .q_minus)) {
      derivative_weights[i] = nlr_rook.partial_derivative_q_minus(f_r);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::ROOK]
                            .r_plus)) {
      derivative_weights[i] = nlr_rook.partial_derivative_r_plus(f_r);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::ROOK]
                            .r_minus)) {
      derivative_weights[i] = nlr_rook.partial_derivative_r_minus(f_r);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::ROOK]
                            .g_plus)) {
      derivative_weights[i] = nlr_rook.partial_derivative_g_plus(f_r);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::ROOK]
                            .g_minus)) {
      derivative_weights[i] = nlr_rook.partial_derivative_g_minus(f_r);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::BISHOP]
                            .h_plus)) {
      derivative_weights[i] = nlr_bishop.partial_derivative_h_plus(f_b);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::BISHOP]
                            .h_minus)) {
      derivative_weights[i] = nlr_bishop.partial_derivative_h_minus(f_b);
    } else if (i ==
               m_weights.get_index_of(
                   m_weights.piece_mobility_NLR_parameters[PIECES::BISHOP].z)) {
      derivative_weights[i] = nlr_bishop.partial_derivative_z(f_b);
    } else if (i ==
               m_weights.get_index_of(
                   m_weights.piece_mobility_NLR_parameters[PIECES::BISHOP].k)) {
      derivative_weights[i] = nlr_bishop.partial_derivative_k(f_b);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::BISHOP]
                            .q_plus)) {
      derivative_weights[i] = nlr_bishop.partial_derivative_q_plus(f_b);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::BISHOP]
                            .q_minus)) {
      derivative_weights[i] = nlr_bishop.partial_derivative_q_minus(f_b);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::BISHOP]
                            .r_plus)) {
      derivative_weights[i] = nlr_bishop.partial_derivative_r_plus(f_b);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::BISHOP]
                            .r_minus)) {
      derivative_weights[i] = nlr_bishop.partial_derivative_r_minus(f_b);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::BISHOP]
                            .g_plus)) {
      derivative_weights[i] = nlr_bishop.partial_derivative_g_plus(f_b);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::BISHOP]
                            .g_minus)) {
      derivative_weights[i] = nlr_bishop.partial_derivative_g_minus(f_b);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::KNIGHT]
                            .h_plus)) {
      derivative_weights[i] = nlr_knight.partial_derivative_h_plus(f_n);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::KNIGHT]
                            .h_minus)) {
      derivative_weights[i] = nlr_knight.partial_derivative_h_minus(f_n);
    } else if (i ==
               m_weights.get_index_of(
                   m_weights.piece_mobility_NLR_parameters[PIECES::KNIGHT].z)) {
      derivative_weights[i] = nlr_knight.partial_derivative_z(f_n);
    } else if (i ==
               m_weights.get_index_of(
                   m_weights.piece_mobility_NLR_parameters[PIECES::KNIGHT].k)) {
      derivative_weights[i] = nlr_knight.partial_derivative_k(f_n);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::KNIGHT]
                            .q_plus)) {
      derivative_weights[i] = nlr_knight.partial_derivative_q_plus(f_n);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::KNIGHT]
                            .q_minus)) {
      derivative_weights[i] = nlr_knight.partial_derivative_q_minus(f_n);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::KNIGHT]
                            .r_plus)) {
      derivative_weights[i] = nlr_knight.partial_derivative_r_plus(f_n);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::KNIGHT]
                            .r_minus)) {
      derivative_weights[i] = nlr_knight.partial_derivative_r_minus(f_n);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::KNIGHT]
                            .g_plus)) {
      derivative_weights[i] = nlr_knight.partial_derivative_g_plus(f_n);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::KNIGHT]
                            .g_minus)) {
      derivative_weights[i] = nlr_knight.partial_derivative_g_minus(f_n);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::PAWN]
                            .h_plus)) {
      derivative_weights[i] = nlr_pawn.partial_derivative_h_plus(f_p);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::PAWN]
                            .h_minus)) {
      derivative_weights[i] = nlr_pawn.partial_derivative_h_minus(f_p);
    } else if (i ==
               m_weights.get_index_of(
                   m_weights.piece_mobility_NLR_parameters[PIECES::PAWN].z)) {
      derivative_weights[i] = nlr_pawn.partial_derivative_z(f_p);
    } else if (i ==
               m_weights.get_index_of(
                   m_weights.piece_mobility_NLR_parameters[PIECES::PAWN].k)) {
      derivative_weights[i] = nlr_pawn.partial_derivative_k(f_p);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::PAWN]
                            .q_plus)) {
      derivative_weights[i] = nlr_pawn.partial_derivative_q_plus(f_p);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::PAWN]
                            .q_minus)) {
      derivative_weights[i] = nlr_pawn.partial_derivative_q_minus(f_p);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::PAWN]
                            .r_plus)) {
      derivative_weights[i] = nlr_pawn.partial_derivative_r_plus(f_p);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::PAWN]
                            .r_minus)) {
      derivative_weights[i] = nlr_pawn.partial_derivative_r_minus(f_p);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::PAWN]
                            .g_plus)) {
      derivative_weights[i] = nlr_pawn.partial_derivative_g_plus(f_p);
    } else if (i == m_weights.get_index_of(
                        m_weights.piece_mobility_NLR_parameters[PIECES::PAWN]
                            .g_minus)) {
      derivative_weights[i] = nlr_pawn.partial_derivative_g_minus(f_p);
    } else {
      derivative_weights[i] = 0;
    }
  }

  return derivative_weights;
}
