#pragma once

#include <cmath>

#include "chess_board.hpp"
#include "globals.hpp"
#include "move_generator.hpp"
#include "non_linear_response.hpp"
#include "score.hpp"

template <typename T>
class Evaluation_Weights
{

    using Piece_Square_Table_Type = multi_array<T, NUM_OF_PLAYERS, NUM_OF_UNIQUE_PIECES_PER_PLAYER, NUM_OF_SQUARES_ON_CHESS_BOARD>;

    // clang-format off
  using Evaluation_Weights_Reference_Array = Reference_Array<
      T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,
      T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,
      T, multi_array<T, (NUM_OF_UNIQUE_PIECES_PER_PLAYER - 1)>, T, T, T, T, T,
      T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,
      T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,
      T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, 
      T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, 
      T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, 
      T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, 
      T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, 
      T, T, T, T, T, Piece_Square_Table_Type, T, T, T, T, T, T, T, T, T, T, T,
      T, T, T, T, T, T, T, T, T>;
    // clang-format on

  public:

    Evaluation_Weights() :
        material_NLR_parameters {},
        material {},
        piece_mobility_NLR_parameters {},
        diagonal_mobility(T {}),
        orthogonal_mobility(T {}),
        knight_movement_mobility(T {}),
        multi_movement_mobility(T {}),
        backwards_movement_mobility(T {}),
        piece_square_NLR_parameters {},
        piece_square_tables {},
        interactive_piece_square_NLR_parameters {},
        m_weight_ref_array(
            NLR_ARRAY_FIELDS(material_NLR_parameters, PIECES::PAWN),
            NLR_ARRAY_FIELDS(material_NLR_parameters, PIECES::KNIGHT),
            NLR_ARRAY_FIELDS(material_NLR_parameters, PIECES::BISHOP),
            NLR_ARRAY_FIELDS(material_NLR_parameters, PIECES::ROOK),
            NLR_ARRAY_FIELDS(material_NLR_parameters, PIECES::QUEEN),
            material,
            NLR_ARRAY_FIELDS(piece_mobility_NLR_parameters, PIECES::PAWN),
            NLR_ARRAY_FIELDS(piece_mobility_NLR_parameters, PIECES::KNIGHT),
            NLR_ARRAY_FIELDS(piece_mobility_NLR_parameters, PIECES::BISHOP),
            NLR_ARRAY_FIELDS(piece_mobility_NLR_parameters, PIECES::ROOK),
            NLR_ARRAY_FIELDS(piece_mobility_NLR_parameters, PIECES::QUEEN),
            NLR_ARRAY_FIELDS(piece_mobility_NLR_parameters, PIECES::KING),
            diagonal_mobility,
            orthogonal_mobility,
            knight_movement_mobility,
            multi_movement_mobility,
            backwards_movement_mobility,
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters, PIECE_COLOR::WHITE, PIECES::PAWN),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters, PIECE_COLOR::WHITE, PIECES::KNIGHT),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters, PIECE_COLOR::WHITE, PIECES::BISHOP),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters, PIECE_COLOR::WHITE, PIECES::ROOK),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters, PIECE_COLOR::WHITE, PIECES::QUEEN),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters, PIECE_COLOR::WHITE, PIECES::KING),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters, PIECE_COLOR::BLACK, PIECES::PAWN),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters, PIECE_COLOR::BLACK, PIECES::KNIGHT),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters, PIECE_COLOR::BLACK, PIECES::BISHOP),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters, PIECE_COLOR::BLACK, PIECES::ROOK),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters, PIECE_COLOR::BLACK, PIECES::QUEEN),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters, PIECE_COLOR::BLACK, PIECES::KING),
            piece_square_tables,
            NLR_ARRAY_FIELDS(interactive_piece_square_NLR_parameters, PIECE_COLOR::WHITE),
            NLR_ARRAY_FIELDS(interactive_piece_square_NLR_parameters, PIECE_COLOR::BLACK)
        )
    {
    }

    Evaluation_Weights(
        multi_array<NLR_Parameters<T>, (NUM_OF_UNIQUE_PIECES_PER_PLAYER - 1)>
            material_NLR_weights,
        multi_array<T, (NUM_OF_UNIQUE_PIECES_PER_PLAYER - 1)> material_weights,
        multi_array<NLR_Parameters<T>, NUM_OF_UNIQUE_PIECES_PER_PLAYER>
            piece_mobility_NLR_weights,
        T   diagonal_mobility_weight,
        T   orthogonal_mobility_weight,
        T   knight_movement_mobility_weight,
        T   multi_movement_mobility_weight,
        T   backwards_movement_mobility_weight,
        multi_array<NLR_Parameters<T>, NUM_OF_PLAYERS, NUM_OF_UNIQUE_PIECES_PER_PLAYER> piece_square_NLR_weights,
        Piece_Square_Table_Type piece_square_weights,
        multi_array<NLR_Parameters<T>, NUM_OF_PLAYERS> interactive_piece_square_NLR_weights):
        material_NLR_parameters(material_NLR_weights),
        material(material_weights),
        piece_mobility_NLR_parameters(piece_mobility_NLR_weights),
        diagonal_mobility(diagonal_mobility_weight),
        orthogonal_mobility(orthogonal_mobility_weight),
        knight_movement_mobility(knight_movement_mobility_weight),
        multi_movement_mobility(multi_movement_mobility_weight),
        backwards_movement_mobility(backwards_movement_mobility_weight),
        piece_square_NLR_parameters(piece_square_NLR_weights),
        piece_square_tables(piece_square_weights),
        interactive_piece_square_NLR_parameters(interactive_piece_square_NLR_weights),
        m_weight_ref_array(
            NLR_ARRAY_FIELDS(material_NLR_parameters, PIECES::PAWN),
            NLR_ARRAY_FIELDS(material_NLR_parameters, PIECES::KNIGHT),
            NLR_ARRAY_FIELDS(material_NLR_parameters, PIECES::BISHOP),
            NLR_ARRAY_FIELDS(material_NLR_parameters, PIECES::ROOK),
            NLR_ARRAY_FIELDS(material_NLR_parameters, PIECES::QUEEN),
            material,
            NLR_ARRAY_FIELDS(piece_mobility_NLR_parameters, PIECES::PAWN),
            NLR_ARRAY_FIELDS(piece_mobility_NLR_parameters, PIECES::KNIGHT),
            NLR_ARRAY_FIELDS(piece_mobility_NLR_parameters, PIECES::BISHOP),
            NLR_ARRAY_FIELDS(piece_mobility_NLR_parameters, PIECES::ROOK),
            NLR_ARRAY_FIELDS(piece_mobility_NLR_parameters, PIECES::QUEEN),
            NLR_ARRAY_FIELDS(piece_mobility_NLR_parameters, PIECES::KING),
            diagonal_mobility,
            orthogonal_mobility,
            knight_movement_mobility,
            multi_movement_mobility,
            backwards_movement_mobility,
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters, PIECE_COLOR::WHITE, PIECES::PAWN),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters, PIECE_COLOR::WHITE, PIECES::KNIGHT),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters, PIECE_COLOR::WHITE, PIECES::BISHOP),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters, PIECE_COLOR::WHITE, PIECES::ROOK),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters, PIECE_COLOR::WHITE, PIECES::QUEEN),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters, PIECE_COLOR::WHITE, PIECES::KING),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters, PIECE_COLOR::BLACK, PIECES::PAWN),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters, PIECE_COLOR::BLACK, PIECES::KNIGHT),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters, PIECE_COLOR::BLACK, PIECES::BISHOP),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters, PIECE_COLOR::BLACK, PIECES::ROOK),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters, PIECE_COLOR::BLACK, PIECES::QUEEN),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters, PIECE_COLOR::BLACK, PIECES::KING),
            piece_square_tables,
            NLR_ARRAY_FIELDS(interactive_piece_square_NLR_parameters, PIECE_COLOR::WHITE),
            NLR_ARRAY_FIELDS(interactive_piece_square_NLR_parameters, PIECE_COLOR::BLACK)
        )
    {
    }

    Evaluation_Weights(const Evaluation_Weights& other);
    Evaluation_Weights& operator=(const Evaluation_Weights& other);
    Evaluation_Weights(Evaluation_Weights&& other) noexcept;
    Evaluation_Weights& operator=(Evaluation_Weights&& other) noexcept;

    // Material weights.
    multi_array<NLR_Parameters<T>, (NUM_OF_UNIQUE_PIECES_PER_PLAYER - 1)>
        material_NLR_parameters;
    multi_array<T, (NUM_OF_UNIQUE_PIECES_PER_PLAYER - 1)> material;

    // Mobility weights.
    multi_array<NLR_Parameters<T>, NUM_OF_UNIQUE_PIECES_PER_PLAYER>
        piece_mobility_NLR_parameters;
    T   diagonal_mobility;
    T   orthogonal_mobility;
    T   knight_movement_mobility;
    T   multi_movement_mobility;
    T   backwards_movement_mobility;

    // Piece Square Tables
    multi_array<NLR_Parameters<T>, NUM_OF_PLAYERS, NUM_OF_UNIQUE_PIECES_PER_PLAYER> piece_square_NLR_parameters;
    Piece_Square_Table_Type piece_square_tables;
    multi_array<NLR_Parameters<T>, NUM_OF_PLAYERS> interactive_piece_square_NLR_parameters;

    T&       operator[](std::size_t index);
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

    Evaluation_Weights sqrt() const;
    T                  magnitude() const;

    Evaluation_Weights<double>        to_double() const;
    Evaluation_Weights<Matrex_FP_Int> to_matrex_fp_int() const;

    template <typename W>
    friend std::ostream& operator<<(std::ostream&                os,
                                    const Evaluation_Weights<W>& weights);

    template <typename U>
    std::size_t get_index_of(const U& ref) const;

    std::size_t get_size() const;

  private:

    Evaluation_Weights_Reference_Array m_weight_ref_array;
};

// Function prototype for non-member function.
template <typename T>
Evaluation_Weights<T> operator/(T scalar, const Evaluation_Weights<T>& weights);

// Copy constructor: copy values not references - avoids dangling references
// when other is a temporary object.
template <typename T>
Evaluation_Weights<T>::Evaluation_Weights(const Evaluation_Weights& other) :
    Evaluation_Weights()
{ // default-construct: zero + build
  // m_weight_ref_array of references of this
  // object's weights
    for (std::size_t i = 0; i < get_size(); ++i) { (*this)[i] = other[i]; }
}

template <typename T>
Evaluation_Weights<T>&
Evaluation_Weights<T>::operator=(const Evaluation_Weights& other)
{
    if (this == &other) { return *this; }

    for (std::size_t i = 0; i < get_size(); ++i) { (*this)[i] = other[i]; }
    return *this;
}

// Move constructor - same as copy constructor
template <typename T>
Evaluation_Weights<T>::Evaluation_Weights(Evaluation_Weights&& other) noexcept :
    Evaluation_Weights()
{
    for (std::size_t i = 0; i < get_size(); ++i) { (*this)[i] = other[i]; }
}

template <typename T>
Evaluation_Weights<T>&
Evaluation_Weights<T>::operator=(Evaluation_Weights&& other) noexcept
{
    if (this == &other) { return *this; }

    for (std::size_t i = 0; i < get_size(); ++i) { (*this)[i] = other[i]; }
    return *this;
}

template <typename T>
T& Evaluation_Weights<T>::operator[](std::size_t index)
{
    if (index >= m_weight_ref_array.size)
    {
        throw std::out_of_range("Index out of range in Evaluation_Weights");
    }
    return m_weight_ref_array.get_array()[index].value().get();
}

template <typename T>
const T& Evaluation_Weights<T>::operator[](std::size_t index) const
{
    if (index >= m_weight_ref_array.size)
    {
        throw std::out_of_range("Index out of range in Evaluation_Weights");
    }
    return m_weight_ref_array.get_array()[index].value().get();
}

template <typename T>
Evaluation_Weights<T>
Evaluation_Weights<T>::operator+(const Evaluation_Weights& other) const
{
    Evaluation_Weights result;

    for (std::size_t i = 0; i < m_weight_ref_array.size; ++i)
    {
        result[i] = m_weight_ref_array.get_array()[i].value().get()
                  + other.m_weight_ref_array.get_array()[i].value().get();
    }

    return result;
}

template <typename T>
Evaluation_Weights<T>
Evaluation_Weights<T>::operator-(const Evaluation_Weights& other) const
{
    Evaluation_Weights result;

    for (std::size_t i = 0; i < m_weight_ref_array.size; ++i)
    {
        result[i] = m_weight_ref_array.get_array()[i].value().get()
                  - other.m_weight_ref_array.get_array()[i].value().get();
    }

    return result;
}

template <typename T>
Evaluation_Weights<T>
Evaluation_Weights<T>::operator/(const Evaluation_Weights& other) const
{
    Evaluation_Weights result;

    for (std::size_t i = 0; i < m_weight_ref_array.size; ++i)
    {
        result[i] = m_weight_ref_array.get_array()[i].value().get()
                  / other.m_weight_ref_array.get_array()[i].value().get();
    }

    return result;
}

template <typename T>
Evaluation_Weights<T>
Evaluation_Weights<T>::operator*(const Evaluation_Weights& other) const
{
    Evaluation_Weights result;

    for (std::size_t i = 0; i < m_weight_ref_array.size; ++i)
    {
        result[i] = m_weight_ref_array.get_array()[i].value().get()
                  * other.m_weight_ref_array.get_array()[i].value().get();
    }

    return result;
}

template <typename T>
Evaluation_Weights<T> Evaluation_Weights<T>::operator+(T value) const
{
    Evaluation_Weights result;

    for (std::size_t i = 0; i < m_weight_ref_array.size; ++i)
    {
        result[i] = m_weight_ref_array.get_array()[i].value().get() + value;
    }

    return result;
}

template <typename T>
Evaluation_Weights<T> Evaluation_Weights<T>::operator-(T value) const
{
    Evaluation_Weights result;

    for (std::size_t i = 0; i < m_weight_ref_array.size; ++i)
    {
        result[i] = m_weight_ref_array.get_array()[i].value().get() - value;
    }

    return result;
}

template <typename T>
Evaluation_Weights<T> Evaluation_Weights<T>::operator*(T value) const
{
    Evaluation_Weights result;

    for (std::size_t i = 0; i < m_weight_ref_array.size; ++i)
    {
        result[i] = m_weight_ref_array.get_array()[i].value().get() * value;
    }

    return result;
}

template <typename T>
Evaluation_Weights<T> Evaluation_Weights<T>::operator/(T value) const
{
    Evaluation_Weights result;

    for (std::size_t i = 0; i < m_weight_ref_array.size; ++i)
    {
        result[i] = m_weight_ref_array.get_array()[i].value().get() / value;
    }

    return result;
}

template <typename T>
Evaluation_Weights<T> operator/(T scalar, const Evaluation_Weights<T>& weights)
{
    Evaluation_Weights<T> result;

    for (std::size_t i = 0; i < result.get_size(); ++i)
    {
        result[i] = scalar / weights[i];
    }

    return result;
}

template <typename T>
Evaluation_Weights<T> Evaluation_Weights<T>::sqrt() const
{
    Evaluation_Weights result;

    for (std::size_t i = 0; i < m_weight_ref_array.size; ++i)
    {
        result[i] = static_cast<T>(
            std::sqrt(m_weight_ref_array.get_array()[i].value().get()));
    }

    return result;
}

template <typename T>
T Evaluation_Weights<T>::magnitude() const
{
    T result = 0;

    for (std::size_t i = 0; i < (*this).get_size(); ++i)
    {
        result += ((*this)[i] * (*this)[i]);
    }

    result = static_cast<T>(std::sqrt(result));

    return result;
}

template <typename T>
Evaluation_Weights<double> Evaluation_Weights<T>::to_double() const
{
    Evaluation_Weights<double> result;

    for (std::size_t i = 0; i < m_weight_ref_array.size; ++i)
    {
        result[i] = static_cast<double>((*this)[i].to_double());
    }

    return result;
}

template <typename T>
Evaluation_Weights<Matrex_FP_Int>
Evaluation_Weights<T>::to_matrex_fp_int() const
{
    Evaluation_Weights<Matrex_FP_Int> result;

    for (std::size_t i = 0; i < m_weight_ref_array.size; ++i)
    {
        result[i] = Matrex_FP_Int::from_double((*this)[i]);
    }

    return result;
}

template <typename W>
std::ostream& operator<<(std::ostream& os, const Evaluation_Weights<W>& weights)
{
    os << "{";
    for (std::size_t i = 0; i < weights.get_size(); ++i)
    {
        if (i != (weights.get_size() - 1)) { os << weights[i] << ", "; }
        else { os << weights[i]; }
    }
    os << "}";
    return os;
}

// Example use case: weights.get_index_of(weights.diagonal_mobility);
template <typename T>
template <typename U>
std::size_t Evaluation_Weights<T>::get_index_of(const U& ref) const
{
    return m_weight_ref_array.get_index_of(ref);
}

template <typename T>
std::size_t Evaluation_Weights<T>::get_size() const
{
    return m_weight_ref_array.size;
}

template <typename T>
class Evaluator
{
  public:

    Evaluator(const Evaluation_Weights<T>& weights,
              const Chess_Board&           cb,
              const Moves_Bitboard_Matrix& moving_side_matrix,
              const Moves_Bitboard_Matrix& opposing_side_matrix);

    T                     evaluate_template_typed() const;
    Score                 evaluate() const;
    Evaluation_Weights<T> derivative_evaluate() const;

    template <PIECE_COLOR moving_side>
    inline T material_score() const;

    template <PIECE_COLOR moving_side>
    inline Evaluation_Weights<T> derivative_material_score() const;

    template <PIECE_COLOR moving_side>
    inline T mobility_score() const;

    template <PIECE_COLOR moving_side>
    inline Evaluation_Weights<T> derivative_mobility_score() const;

    template <PIECE_COLOR moving_side>
    inline T piece_square_score() const;

    template <PIECE_COLOR moving_side>
    inline Evaluation_Weights<T> derivative_piece_square_score() const;

    // Helpers
    template <PIECE_COLOR side>
    inline T calculate_piece_mobility(const Moves_Bitboard_Matrix& matrix,
                                      PIECES                       piece) const;

  private:

    const Evaluation_Weights<T>& m_weights;
    const Chess_Board&           m_chess_board;
    const Moves_Bitboard_Matrix& m_moving_side_matrix;
    const Moves_Bitboard_Matrix& m_opposing_side_matrix;
};

template <typename T>
Evaluator<T>::Evaluator(const Evaluation_Weights<T>& weights,
                        const Chess_Board&           cb,
                        const Moves_Bitboard_Matrix& moving_side_matrix,
                        const Moves_Bitboard_Matrix& opposing_side_matrix) :
    m_weights(weights),
    m_chess_board(cb),
    m_moving_side_matrix(moving_side_matrix),
    m_opposing_side_matrix(opposing_side_matrix)
{
}

template <typename T>
T Evaluator<T>::evaluate_template_typed() const
{
    PIECE_COLOR moving_side = m_chess_board.get_side_to_move();

    T material;
    T mobility;

    if (moving_side == PIECE_COLOR::WHITE)
    {
        material = material_score<PIECE_COLOR::WHITE>();
        mobility = mobility_score<PIECE_COLOR::WHITE>();
    }
    else
    {
        material = material_score<PIECE_COLOR::BLACK>();
        mobility = mobility_score<PIECE_COLOR::BLACK>();
    }

    T evaluation = material + mobility;

    return evaluation;
}

template <typename T>
Score Evaluator<T>::evaluate() const
{
    T evaluation =
        Matrex_FP_Int(std::clamp(evaluate_template_typed().get_value(),
                                 FP_EVALUATION_MIN,
                                 FP_EVALUATION_MAX));
    Score return_value = Score(evaluation);
    return return_value;
}

template <typename T>
Evaluation_Weights<T> Evaluator<T>::derivative_evaluate() const
{
    PIECE_COLOR moving_side = m_chess_board.get_side_to_move();

    Evaluation_Weights<T> material;
    Evaluation_Weights<T> mobility;

    if (moving_side == PIECE_COLOR::WHITE)
    {
        material = derivative_material_score<PIECE_COLOR::WHITE>();
        mobility = derivative_mobility_score<PIECE_COLOR::WHITE>();
    }
    else
    {
        material = derivative_material_score<PIECE_COLOR::BLACK>();
        mobility = derivative_mobility_score<PIECE_COLOR::BLACK>();
    }

    Evaluation_Weights<T> evaluation = material + mobility;

    return evaluation;
}

template <typename T>
template <PIECE_COLOR moving_side>
inline T Evaluator<T>::material_score() const
{
    constexpr PIECE_COLOR opposing_side = (PIECE_COLOR) ((~moving_side) & 0x1);

    T return_value = explicit_fp_double_conversion<T>(0.0);

    for (uint8_t piece = PIECES::PAWN; piece <= PIECES::QUEEN; piece++)
    {
        T material_difference = explicit_fp_double_conversion<T>(0.0);

        material_difference +=
            (m_weights.material[piece]
             * m_chess_board.get_piece_occupancies(moving_side, (PIECES) piece)
                   .high_bit_count());
        material_difference -=
            (m_weights.material[piece]
             * m_chess_board
                   .get_piece_occupancies(opposing_side, (PIECES) piece)
                   .high_bit_count());
        T non_linear_material =
            Non_Linear_Response(m_weights.material_NLR_parameters[piece])
                .value(material_difference);
        return_value += non_linear_material;
    }

    return return_value;
}

template <typename T>
template <PIECE_COLOR moving_side>
inline Evaluation_Weights<T> Evaluator<T>::derivative_material_score() const
{
    constexpr PIECE_COLOR opposing_side = (PIECE_COLOR) ((~moving_side) & 0x1);

    Evaluation_Weights<T> grad;

    for (uint8_t piece = PIECES::PAWN; piece <= PIECES::QUEEN; ++piece)
    {
        const int c_plus =
            (int) m_chess_board
                .get_piece_occupancies(moving_side, (PIECES) piece)
                .high_bit_count();

        const int c_minus =
            (int) m_chess_board
                .get_piece_occupancies(opposing_side, (PIECES) piece)
                .high_bit_count();

        const int delta = c_plus - c_minus;
        const T   w     = m_weights.material[piece];
        const T   F     = w * (T) delta;

        Non_Linear_Response nlr(m_weights.material_NLR_parameters[piece]);

        const T dYdF = (T) nlr.partial_derivative_u(F);

        // 1) Gradient w.r.t. linear material weight w_p
        {
            const std::size_t idx_w =
                m_weights.get_index_of(m_weights.material[piece]);
            grad[idx_w] += dYdF * (T) delta;
        }

        // 2) Gradients w.r.t. this piece's NLR parameters
        {
            const auto& p = m_weights.material_NLR_parameters[piece];

            grad[m_weights.get_index_of(p.h_plus)] +=
                (T) nlr.partial_derivative_h_plus(F);
            grad[m_weights.get_index_of(p.h_minus)] +=
                (T) nlr.partial_derivative_h_minus(F);
            grad[m_weights.get_index_of(p.z)] +=
                (T) nlr.partial_derivative_z(F);
            grad[m_weights.get_index_of(p.k)] +=
                (T) nlr.partial_derivative_k(F);
            grad[m_weights.get_index_of(p.q_plus)] +=
                (T) nlr.partial_derivative_q_plus(F);
            grad[m_weights.get_index_of(p.q_minus)] +=
                (T) nlr.partial_derivative_q_minus(F);
            grad[m_weights.get_index_of(p.r_plus)] +=
                (T) nlr.partial_derivative_r_plus(F);
            grad[m_weights.get_index_of(p.r_minus)] +=
                (T) nlr.partial_derivative_r_minus(F);
            grad[m_weights.get_index_of(p.g_plus)] +=
                (T) nlr.partial_derivative_g_plus(F);
            grad[m_weights.get_index_of(p.g_minus)] +=
                (T) nlr.partial_derivative_g_minus(F);
        }
    }

    return grad;
}

template <typename T>
template <PIECE_COLOR moving_side>
inline T Evaluator<T>::mobility_score() const
{
    constexpr PIECE_COLOR opposing_side = (PIECE_COLOR) ((~moving_side) & 0x1);

    T mobility = explicit_fp_double_conversion<T>(0.0);

    for (uint8_t piece = PIECES::PAWN; piece <= PIECES::KING; piece++)
    {
        const T moving_side_piece_mobility =
            calculate_piece_mobility<moving_side>(m_moving_side_matrix,
                                                  (PIECES) piece);
        const T opposing_side_piece_mobility =
            calculate_piece_mobility<opposing_side>(m_opposing_side_matrix,
                                                    (PIECES) piece);
        const T piece_mobility_difference =
            moving_side_piece_mobility - opposing_side_piece_mobility;

        mobility +=
            Non_Linear_Response(m_weights.piece_mobility_NLR_parameters[piece])
                .value(piece_mobility_difference);
    }

    return static_cast<T>(mobility);
}

template <typename T>
template <PIECE_COLOR moving_side>
inline Evaluation_Weights<T> Evaluator<T>::derivative_mobility_score() const
{
    constexpr PIECE_COLOR opposing_side = (PIECE_COLOR) ((~moving_side) & 0x1);

    Attacks               a;
    Evaluation_Weights<T> derivative_weights;

    struct Counts
    {
        int16_t diagonal_movement   = 0;
        int16_t orthogonal_movement = 0;
        int16_t knight_movement     = 0;
        int16_t multi_movement      = 0;
        int16_t backwards_movement  = 0;
    };

    Counts piece_movement_counts[NUM_OF_UNIQUE_PIECES_PER_PLAYER];

    for (uint8_t piece = PIECES::PAWN; piece <= PIECES::KING; piece++)
    {
        std::vector<Moves_Bitboard> moving_side_moves_bitboards;
        m_moving_side_matrix.get_piece_moves_bitboards(
            moving_side,
            (PIECES) piece,
            moving_side_moves_bitboards);
        for (Moves_Bitboard& mb : moving_side_moves_bitboards)
        {
            const uint16_t diagonal_movements =
                (mb.bitboard
                 & (a.get_bishop_attacks(
                     mb.square,
                     m_chess_board.get_both_color_occupancies())))
                    .high_bit_count();
            const uint16_t orthogonal_movements =
                (mb.bitboard
                 & (a.get_rook_attacks(
                     mb.square,
                     m_chess_board.get_both_color_occupancies())))
                    .high_bit_count();
            const uint16_t backward_movements =
                mb.bitboard.get_backward_squares_mask(mb.square, moving_side)
                    .high_bit_count();
            const uint16_t multi_movement =
                ((diagonal_movements > 0) && (orthogonal_movements > 0));
            const uint16_t knight_movements =
                mb.bitboard.high_bit_count() * (piece == PIECES::KNIGHT);

            piece_movement_counts[piece].diagonal_movement +=
                diagonal_movements;
            piece_movement_counts[piece].orthogonal_movement +=
                orthogonal_movements;
            piece_movement_counts[piece].backwards_movement +=
                backward_movements;
            piece_movement_counts[piece].multi_movement  += multi_movement;
            piece_movement_counts[piece].knight_movement += knight_movements;
        }

        std::vector<Moves_Bitboard> opposing_side_moves_bitboards;
        m_opposing_side_matrix.get_piece_moves_bitboards(
            opposing_side,
            (PIECES) piece,
            opposing_side_moves_bitboards);
        for (Moves_Bitboard& mb : opposing_side_moves_bitboards)
        {
            const uint16_t diagonal_movements =
                (mb.bitboard
                 & (a.get_bishop_attacks(
                     mb.square,
                     m_chess_board.get_both_color_occupancies())))
                    .high_bit_count();
            const uint16_t orthogonal_movements =
                (mb.bitboard
                 & (a.get_rook_attacks(
                     mb.square,
                     m_chess_board.get_both_color_occupancies())))
                    .high_bit_count();
            const uint16_t backward_movements =
                mb.bitboard.get_backward_squares_mask(mb.square, opposing_side)
                    .high_bit_count();
            const uint16_t multi_movement =
                ((diagonal_movements > 0) && (orthogonal_movements > 0));
            const uint16_t knight_movements =
                mb.bitboard.high_bit_count() * (piece == PIECES::KNIGHT);

            piece_movement_counts[piece].diagonal_movement -=
                diagonal_movements;
            piece_movement_counts[piece].orthogonal_movement -=
                orthogonal_movements;
            piece_movement_counts[piece].backwards_movement -=
                backward_movements;
            piece_movement_counts[piece].multi_movement  -= multi_movement;
            piece_movement_counts[piece].knight_movement -= knight_movements;
        }
    }

    T f_k = piece_movement_counts[PIECES::KING].diagonal_movement
              * m_weights.diagonal_mobility
          + piece_movement_counts[PIECES::KING].orthogonal_movement
                * m_weights.orthogonal_mobility
          + piece_movement_counts[PIECES::KING].multi_movement
                * m_weights.multi_movement_mobility
          + piece_movement_counts[PIECES::KING].backwards_movement
                * m_weights.backwards_movement_mobility;
    T f_q = piece_movement_counts[PIECES::QUEEN].diagonal_movement
              * m_weights.diagonal_mobility
          + piece_movement_counts[PIECES::QUEEN].orthogonal_movement
                * m_weights.orthogonal_mobility
          + piece_movement_counts[PIECES::QUEEN].multi_movement
                * m_weights.multi_movement_mobility
          + piece_movement_counts[PIECES::QUEEN].backwards_movement
                * m_weights.backwards_movement_mobility;
    T f_r = piece_movement_counts[PIECES::ROOK].orthogonal_movement
              * m_weights.orthogonal_mobility
          + piece_movement_counts[PIECES::ROOK].backwards_movement
                * m_weights.backwards_movement_mobility;
    T f_b = piece_movement_counts[PIECES::BISHOP].diagonal_movement
              * m_weights.diagonal_mobility
          + piece_movement_counts[PIECES::BISHOP].backwards_movement
                * m_weights.backwards_movement_mobility;
    T f_n = piece_movement_counts[PIECES::KNIGHT].knight_movement
              * m_weights.knight_movement_mobility
          + piece_movement_counts[PIECES::KNIGHT].backwards_movement
                * m_weights.backwards_movement_mobility;
    T f_p = piece_movement_counts[PIECES::PAWN].diagonal_movement
              * m_weights.diagonal_mobility
          + piece_movement_counts[PIECES::PAWN].orthogonal_movement
                * m_weights.orthogonal_mobility
          + piece_movement_counts[PIECES::PAWN].multi_movement
                * m_weights.multi_movement_mobility;

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

    for (uint64_t i = 0; i < derivative_weights.get_size(); i++)
    {
        if (i == m_weights.get_index_of(m_weights.diagonal_mobility))
        {
            derivative_weights[i] =
                nlr_king.partial_derivative_u(f_k)
                    * piece_movement_counts[PIECES::KING].diagonal_movement
                + nlr_queen.partial_derivative_u(f_q)
                      * piece_movement_counts[PIECES::QUEEN].diagonal_movement
                + nlr_bishop.partial_derivative_u(f_b)
                      * piece_movement_counts[PIECES::BISHOP].diagonal_movement
                + nlr_pawn.partial_derivative_u(f_p)
                      * piece_movement_counts[PIECES::PAWN].diagonal_movement;
        }
        else if (i == m_weights.get_index_of(m_weights.orthogonal_mobility))
        {
            derivative_weights[i] =
                nlr_king.partial_derivative_u(f_k)
                    * piece_movement_counts[PIECES::KING].orthogonal_movement
                + nlr_queen.partial_derivative_u(f_q)
                      * piece_movement_counts[PIECES::QUEEN].orthogonal_movement
                + nlr_rook.partial_derivative_u(f_r)
                      * piece_movement_counts[PIECES::ROOK].orthogonal_movement
                + nlr_pawn.partial_derivative_u(f_p)
                      * piece_movement_counts[PIECES::PAWN].orthogonal_movement;
        }
        else if (i
                 == m_weights.get_index_of(m_weights.knight_movement_mobility))
        {
            derivative_weights[i] =
                nlr_knight.partial_derivative_u(f_n)
                * piece_movement_counts[PIECES::KNIGHT].knight_movement;
        }
        else if (i == m_weights.get_index_of(m_weights.multi_movement_mobility))
        {
            derivative_weights[i] =
                nlr_king.partial_derivative_u(f_k)
                    * piece_movement_counts[PIECES::KING].multi_movement
                + nlr_queen.partial_derivative_u(f_q)
                      * piece_movement_counts[PIECES::QUEEN].multi_movement
                + nlr_pawn.partial_derivative_u(f_p)
                      * piece_movement_counts[PIECES::PAWN].multi_movement;
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.backwards_movement_mobility))
        {
            derivative_weights[i] =
                nlr_king.partial_derivative_u(f_k)
                    * piece_movement_counts[PIECES::KING].backwards_movement
                + nlr_queen.partial_derivative_u(f_q)
                      * piece_movement_counts[PIECES::QUEEN].backwards_movement
                + nlr_rook.partial_derivative_u(f_r)
                      * piece_movement_counts[PIECES::ROOK].backwards_movement
                + nlr_bishop.partial_derivative_u(f_b)
                      * piece_movement_counts[PIECES::BISHOP].backwards_movement
                + nlr_knight.partial_derivative_u(f_n)
                      * piece_movement_counts[PIECES::KNIGHT]
                            .backwards_movement;
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::KING]
                         .h_plus))
        {
            derivative_weights[i] = nlr_king.partial_derivative_h_plus(f_k);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::KING]
                         .h_minus))
        {
            derivative_weights[i] = nlr_king.partial_derivative_h_minus(f_k);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::KING].z))
        {
            derivative_weights[i] = nlr_king.partial_derivative_z(f_k);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::KING].k))
        {
            derivative_weights[i] = nlr_king.partial_derivative_k(f_k);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::KING]
                         .q_plus))
        {
            derivative_weights[i] = nlr_king.partial_derivative_q_plus(f_k);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::KING]
                         .q_minus))
        {
            derivative_weights[i] = nlr_king.partial_derivative_q_minus(f_k);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::KING]
                         .r_plus))
        {
            derivative_weights[i] = nlr_king.partial_derivative_r_plus(f_k);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::KING]
                         .r_minus))
        {
            derivative_weights[i] = nlr_king.partial_derivative_r_minus(f_k);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::KING]
                         .g_plus))
        {
            derivative_weights[i] = nlr_king.partial_derivative_g_plus(f_k);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::KING]
                         .g_minus))
        {
            derivative_weights[i] = nlr_king.partial_derivative_g_minus(f_k);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::QUEEN]
                         .h_plus))
        {
            derivative_weights[i] = nlr_queen.partial_derivative_h_plus(f_q);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::QUEEN]
                         .h_minus))
        {
            derivative_weights[i] = nlr_queen.partial_derivative_h_minus(f_q);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::QUEEN].z))
        {
            derivative_weights[i] = nlr_queen.partial_derivative_z(f_q);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::QUEEN].k))
        {
            derivative_weights[i] = nlr_queen.partial_derivative_k(f_q);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::QUEEN]
                         .q_plus))
        {
            derivative_weights[i] = nlr_queen.partial_derivative_q_plus(f_q);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::QUEEN]
                         .q_minus))
        {
            derivative_weights[i] = nlr_queen.partial_derivative_q_minus(f_q);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::QUEEN]
                         .r_plus))
        {
            derivative_weights[i] = nlr_queen.partial_derivative_r_plus(f_q);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::QUEEN]
                         .r_minus))
        {
            derivative_weights[i] = nlr_queen.partial_derivative_r_minus(f_q);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::QUEEN]
                         .g_plus))
        {
            derivative_weights[i] = nlr_queen.partial_derivative_g_plus(f_q);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::QUEEN]
                         .g_minus))
        {
            derivative_weights[i] = nlr_queen.partial_derivative_g_minus(f_q);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::ROOK]
                         .h_plus))
        {
            derivative_weights[i] = nlr_rook.partial_derivative_h_plus(f_r);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::ROOK]
                         .h_minus))
        {
            derivative_weights[i] = nlr_rook.partial_derivative_h_minus(f_r);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::ROOK].z))
        {
            derivative_weights[i] = nlr_rook.partial_derivative_z(f_r);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::ROOK].k))
        {
            derivative_weights[i] = nlr_rook.partial_derivative_k(f_r);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::ROOK]
                         .q_plus))
        {
            derivative_weights[i] = nlr_rook.partial_derivative_q_plus(f_r);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::ROOK]
                         .q_minus))
        {
            derivative_weights[i] = nlr_rook.partial_derivative_q_minus(f_r);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::ROOK]
                         .r_plus))
        {
            derivative_weights[i] = nlr_rook.partial_derivative_r_plus(f_r);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::ROOK]
                         .r_minus))
        {
            derivative_weights[i] = nlr_rook.partial_derivative_r_minus(f_r);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::ROOK]
                         .g_plus))
        {
            derivative_weights[i] = nlr_rook.partial_derivative_g_plus(f_r);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::ROOK]
                         .g_minus))
        {
            derivative_weights[i] = nlr_rook.partial_derivative_g_minus(f_r);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::BISHOP]
                         .h_plus))
        {
            derivative_weights[i] = nlr_bishop.partial_derivative_h_plus(f_b);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::BISHOP]
                         .h_minus))
        {
            derivative_weights[i] = nlr_bishop.partial_derivative_h_minus(f_b);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::BISHOP].z))
        {
            derivative_weights[i] = nlr_bishop.partial_derivative_z(f_b);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::BISHOP].k))
        {
            derivative_weights[i] = nlr_bishop.partial_derivative_k(f_b);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::BISHOP]
                         .q_plus))
        {
            derivative_weights[i] = nlr_bishop.partial_derivative_q_plus(f_b);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::BISHOP]
                         .q_minus))
        {
            derivative_weights[i] = nlr_bishop.partial_derivative_q_minus(f_b);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::BISHOP]
                         .r_plus))
        {
            derivative_weights[i] = nlr_bishop.partial_derivative_r_plus(f_b);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::BISHOP]
                         .r_minus))
        {
            derivative_weights[i] = nlr_bishop.partial_derivative_r_minus(f_b);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::BISHOP]
                         .g_plus))
        {
            derivative_weights[i] = nlr_bishop.partial_derivative_g_plus(f_b);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::BISHOP]
                         .g_minus))
        {
            derivative_weights[i] = nlr_bishop.partial_derivative_g_minus(f_b);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::KNIGHT]
                         .h_plus))
        {
            derivative_weights[i] = nlr_knight.partial_derivative_h_plus(f_n);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::KNIGHT]
                         .h_minus))
        {
            derivative_weights[i] = nlr_knight.partial_derivative_h_minus(f_n);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::KNIGHT].z))
        {
            derivative_weights[i] = nlr_knight.partial_derivative_z(f_n);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::KNIGHT].k))
        {
            derivative_weights[i] = nlr_knight.partial_derivative_k(f_n);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::KNIGHT]
                         .q_plus))
        {
            derivative_weights[i] = nlr_knight.partial_derivative_q_plus(f_n);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::KNIGHT]
                         .q_minus))
        {
            derivative_weights[i] = nlr_knight.partial_derivative_q_minus(f_n);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::KNIGHT]
                         .r_plus))
        {
            derivative_weights[i] = nlr_knight.partial_derivative_r_plus(f_n);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::KNIGHT]
                         .r_minus))
        {
            derivative_weights[i] = nlr_knight.partial_derivative_r_minus(f_n);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::KNIGHT]
                         .g_plus))
        {
            derivative_weights[i] = nlr_knight.partial_derivative_g_plus(f_n);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::KNIGHT]
                         .g_minus))
        {
            derivative_weights[i] = nlr_knight.partial_derivative_g_minus(f_n);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::PAWN]
                         .h_plus))
        {
            derivative_weights[i] = nlr_pawn.partial_derivative_h_plus(f_p);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::PAWN]
                         .h_minus))
        {
            derivative_weights[i] = nlr_pawn.partial_derivative_h_minus(f_p);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::PAWN].z))
        {
            derivative_weights[i] = nlr_pawn.partial_derivative_z(f_p);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::PAWN].k))
        {
            derivative_weights[i] = nlr_pawn.partial_derivative_k(f_p);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::PAWN]
                         .q_plus))
        {
            derivative_weights[i] = nlr_pawn.partial_derivative_q_plus(f_p);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::PAWN]
                         .q_minus))
        {
            derivative_weights[i] = nlr_pawn.partial_derivative_q_minus(f_p);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::PAWN]
                         .r_plus))
        {
            derivative_weights[i] = nlr_pawn.partial_derivative_r_plus(f_p);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::PAWN]
                         .r_minus))
        {
            derivative_weights[i] = nlr_pawn.partial_derivative_r_minus(f_p);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::PAWN]
                         .g_plus))
        {
            derivative_weights[i] = nlr_pawn.partial_derivative_g_plus(f_p);
        }
        else if (i
                 == m_weights.get_index_of(
                     m_weights.piece_mobility_NLR_parameters[PIECES::PAWN]
                         .g_minus))
        {
            derivative_weights[i] = nlr_pawn.partial_derivative_g_minus(f_p);
        }
        else { derivative_weights[i] = 0; }
    }

    return derivative_weights;
}

template <typename T>
template <PIECE_COLOR moving_side>
inline T Evaluator<T>::piece_square_score() const
{
    constexpr PIECE_COLOR opposing_side = ~moving_side;

    // Accumulate the piece-square values from the piece-square tables for the
    // present state of the board. The accumulations are per piece per side.
    multi_array<T, NUM_OF_PLAYERS, NUM_OF_UNIQUE_PIECES_PER_PLAYER> color_piece_values{};
    for (uint8_t color = PIECE_COLOR::WHITE; color <= PIECE_COLOR::BLACK; color++)
    {
        for (uint8_t piece = PIECES::PAWN; piece <= PIECES::KING; piece++)
        {
            for (uint8_t square_idx = 0; square_idx < NUM_OF_SQUARES_ON_CHESS_BOARD; square_idx++)
            {
                color_piece_values[color][piece] += (m_chess_board.get_piece_occupancies((PIECE_COLOR) color, (PIECES) piece).get_square(Square(square_idx)) > 0)
                * m_weights.piece_square_tables[color][piece][square_idx];
            }
        }
    }

    // NLR objects for this side's pieces.
    const Non_Linear_Response<T> nlr_this_king(
        m_weights.piece_square_NLR_parameters[moving_side][PIECES::KING]);
    const Non_Linear_Response<T> nlr_this_queen(
        m_weights.piece_square_NLR_parameters[moving_side][PIECES::QUEEN]);
    const Non_Linear_Response<T> nlr_this_rook(
        m_weights.piece_square_NLR_parameters[moving_side][PIECES::ROOK]);
    const Non_Linear_Response<T> nlr_this_bishop(
        m_weights.piece_square_NLR_parameters[moving_side][PIECES::BISHOP]);
    const Non_Linear_Response<T> nlr_this_knight(
        m_weights.piece_square_NLR_parameters[moving_side][PIECES::KNIGHT]);
    const Non_Linear_Response<T> nlr_this_pawn(
        m_weights.piece_square_NLR_parameters[moving_side][PIECES::PAWN]);

    // NLR values for this side's pieces and their interaction.
    const T nlr_this_king_value = nlr_this_king.value(color_piece_values[moving_side][PIECES::KING]);
    const T nlr_this_queen_value = nlr_this_queen.value(color_piece_values[moving_side][PIECES::QUEEN]);
    const T nlr_this_rook_value = nlr_this_rook.value(color_piece_values[moving_side][PIECES::ROOK]);
    const T nlr_this_bishop_value = nlr_this_bishop.value(color_piece_values[moving_side][PIECES::BISHOP]);
    const T nlr_this_knight_value = nlr_this_knight.value(color_piece_values[moving_side][PIECES::KNIGHT]);
    const T nlr_this_pawn_value = nlr_this_pawn.value(color_piece_values[moving_side][PIECES::PAWN]);
    const T nlr_this_interaction_value = nlr_this_king_value * nlr_this_queen_value * nlr_this_rook_value * nlr_this_bishop_value * nlr_this_knight_value * nlr_this_pawn_value;

    // NLR objects for opposing side's pieces.
    const Non_Linear_Response<T> nlr_opposing_king(
        m_weights.piece_square_NLR_parameters[opposing_side][PIECES::KING]);
    const Non_Linear_Response<T> nlr_opposing_queen(
        m_weights.piece_square_NLR_parameters[opposing_side][PIECES::QUEEN]);
    const Non_Linear_Response<T> nlr_opposing_rook(
        m_weights.piece_square_NLR_parameters[opposing_side][PIECES::ROOK]);
    const Non_Linear_Response<T> nlr_opposing_bishop(
        m_weights.piece_square_NLR_parameters[opposing_side][PIECES::BISHOP]);
    const Non_Linear_Response<T> nlr_opposing_knight(
        m_weights.piece_square_NLR_parameters[opposing_side][PIECES::KNIGHT]);
    const Non_Linear_Response<T> nlr_opposing_pawn(
        m_weights.piece_square_NLR_parameters[opposing_side][PIECES::PAWN]);

    // NLR values for opposing side's pieces and their interaction.
    const T nlr_opposing_king_value = nlr_opposing_king.value(color_piece_values[opposing_side][PIECES::KING]);
    const T nlr_opposing_queen_value = nlr_opposing_queen.value(color_piece_values[opposing_side][PIECES::QUEEN]);
    const T nlr_opposing_rook_value = nlr_opposing_rook.value(color_piece_values[opposing_side][PIECES::ROOK]);
    const T nlr_opposing_bishop_value = nlr_opposing_bishop.value(color_piece_values[opposing_side][PIECES::BISHOP]);
    const T nlr_opposing_knight_value = nlr_opposing_knight.value(color_piece_values[opposing_side][PIECES::KNIGHT]);
    const T nlr_opposing_pawn_value = nlr_opposing_pawn.value(color_piece_values[opposing_side][PIECES::PAWN]);
    const T nlr_opposing_interaction_value = nlr_opposing_king_value * nlr_opposing_queen_value * nlr_opposing_rook_value * nlr_opposing_bishop_value * nlr_opposing_knight_value * nlr_opposing_pawn_value;

    // Explicit interactive term NLR objects per side.
    const Non_Linear_Response<T> nlr_this_side(m_weights.interactive_piece_square_NLR_parameters[moving_side]);
    const Non_Linear_Response<T> nlr_opposing_side(m_weights.interactive_piece_square_NLR_parameters[opposing_side]);

    // Explicit interactive term NLR values per side.
    const T nlr_this_value = nlr_this_side.value(nlr_this_interaction_value);
    const T nlr_opposing_value = nlr_opposing_side.value(nlr_opposing_interaction_value);

    return (nlr_this_value + nlr_this_king_value + nlr_this_queen_value + nlr_this_rook_value + nlr_this_bishop_value + nlr_this_knight_value + nlr_this_pawn_value) - 
           (nlr_opposing_value + nlr_opposing_king_value + nlr_opposing_queen_value + nlr_opposing_rook_value + nlr_opposing_bishop_value + nlr_opposing_knight_value + nlr_opposing_pawn_value);
}

// template <typename T>
// template <PIECE_COLOR moving_side>
// inline Evaluation_Weights<T> Evaluator<T>::derivative_piece_square_score() const
// {

// }

/*******************************************************************************
 *
 * HELPER FUNCTIONS FOR EVALUATOR
 *
 *******************************************************************************/

template <typename T>
template <PIECE_COLOR side>
inline T
Evaluator<T>::calculate_piece_mobility(const Moves_Bitboard_Matrix& matrix,
                                       PIECES                       piece) const
{
    Attacks a;

    std::vector<Moves_Bitboard> moves_bitboards;
    matrix.get_piece_moves_bitboards(side, piece, moves_bitboards);
    T piece_mobility = explicit_fp_double_conversion<T>(0.0);
    for (Moves_Bitboard& mb : moves_bitboards)
    {
        const Bitboard diagonal_movements =
            mb.bitboard
            & (a.get_bishop_attacks(
                mb.square,
                m_chess_board.get_both_color_occupancies()));
        const Bitboard orthogonal_movements =
            mb.bitboard
            & (a.get_rook_attacks(mb.square,
                                  m_chess_board.get_both_color_occupancies()));
        const Bitboard backward_movements =
            mb.bitboard.get_backward_squares_mask(mb.square, side);

        const T diagonal_mobility =
            diagonal_movements.high_bit_count() * m_weights.diagonal_mobility;
        const T orthogonal_mobility = orthogonal_movements.high_bit_count()
                                    * m_weights.orthogonal_mobility;
        const T backward_mobility = backward_movements.high_bit_count()
                                  * m_weights.backwards_movement_mobility;
        const T multi_movement_mobility =
            ((diagonal_movements.high_bit_count() > 0)
             && (orthogonal_movements.high_bit_count() > 0))
            * m_weights.multi_movement_mobility;
        const T knight_movements_mobility = mb.bitboard.high_bit_count()
                                          * (piece == PIECES::KNIGHT)
                                          * m_weights.knight_movement_mobility;

        piece_mobility +=
            (diagonal_mobility + orthogonal_mobility + backward_mobility
             + multi_movement_mobility + knight_movements_mobility);
    }

    return piece_mobility;
}
