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

    using Piece_Square_Table_Type = multi_array<T,
                                                NUM_OF_PLAYERS,
                                                NUM_OF_UNIQUE_PIECES_PER_PLAYER,
                                                NUM_OF_SQUARES_ON_CHESS_BOARD>;

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
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters,
                                PIECE_COLOR::WHITE,
                                PIECES::PAWN),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters,
                                PIECE_COLOR::WHITE,
                                PIECES::KNIGHT),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters,
                                PIECE_COLOR::WHITE,
                                PIECES::BISHOP),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters,
                                PIECE_COLOR::WHITE,
                                PIECES::ROOK),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters,
                                PIECE_COLOR::WHITE,
                                PIECES::QUEEN),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters,
                                PIECE_COLOR::WHITE,
                                PIECES::KING),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters,
                                PIECE_COLOR::BLACK,
                                PIECES::PAWN),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters,
                                PIECE_COLOR::BLACK,
                                PIECES::KNIGHT),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters,
                                PIECE_COLOR::BLACK,
                                PIECES::BISHOP),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters,
                                PIECE_COLOR::BLACK,
                                PIECES::ROOK),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters,
                                PIECE_COLOR::BLACK,
                                PIECES::QUEEN),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters,
                                PIECE_COLOR::BLACK,
                                PIECES::KING),
            piece_square_tables,
            NLR_ARRAY_FIELDS(interactive_piece_square_NLR_parameters,
                             PIECE_COLOR::WHITE),
            NLR_ARRAY_FIELDS(interactive_piece_square_NLR_parameters,
                             PIECE_COLOR::BLACK))
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
        multi_array<NLR_Parameters<T>,
                    NUM_OF_PLAYERS,
                    NUM_OF_UNIQUE_PIECES_PER_PLAYER> piece_square_NLR_weights,
        Piece_Square_Table_Type                      piece_square_weights,
        multi_array<NLR_Parameters<T>, NUM_OF_PLAYERS>
            interactive_piece_square_NLR_weights) :
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
        interactive_piece_square_NLR_parameters(
            interactive_piece_square_NLR_weights),
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
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters,
                                PIECE_COLOR::WHITE,
                                PIECES::PAWN),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters,
                                PIECE_COLOR::WHITE,
                                PIECES::KNIGHT),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters,
                                PIECE_COLOR::WHITE,
                                PIECES::BISHOP),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters,
                                PIECE_COLOR::WHITE,
                                PIECES::ROOK),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters,
                                PIECE_COLOR::WHITE,
                                PIECES::QUEEN),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters,
                                PIECE_COLOR::WHITE,
                                PIECES::KING),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters,
                                PIECE_COLOR::BLACK,
                                PIECES::PAWN),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters,
                                PIECE_COLOR::BLACK,
                                PIECES::KNIGHT),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters,
                                PIECE_COLOR::BLACK,
                                PIECES::BISHOP),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters,
                                PIECE_COLOR::BLACK,
                                PIECES::ROOK),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters,
                                PIECE_COLOR::BLACK,
                                PIECES::QUEEN),
            NLR_2D_ARRAY_FIELDS(piece_square_NLR_parameters,
                                PIECE_COLOR::BLACK,
                                PIECES::KING),
            piece_square_tables,
            NLR_ARRAY_FIELDS(interactive_piece_square_NLR_parameters,
                             PIECE_COLOR::WHITE),
            NLR_ARRAY_FIELDS(interactive_piece_square_NLR_parameters,
                             PIECE_COLOR::BLACK))
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
    multi_array<NLR_Parameters<T>,
                NUM_OF_PLAYERS,
                NUM_OF_UNIQUE_PIECES_PER_PLAYER>
                            piece_square_NLR_parameters;
    Piece_Square_Table_Type piece_square_tables;
    multi_array<NLR_Parameters<T>, NUM_OF_PLAYERS>
        interactive_piece_square_NLR_parameters;

    T&       operator[](std::size_t index);
    const T& operator[](std::size_t index) const;

    // Arithmetic operators with another evaluation weights.
    Evaluation_Weights operator+(const Evaluation_Weights& other) const;
    Evaluation_Weights operator-(const Evaluation_Weights& other) const;
    Evaluation_Weights operator/(const Evaluation_Weights& other) const;
    Evaluation_Weights operator*(const Evaluation_Weights& other) const;

    Evaluation_Weights& operator+=(const Evaluation_Weights& other);
    Evaluation_Weights& operator-=(const Evaluation_Weights& other);
    Evaluation_Weights& operator/=(const Evaluation_Weights& other);
    Evaluation_Weights& operator*=(const Evaluation_Weights& other);
    Evaluation_Weights operator-() const;

    // Arithmetic operators with T.
    Evaluation_Weights operator+(const T value) const;
    Evaluation_Weights operator-(const T value) const;
    Evaluation_Weights operator*(const T value) const;
    Evaluation_Weights operator/(const T value) const;

    Evaluation_Weights& operator+=(const T other);
    Evaluation_Weights& operator-=(const T other);
    Evaluation_Weights& operator/=(const T other);
    Evaluation_Weights& operator*=(const T other);

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
    return m_weight_ref_array.get_array()[index].value().get();
}

template <typename T>
const T& Evaluation_Weights<T>::operator[](std::size_t index) const
{
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
Evaluation_Weights<T>& Evaluation_Weights<T>::operator+=(const Evaluation_Weights<T>& other)
{
    Evaluation_Weights<T> sum = (*this) + other;

    for (std::size_t i = 0; i < get_size(); ++i) { (*this)[i] = sum[i]; }

    return *this;
}

template <typename T>
Evaluation_Weights<T>& Evaluation_Weights<T>::operator-=(const Evaluation_Weights<T>& other)
{
    Evaluation_Weights<T> difference = (*this) - other;

    for (std::size_t i = 0; i < get_size(); ++i) { (*this)[i] = difference[i]; }

    return *this;
}

template <typename T>
Evaluation_Weights<T>& Evaluation_Weights<T>::operator/=(const Evaluation_Weights& other)
{
    Evaluation_Weights<T> quotient = (*this) / other;

    for (std::size_t i = 0; i < get_size(); ++i) { (*this)[i] = quotient[i]; }

    return *this;
}

template <typename T>
Evaluation_Weights<T>& Evaluation_Weights<T>::operator*=(const Evaluation_Weights& other)
{
    Evaluation_Weights<T> product = (*this) * other;

    for (std::size_t i = 0; i < get_size(); ++i) { (*this)[i] = product[i]; }

    return *this;
}

template <typename T>
Evaluation_Weights<T> Evaluation_Weights<T>::operator-() const
{
    Evaluation_Weights result;

    for (std::size_t i = 0; i < m_weight_ref_array.size; ++i)
    {
        result[i] = -m_weight_ref_array.get_array()[i].value().get();
    }

    return result;
}

template <typename T>
Evaluation_Weights<T> Evaluation_Weights<T>::operator+(const T value) const
{
    Evaluation_Weights result;

    for (std::size_t i = 0; i < m_weight_ref_array.size; ++i)
    {
        result[i] = m_weight_ref_array.get_array()[i].value().get() + value;
    }

    return result;
}

template <typename T>
Evaluation_Weights<T> Evaluation_Weights<T>::operator-(const T value) const
{
    Evaluation_Weights result;

    for (std::size_t i = 0; i < m_weight_ref_array.size; ++i)
    {
        result[i] = m_weight_ref_array.get_array()[i].value().get() - value;
    }

    return result;
}

template <typename T>
Evaluation_Weights<T> Evaluation_Weights<T>::operator*(const T value) const
{
    Evaluation_Weights result;

    for (std::size_t i = 0; i < m_weight_ref_array.size; ++i)
    {
        result[i] = m_weight_ref_array.get_array()[i].value().get() * value;
    }

    return result;
}

template <typename T>
Evaluation_Weights<T> Evaluation_Weights<T>::operator/(const T value) const
{
    Evaluation_Weights result;

    for (std::size_t i = 0; i < m_weight_ref_array.size; ++i)
    {
        result[i] = m_weight_ref_array.get_array()[i].value().get() / value;
    }

    return result;
}

template <typename T>
Evaluation_Weights<T>& Evaluation_Weights<T>::operator+=(const T other)
{
    Evaluation_Weights<T> sum = (*this) + other;

    for (std::size_t i = 0; i < get_size(); ++i) { (*this)[i] = sum[i]; }

    return *this;
}

template <typename T>
Evaluation_Weights<T>& Evaluation_Weights<T>::operator-=(const T other)
{
    Evaluation_Weights<T> difference = (*this) - other;

    for (std::size_t i = 0; i < get_size(); ++i) { (*this)[i] = difference[i]; }

    return *this;
}

template <typename T>
Evaluation_Weights<T>& Evaluation_Weights<T>::operator/=(const T other)
{
    Evaluation_Weights<T> quotient = (*this) / other;

    for (std::size_t i = 0; i < get_size(); ++i) { (*this)[i] = quotient[i]; }

    return *this;
}

template <typename T>
Evaluation_Weights<T>& Evaluation_Weights<T>::operator*=(const T other)
{
    Evaluation_Weights<T> product = (*this) * other;

    for (std::size_t i = 0; i < get_size(); ++i) { (*this)[i] = product[i]; }

    return *this;
}

template <typename T>
Evaluation_Weights<T> operator/(const T                      scalar,
                                const Evaluation_Weights<T>& weights)
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
        else
        {
            os << weights[i];
        }
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

    T     evaluate_template_typed() const;
    Score evaluate() const;

    template <PIECE_COLOR moving_side>
    inline T material_score() const;

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
    T piece_square;

    if (moving_side == PIECE_COLOR::WHITE)
    {
        material     = material_score<PIECE_COLOR::WHITE>();
        mobility     = mobility_score<PIECE_COLOR::WHITE>();
        piece_square = piece_square_score<PIECE_COLOR::WHITE>();
    }
    else
    {
        material     = material_score<PIECE_COLOR::BLACK>();
        mobility     = mobility_score<PIECE_COLOR::BLACK>();
        piece_square = piece_square_score<PIECE_COLOR::BLACK>();
    }

    T evaluation = material + mobility + piece_square;

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
inline T Evaluator<T>::piece_square_score() const
{
    constexpr PIECE_COLOR opposing_side = ~moving_side;

    // Accumulate the piece-square values from the piece-square tables for the
    // present state of the board. The accumulations are per piece per side.
    multi_array<T, NUM_OF_PLAYERS, NUM_OF_UNIQUE_PIECES_PER_PLAYER>
        color_piece_values {};
    for (uint8_t color = PIECE_COLOR::WHITE; color <= PIECE_COLOR::BLACK;
         color++)
    {
        for (uint8_t piece = PIECES::PAWN; piece <= PIECES::KING; piece++)
        {
            for (uint8_t square_idx = 0;
                 square_idx < NUM_OF_SQUARES_ON_CHESS_BOARD;
                 square_idx++)
            {
                color_piece_values[color][piece] +=
                    (m_chess_board
                         .get_piece_occupancies((PIECE_COLOR) color,
                                                (PIECES) piece)
                         .get_square(Square(square_idx))
                     > 0)
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
    const T nlr_this_king_value =
        nlr_this_king.value(color_piece_values[moving_side][PIECES::KING]);
    const T nlr_this_queen_value =
        nlr_this_queen.value(color_piece_values[moving_side][PIECES::QUEEN]);
    const T nlr_this_rook_value =
        nlr_this_rook.value(color_piece_values[moving_side][PIECES::ROOK]);
    const T nlr_this_bishop_value =
        nlr_this_bishop.value(color_piece_values[moving_side][PIECES::BISHOP]);
    const T nlr_this_knight_value =
        nlr_this_knight.value(color_piece_values[moving_side][PIECES::KNIGHT]);
    const T nlr_this_pawn_value =
        nlr_this_pawn.value(color_piece_values[moving_side][PIECES::PAWN]);
    const T nlr_this_interaction_value =
        nlr_this_king_value * nlr_this_queen_value * nlr_this_rook_value
        * nlr_this_bishop_value * nlr_this_knight_value * nlr_this_pawn_value;

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
    const T nlr_opposing_king_value = nlr_opposing_king.value(
        color_piece_values[opposing_side][PIECES::KING]);
    const T nlr_opposing_queen_value = nlr_opposing_queen.value(
        color_piece_values[opposing_side][PIECES::QUEEN]);
    const T nlr_opposing_rook_value = nlr_opposing_rook.value(
        color_piece_values[opposing_side][PIECES::ROOK]);
    const T nlr_opposing_bishop_value = nlr_opposing_bishop.value(
        color_piece_values[opposing_side][PIECES::BISHOP]);
    const T nlr_opposing_knight_value = nlr_opposing_knight.value(
        color_piece_values[opposing_side][PIECES::KNIGHT]);
    const T nlr_opposing_pawn_value = nlr_opposing_pawn.value(
        color_piece_values[opposing_side][PIECES::PAWN]);
    const T nlr_opposing_interaction_value =
        nlr_opposing_king_value * nlr_opposing_queen_value
        * nlr_opposing_rook_value * nlr_opposing_bishop_value
        * nlr_opposing_knight_value * nlr_opposing_pawn_value;

    // Explicit interactive term NLR objects per side.
    const Non_Linear_Response<T> nlr_this_side(
        m_weights.interactive_piece_square_NLR_parameters[moving_side]);
    const Non_Linear_Response<T> nlr_opposing_side(
        m_weights.interactive_piece_square_NLR_parameters[opposing_side]);

    // Explicit interactive term NLR values per side.
    const T nlr_this_value = nlr_this_side.value(nlr_this_interaction_value);
    const T nlr_opposing_value =
        nlr_opposing_side.value(nlr_opposing_interaction_value);

    return (nlr_this_value + nlr_this_king_value + nlr_this_queen_value
            + nlr_this_rook_value + nlr_this_bishop_value
            + nlr_this_knight_value + nlr_this_pawn_value)
         - (nlr_opposing_value + nlr_opposing_king_value
            + nlr_opposing_queen_value + nlr_opposing_rook_value
            + nlr_opposing_bishop_value + nlr_opposing_knight_value
            + nlr_opposing_pawn_value);
}

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
