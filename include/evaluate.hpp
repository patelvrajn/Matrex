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

  T king_mobility_h_plus_parameter;
  T king_mobility_h_minus_parameter;
  T king_mobility_z_parameter;
  T king_mobility_k_parameter;
  T king_mobility_q_plus_parameter;
  T king_mobility_q_minus_parameter;
  T king_mobility_r_plus_parameter;
  T king_mobility_r_minus_parameter;
  T king_mobility_g_plus_parameter;
  T king_mobility_g_minus_parameter;

  T queen_mobility_h_plus_parameter;
  T queen_mobility_h_minus_parameter;
  T queen_mobility_z_parameter;
  T queen_mobility_k_parameter;
  T queen_mobility_q_plus_parameter;
  T queen_mobility_q_minus_parameter;
  T queen_mobility_r_plus_parameter;
  T queen_mobility_r_minus_parameter;
  T queen_mobility_g_plus_parameter;
  T queen_mobility_g_minus_parameter;

  T rook_mobility_h_plus_parameter;
  T rook_mobility_h_minus_parameter;
  T rook_mobility_z_parameter;
  T rook_mobility_k_parameter;
  T rook_mobility_q_plus_parameter;
  T rook_mobility_q_minus_parameter;
  T rook_mobility_r_plus_parameter;
  T rook_mobility_r_minus_parameter;
  T rook_mobility_g_plus_parameter;
  T rook_mobility_g_minus_parameter;

  T bishop_mobility_h_plus_parameter;
  T bishop_mobility_h_minus_parameter;
  T bishop_mobility_z_parameter;
  T bishop_mobility_k_parameter;
  T bishop_mobility_q_plus_parameter;
  T bishop_mobility_q_minus_parameter;
  T bishop_mobility_r_plus_parameter;
  T bishop_mobility_r_minus_parameter;
  T bishop_mobility_g_plus_parameter;
  T bishop_mobility_g_minus_parameter;

  T knight_mobility_h_plus_parameter;
  T knight_mobility_h_minus_parameter;
  T knight_mobility_z_parameter;
  T knight_mobility_k_parameter;
  T knight_mobility_q_plus_parameter;
  T knight_mobility_q_minus_parameter;
  T knight_mobility_r_plus_parameter;
  T knight_mobility_r_minus_parameter;
  T knight_mobility_g_plus_parameter;
  T knight_mobility_g_minus_parameter;

  T pawn_mobility_h_plus_parameter;
  T pawn_mobility_h_minus_parameter;
  T pawn_mobility_z_parameter;
  T pawn_mobility_k_parameter;
  T pawn_mobility_q_plus_parameter;
  T pawn_mobility_q_minus_parameter;
  T pawn_mobility_r_plus_parameter;
  T pawn_mobility_r_minus_parameter;
  T pawn_mobility_g_plus_parameter;
  T pawn_mobility_g_minus_parameter;

  T& operator[](std::size_t index);
  const T& operator[](std::size_t index) const;

  template <typename U>
  std::size_t get_index_of(const U& ref) const;

  std::size_t get_size() const;

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

// Example use case: weights.get_index_of(weights.diagonal_mobility);
template <typename T>
template <typename U>
std::size_t Evaluation_Weights<T>::get_index_of(const U& ref) const {
  return m_weight_ref_array.get_index_of(ref);
}

std::size_t get_size() const { return m_weight_ref_array.size; }

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

  template <PIECE_COLOR moving_side>
  inline Score mobility_score() const;

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

template <typename T>
template <PIECE_COLOR moving_side>
inline Score Evaluator<T>::mobility_score() const {
  constexpr PIECE_COLOR opposing_side = (PIECE_COLOR)((~moving_side) & 0x1);

  Attacks a;

  std::vector<Moves_Bitboard> moving_side_king_moves_bitboards;
  m_moving_side_matrix.get_piece_moves_bitboards(
      moving_side, PIECES::KING, moving_side_king_moves_bitboards);
  Score moving_side_king_mobility_score = Score::from_int(0);
  for (Moves_Bitboard& mb :
       moving_side_king_moves_bitboards) {  // There is only one king.
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

    const Score diagonal_mobility_score =
        diagonal_movements.high_bit_count() * m_weights.diagonal_mobility;
    const Score orthogonal_mobility_score =
        orthogonal_movements.high_bit_count() * m_weights.orthogonal_mobility;
    const Score multi_movement_mobility_score =
        ((diagonal_movements.high_bit_count() > 0) &&
         (orthogonal_movements.high_bit_count() > 0)) *
        m_weights.multi_movement_mobility;
    const Score backward_mobility_score = backward_movements.high_bit_count() *
                                          m_weights.backwards_movement_mobility;

    moving_side_king_mobility_score +=
        (diagonal_mobility_score + orthogonal_mobility_score +
         multi_movement_mobility_score + backward_mobility_score);
  }

  moving_side_king_mobility_score =
      Non_Linear_Response(m_weights.king_mobility_h_plus_parameter,
                          m_weights.king_mobility_h_minus_parameter,
                          m_weights.king_mobility_z_parameter,
                          m_weights.king_mobility_k_parameter,
                          m_weights.king_mobility_q_plus_parameter,
                          m_weights.king_mobility_q_minus_parameter,
                          m_weights.king_mobility_r_plus_parameter,
                          m_weights.king_mobility_r_minus_parameter,
                          m_weights.king_mobility_g_plus_parameter,
                          m_weights.king_mobility_g_minus_parameter)
          .value(moving_side_king_mobility_score.to_int());

  std::vector<Moves_Bitboard> moving_side_queens_moves_bitboards;
  m_moving_side_matrix.get_piece_moves_bitboards(
      moving_side, PIECES::QUEEN, moving_side_queens_moves_bitboards);
  Score moving_side_queens_mobility_score = Score::from_int(0);
  for (Moves_Bitboard& mb : moving_side_queens_moves_bitboards) {
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

    const Score diagonal_mobility_score =
        diagonal_movements.high_bit_count() * m_weights.diagonal_mobility;
    const Score orthogonal_mobility_score =
        orthogonal_movements.high_bit_count() * m_weights.orthogonal_mobility;
    const Score multi_movement_mobility_score =
        ((diagonal_movements.high_bit_count() > 0) &&
         (orthogonal_movements.high_bit_count() > 0)) *
        m_weights.multi_movement_mobility;
    const Score backward_mobility_score = backward_movements.high_bit_count() *
                                          m_weights.backwards_movement_mobility;

    moving_side_queens_mobility_score +=
        (diagonal_mobility_score + orthogonal_mobility_score +
         multi_movement_mobility_score + backward_mobility_score);
  }

  moving_side_queens_mobility_score =
      Non_Linear_Response(m_weights.queen_mobility_h_plus_parameter,
                          m_weights.queen_mobility_h_minus_parameter,
                          m_weights.queen_mobility_z_parameter,
                          m_weights.queen_mobility_k_parameter,
                          m_weights.queen_mobility_q_plus_parameter,
                          m_weights.queen_mobility_q_minus_parameter,
                          m_weights.queen_mobility_r_plus_parameter,
                          m_weights.queen_mobility_r_minus_parameter,
                          m_weights.queen_mobility_g_plus_parameter,
                          m_weights.queen_mobility_g_minus_parameter)
          .value(moving_side_queens_mobility_score.to_int());

  std::vector<Moves_Bitboard> moving_side_rooks_moves_bitboards;
  m_moving_side_matrix.get_piece_moves_bitboards(
      moving_side, PIECES::ROOK, moving_side_rooks_moves_bitboards);
  Score moving_side_rooks_mobility_score = Score::from_int(0);
  for (Moves_Bitboard& mb : moving_side_rooks_moves_bitboards) {
    const Bitboard orthogonal_movements = mb.bitboard;
    const Bitboard backward_movements =
        mb.bitboard.get_backward_squares_mask(mb.square, moving_side);

    const Score orthogonal_mobility_score =
        orthogonal_movements.high_bit_count() * m_weights.orthogonal_mobility;
    const Score backward_mobility_score = backward_movements.high_bit_count() *
                                          m_weights.backwards_movement_mobility;

    moving_side_rooks_mobility_score +=
        (orthogonal_mobility_score + backward_mobility_score);
  }

  moving_side_rooks_mobility_score =
      Non_Linear_Response(m_weights.rook_mobility_h_plus_parameter,
                          m_weights.rook_mobility_h_minus_parameter,
                          m_weights.rook_mobility_z_parameter,
                          m_weights.rook_mobility_k_parameter,
                          m_weights.rook_mobility_q_plus_parameter,
                          m_weights.rook_mobility_q_minus_parameter,
                          m_weights.rook_mobility_r_plus_parameter,
                          m_weights.rook_mobility_r_minus_parameter,
                          m_weights.rook_mobility_g_plus_parameter,
                          m_weights.rook_mobility_g_minus_parameter)
          .value(moving_side_rooks_mobility_score.to_int());

  std::vector<Moves_Bitboard> moving_side_bishops_moves_bitboards;
  m_moving_side_matrix.get_piece_moves_bitboards(
      moving_side, PIECES::BISHOP, moving_side_bishops_moves_bitboards);
  Score moving_side_bishops_mobility_score = Score::from_int(0);
  for (Moves_Bitboard& mb : moving_side_bishops_moves_bitboards) {
    const Bitboard diagonal_movements = mb.bitboard;
    const Bitboard backward_movements =
        mb.bitboard.get_backward_squares_mask(mb.square, moving_side);

    const Score diagonal_mobility_score =
        diagonal_movements.high_bit_count() * m_weights.diagonal_mobility;
    const Score backward_mobility_score = backward_movements.high_bit_count() *
                                          m_weights.backwards_movement_mobility;

    moving_side_bishops_mobility_score +=
        (diagonal_mobility_score + backward_mobility_score);
  }

  moving_side_bishops_mobility_score =
      Non_Linear_Response(m_weights.bishop_mobility_h_plus_parameter,
                          m_weights.bishop_mobility_h_minus_parameter,
                          m_weights.bishop_mobility_z_parameter,
                          m_weights.bishop_mobility_k_parameter,
                          m_weights.bishop_mobility_q_plus_parameter,
                          m_weights.bishop_mobility_q_minus_parameter,
                          m_weights.bishop_mobility_r_plus_parameter,
                          m_weights.bishop_mobility_r_minus_parameter,
                          m_weights.bishop_mobility_g_plus_parameter,
                          m_weights.bishop_mobility_g_minus_parameter)
          .value(moving_side_bishops_mobility_score.to_int());

  std::vector<Moves_Bitboard> moving_side_knights_moves_bitboards;
  m_moving_side_matrix.get_piece_moves_bitboards(
      moving_side, PIECES::KNIGHT, moving_side_knights_moves_bitboards);
  Score moving_side_knights_mobility_score = Score::from_int(0);
  for (Moves_Bitboard& mb : moving_side_knights_moves_bitboards) {
    const Bitboard knight_movements = mb.bitboard;
    const Bitboard backward_movements =
        mb.bitboard.get_backward_squares_mask(mb.square, moving_side);

    const Score knight_mobility_score =
        knight_movements.high_bit_count() * m_weights.knight_movement_mobility;
    const Score backward_mobility_score = backward_movements.high_bit_count() *
                                          m_weights.backwards_movement_mobility;

    moving_side_knights_mobility_score +=
        (knight_mobility_score + backward_mobility_score);
  }

  moving_side_knights_mobility_score =
      Non_Linear_Response(m_weights.knight_mobility_h_plus_parameter,
                          m_weights.knight_mobility_h_minus_parameter,
                          m_weights.knight_mobility_z_parameter,
                          m_weights.knight_mobility_k_parameter,
                          m_weights.knight_mobility_q_plus_parameter,
                          m_weights.knight_mobility_q_minus_parameter,
                          m_weights.knight_mobility_r_plus_parameter,
                          m_weights.knight_mobility_r_minus_parameter,
                          m_weights.knight_mobility_g_plus_parameter,
                          m_weights.knight_mobility_g_minus_parameter)
          .value(moving_side_knights_mobility_score.to_int());

  std::vector<Moves_Bitboard> moving_side_pawns_moves_bitboards;
  m_moving_side_matrix.get_piece_moves_bitboards(
      moving_side, PIECES::PAWN, moving_side_pawns_moves_bitboards);
  Score moving_side_pawns_mobility_score = Score::from_int(0);
  for (Moves_Bitboard& mb : moving_side_pawns_moves_bitboards) {
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

    const Score diagonal_mobility_score =
        diagonal_movements.high_bit_count() * m_weights.diagonal_mobility;
    const Score orthogonal_mobility_score =
        orthogonal_movements.high_bit_count() * m_weights.orthogonal_mobility;
    const Score multi_movement_mobility_score =
        ((diagonal_movements.high_bit_count() > 0) &&
         (orthogonal_movements.high_bit_count() > 0)) *
        m_weights.multi_movement_mobility;

    moving_side_pawns_mobility_score +=
        (diagonal_mobility_score + orthogonal_mobility_score +
         multi_movement_mobility_score);
  }

  moving_side_pawns_mobility_score =
      Non_Linear_Response(m_weights.pawn_mobility_h_plus_parameter,
                          m_weights.pawn_mobility_h_minus_parameter,
                          m_weights.pawn_mobility_z_parameter,
                          m_weights.pawn_mobility_k_parameter,
                          m_weights.pawn_mobility_q_plus_parameter,
                          m_weights.pawn_mobility_q_minus_parameter,
                          m_weights.pawn_mobility_r_plus_parameter,
                          m_weights.pawn_mobility_r_minus_parameter,
                          m_weights.pawn_mobility_g_plus_parameter,
                          m_weights.pawn_mobility_g_minus_parameter)
          .value(moving_side_pawns_mobility_score.to_int());

  return Score(moving_side_king_mobility_score.to_int() +
               moving_side_queens_mobility_score.to_int() +
               moving_side_rooks_mobility_score.to_int() +
               moving_side_bishops_mobility_score.to_int() +
               moving_side_knights_mobility_score.to_int() +
               moving_side_pawns_mobility_score.to_int());
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
              m_weights.knight_mobility +
          piece_movement_counts[PIECES::KNIGHT].backwards_movement *
              m_weights.backwards_movement_mobility;
  T f_p = piece_movement_counts[PIECES::PAWN].diagonal_movement *
              m_weights.diagonal_mobility +
          piece_movement_counts[PIECES::PAWN].orthogonal_movement *
              m_weights.orthogonal_mobility +
          piece_movement_counts[PIECES::PAWN].multi_movement *
              m_weights.multi_movement_mobility;

  Non_Linear_Response nlr_king(m_weights.king_mobility_h_plus_parameter,
                               m_weights.king_mobility_h_minus_parameter,
                               m_weights.king_mobility_z_parameter,
                               m_weights.king_mobility_k_parameter,
                               m_weights.king_mobility_q_plus_parameter,
                               m_weights.king_mobility_q_minus_parameter,
                               m_weights.king_mobility_r_plus_parameter,
                               m_weights.king_mobility_r_minus_parameter,
                               m_weights.king_mobility_g_plus_parameter,
                               m_weights.king_mobility_g_minus_parameter);

  Non_Linear_Response nlr_queen(m_weights.queen_mobility_h_plus_parameter,
                                m_weights.queen_mobility_h_minus_parameter,
                                m_weights.queen_mobility_z_parameter,
                                m_weights.queen_mobility_k_parameter,
                                m_weights.queen_mobility_q_plus_parameter,
                                m_weights.queen_mobility_q_minus_parameter,
                                m_weights.queen_mobility_r_plus_parameter,
                                m_weights.queen_mobility_r_minus_parameter,
                                m_weights.queen_mobility_g_plus_parameter,
                                m_weights.queen_mobility_g_minus_parameter);

  Non_Linear_Response nlr_rook(m_weights.rook_mobility_h_plus_parameter,
                               m_weights.rook_mobility_h_minus_parameter,
                               m_weights.rook_mobility_z_parameter,
                               m_weights.rook_mobility_k_parameter,
                               m_weights.rook_mobility_q_plus_parameter,
                               m_weights.rook_mobility_q_minus_parameter,
                               m_weights.rook_mobility_r_plus_parameter,
                               m_weights.rook_mobility_r_minus_parameter,
                               m_weights.rook_mobility_g_plus_parameter,
                               m_weights.rook_mobility_g_minus_parameter);

  Non_Linear_Response nlr_bishop(m_weights.bishop_mobility_h_plus_parameter,
                                 m_weights.bishop_mobility_h_minus_parameter,
                                 m_weights.bishop_mobility_z_parameter,
                                 m_weights.bishop_mobility_k_parameter,
                                 m_weights.bishop_mobility_q_plus_parameter,
                                 m_weights.bishop_mobility_q_minus_parameter,
                                 m_weights.bishop_mobility_r_plus_parameter,
                                 m_weights.bishop_mobility_r_minus_parameter,
                                 m_weights.bishop_mobility_g_plus_parameter,
                                 m_weights.bishop_mobility_g_minus_parameter);

  Non_Linear_Response nlr_knight(m_weights.knight_mobility_h_plus_parameter,
                                 m_weights.knight_mobility_h_minus_parameter,
                                 m_weights.knight_mobility_z_parameter,
                                 m_weights.knight_mobility_k_parameter,
                                 m_weights.knight_mobility_q_plus_parameter,
                                 m_weights.knight_mobility_q_minus_parameter,
                                 m_weights.knight_mobility_r_plus_parameter,
                                 m_weights.knight_mobility_r_minus_parameter,
                                 m_weights.knight_mobility_g_plus_parameter,
                                 m_weights.knight_mobility_g_minus_parameter);

  Non_Linear_Response nlr_pawn(m_weights.pawn_mobility_h_plus_parameter,
                               m_weights.pawn_mobility_h_minus_parameter,
                               m_weights.pawn_mobility_z_parameter,
                               m_weights.pawn_mobility_k_parameter,
                               m_weights.pawn_mobility_q_plus_parameter,
                               m_weights.pawn_mobility_q_minus_parameter,
                               m_weights.pawn_mobility_r_plus_parameter,
                               m_weights.pawn_mobility_r_minus_parameter,
                               m_weights.pawn_mobility_g_plus_parameter,
                               m_weights.pawn_mobility_g_minus_parameter);

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
                        m_weights.king_mobility_h_plus_parameter)) {
      derivative_weights[i] = nlr_king.partial_derivative_h_plus(f_k);
    } else if (i == m_weights.get_index_of(
                        m_weights.king_mobility_h_minus_parameter)) {
      derivative_weights[i] = nlr_king.partial_derivative_h_minus(f_k);
    } else if (i ==
               m_weights.get_index_of(m_weights.king_mobility_z_parameter)) {
      derivative_weights[i] = nlr_king.partial_derivative_z(f_k);
    } else if (i ==
               m_weights.get_index_of(m_weights.king_mobility_k_parameter)) {
      derivative_weights[i] = nlr_king.partial_derivative_k(f_k);
    } else if (i == m_weights.get_index_of(
                        m_weights.king_mobility_q_plus_parameter)) {
      derivative_weights[i] = nlr_king.partial_derivative_q_plus(f_k);
    } else if (i == m_weights.get_index_of(
                        m_weights.king_mobility_q_minus_parameter)) {
      derivative_weights[i] = nlr_king.partial_derivative_q_minus(f_k);
    } else if (i == m_weights.get_index_of(
                        m_weights.king_mobility_r_plus_parameter)) {
      derivative_weights[i] = nlr_king.partial_derivative_r_plus(f_k);
    } else if (i == m_weights.get_index_of(
                        m_weights.king_mobility_r_minus_parameter)) {
      derivative_weights[i] = nlr_king.partial_derivative_r_minus(f_k);
    } else if (i == m_weights.get_index_of(
                        m_weights.king_mobility_g_plus_parameter)) {
      derivative_weights[i] = nlr_king.partial_derivative_g_plus(f_k);
    } else if (i == m_weights.get_index_of(
                        m_weights.king_mobility_g_minus_parameter)) {
      derivative_weights[i] = nlr_king.partial_derivative_g_minus(f_k);
    } else if (i == m_weights.get_index_of(
                        m_weights.queen_mobility_h_plus_parameter)) {
      derivative_weights[i] = nlr_queen.partial_derivative_h_plus(f_q);
    } else if (i == m_weights.get_index_of(
                        m_weights.queen_mobility_h_minus_parameter)) {
      derivative_weights[i] = nlr_queen.partial_derivative_h_minus(f_q);
    } else if (i ==
               m_weights.get_index_of(m_weights.queen_mobility_z_parameter)) {
      derivative_weights[i] = nlr_queen.partial_derivative_z(f_q);
    } else if (i ==
               m_weights.get_index_of(m_weights.queen_mobility_k_parameter)) {
      derivative_weights[i] = nlr_queen.partial_derivative_k(f_q);
    } else if (i == m_weights.get_index_of(
                        m_weights.queen_mobility_q_plus_parameter)) {
      derivative_weights[i] = nlr_queen.partial_derivative_q_plus(f_q);
    } else if (i == m_weights.get_index_of(
                        m_weights.queen_mobility_q_minus_parameter)) {
      derivative_weights[i] = nlr_queen.partial_derivative_q_minus(f_q);
    } else if (i == m_weights.get_index_of(
                        m_weights.queen_mobility_r_plus_parameter)) {
      derivative_weights[i] = nlr_queen.partial_derivative_r_plus(f_q);
    } else if (i == m_weights.get_index_of(
                        m_weights.queen_mobility_r_minus_parameter)) {
      derivative_weights[i] = nlr_queen.partial_derivative_r_minus(f_q);
    } else if (i == m_weights.get_index_of(
                        m_weights.queen_mobility_g_plus_parameter)) {
      derivative_weights[i] = nlr_queen.partial_derivative_g_plus(f_q);
    } else if (i == m_weights.get_index_of(
                        m_weights.queen_mobility_g_minus_parameter)) {
      derivative_weights[i] = nlr_queen.partial_derivative_g_minus(f_q);
    } else if (i == m_weights.get_index_of(
                        m_weights.rook_mobility_h_plus_parameter)) {
      derivative_weights[i] = nlr_rook.partial_derivative_h_plus(f_r);
    } else if (i == m_weights.get_index_of(
                        m_weights.rook_mobility_h_minus_parameter)) {
      derivative_weights[i] = nlr_rook.partial_derivative_h_minus(f_r);
    } else if (i ==
               m_weights.get_index_of(m_weights.rook_mobility_z_parameter)) {
      derivative_weights[i] = nlr_rook.partial_derivative_z(f_r);
    } else if (i ==
               m_weights.get_index_of(m_weights.rook_mobility_k_parameter)) {
      derivative_weights[i] = nlr_rook.partial_derivative_k(f_r);
    } else if (i == m_weights.get_index_of(
                        m_weights.rook_mobility_q_plus_parameter)) {
      derivative_weights[i] = nlr_rook.partial_derivative_q_plus(f_r);
    } else if (i == m_weights.get_index_of(
                        m_weights.rook_mobility_q_minus_parameter)) {
      derivative_weights[i] = nlr_rook.partial_derivative_q_minus(f_r);
    } else if (i == m_weights.get_index_of(
                        m_weights.rook_mobility_r_plus_parameter)) {
      derivative_weights[i] = nlr_rook.partial_derivative_r_plus(f_r);
    } else if (i == m_weights.get_index_of(
                        m_weights.rook_mobility_r_minus_parameter)) {
      derivative_weights[i] = nlr_rook.partial_derivative_r_minus(f_r);
    } else if (i == m_weights.get_index_of(
                        m_weights.rook_mobility_g_plus_parameter)) {
      derivative_weights[i] = nlr_rook.partial_derivative_g_plus(f_r);
    } else if (i == m_weights.get_index_of(
                        m_weights.rook_mobility_g_minus_parameter)) {
      derivative_weights[i] = nlr_rook.partial_derivative_g_minus(f_r);
    } else if (i == m_weights.get_index_of(
                        m_weights.bishop_mobility_h_plus_parameter)) {
      derivative_weights[i] = nlr_bishop.partial_derivative_h_plus(f_b);
    } else if (i == m_weights.get_index_of(
                        m_weights.bishop_mobility_h_minus_parameter)) {
      derivative_weights[i] = nlr_bishop.partial_derivative_h_minus(f_b);
    } else if (i ==
               m_weights.get_index_of(m_weights.bishop_mobility_z_parameter)) {
      derivative_weights[i] = nlr_bishop.partial_derivative_z(f_b);
    } else if (i ==
               m_weights.get_index_of(m_weights.bishop_mobility_k_parameter)) {
      derivative_weights[i] = nlr_bishop.partial_derivative_k(f_b);
    } else if (i == m_weights.get_index_of(
                        m_weights.bishop_mobility_q_plus_parameter)) {
      derivative_weights[i] = nlr_bishop.partial_derivative_q_plus(f_b);
    } else if (i == m_weights.get_index_of(
                        m_weights.bishop_mobility_q_minus_parameter)) {
      derivative_weights[i] = nlr_bishop.partial_derivative_q_minus(f_b);
    } else if (i == m_weights.get_index_of(
                        m_weights.bishop_mobility_r_plus_parameter)) {
      derivative_weights[i] = nlr_bishop.partial_derivative_r_plus(f_b);
    } else if (i == m_weights.get_index_of(
                        m_weights.bishop_mobility_r_minus_parameter)) {
      derivative_weights[i] = nlr_bishop.partial_derivative_r_minus(f_b);
    } else if (i == m_weights.get_index_of(
                        m_weights.bishop_mobility_g_plus_parameter)) {
      derivative_weights[i] = nlr_bishop.partial_derivative_g_plus(f_b);
    } else if (i == m_weights.get_index_of(
                        m_weights.bishop_mobility_g_minus_parameter)) {
      derivative_weights[i] = nlr_bishop.partial_derivative_g_minus(f_b);
    } else if (i == m_weights.get_index_of(
                        m_weights.knight_mobility_h_plus_parameter)) {
      derivative_weights[i] = nlr_knight.partial_derivative_h_plus(f_n);
    } else if (i == m_weights.get_index_of(
                        m_weights.knight_mobility_h_minus_parameter)) {
      derivative_weights[i] = nlr_knight.partial_derivative_h_minus(f_n);
    } else if (i ==
               m_weights.get_index_of(m_weights.knight_mobility_z_parameter)) {
      derivative_weights[i] = nlr_knight.partial_derivative_z(f_n);
    } else if (i ==
               m_weights.get_index_of(m_weights.knight_mobility_k_parameter)) {
      derivative_weights[i] = nlr_knight.partial_derivative_k(f_n);
    } else if (i == m_weights.get_index_of(
                        m_weights.knight_mobility_q_plus_parameter)) {
      derivative_weights[i] = nlr_knight.partial_derivative_q_plus(f_n);
    } else if (i == m_weights.get_index_of(
                        m_weights.knight_mobility_q_minus_parameter)) {
      derivative_weights[i] = nlr_knight.partial_derivative_q_minus(f_n);
    } else if (i == m_weights.get_index_of(
                        m_weights.knight_mobility_r_plus_parameter)) {
      derivative_weights[i] = nlr_knight.partial_derivative_r_plus(f_n);
    } else if (i == m_weights.get_index_of(
                        m_weights.knight_mobility_r_minus_parameter)) {
      derivative_weights[i] = nlr_knight.partial_derivative_r_minus(f_n);
    } else if (i == m_weights.get_index_of(
                        m_weights.knight_mobility_g_plus_parameter)) {
      derivative_weights[i] = nlr_knight.partial_derivative_g_plus(f_n);
    } else if (i == m_weights.get_index_of(
                        m_weights.knight_mobility_g_minus_parameter)) {
      derivative_weights[i] = nlr_knight.partial_derivative_g_minus(f_n);
    } else if (i == m_weights.get_index_of(
                        m_weights.pawn_mobility_h_plus_parameter)) {
      derivative_weights[i] = nlr_pawn.partial_derivative_h_plus(f_p);
    } else if (i == m_weights.get_index_of(
                        m_weights.pawn_mobility_h_minus_parameter)) {
      derivative_weights[i] = nlr_pawn.partial_derivative_h_minus(f_p);
    } else if (i ==
               m_weights.get_index_of(m_weights.pawn_mobility_z_parameter)) {
      derivative_weights[i] = nlr_pawn.partial_derivative_z(f_p);
    } else if (i ==
               m_weights.get_index_of(m_weights.pawn_mobility_k_parameter)) {
      derivative_weights[i] = nlr_pawn.partial_derivative_k(f_p);
    } else if (i == m_weights.get_index_of(
                        m_weights.pawn_mobility_q_plus_parameter)) {
      derivative_weights[i] = nlr_pawn.partial_derivative_q_plus(f_p);
    } else if (i == m_weights.get_index_of(
                        m_weights.pawn_mobility_q_minus_parameter)) {
      derivative_weights[i] = nlr_pawn.partial_derivative_q_minus(f_p);
    } else if (i == m_weights.get_index_of(
                        m_weights.pawn_mobility_r_plus_parameter)) {
      derivative_weights[i] = nlr_pawn.partial_derivative_r_plus(f_p);
    } else if (i == m_weights.get_index_of(
                        m_weights.pawn_mobility_r_minus_parameter)) {
      derivative_weights[i] = nlr_pawn.partial_derivative_r_minus(f_p);
    } else if (i == m_weights.get_index_of(
                        m_weights.pawn_mobility_g_plus_parameter)) {
      derivative_weights[i] = nlr_pawn.partial_derivative_g_plus(f_p);
    } else if (i == m_weights.get_index_of(
                        m_weights.pawn_mobility_g_minus_parameter)) {
      derivative_weights[i] = nlr_pawn.partial_derivative_g_minus(f_p);
    } else {
      derivative_weights[i] = 0;
    }
  }

  return derivative_weights;
}
