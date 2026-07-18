#pragma once

#include "globals.hpp"
#include "non_linear_response.hpp"

template <typename T>
class Evaluation_Weights
{

    using Piece_Square_Table_Type = Multi_Array<T,
                                                NUM_OF_PLAYERS,
                                                NUM_OF_UNIQUE_PIECES_PER_PLAYER,
                                                NUM_OF_SQUARES_ON_CHESS_BOARD>;

    // clang-format off
  using Evaluation_Weights_Reference_Array = Reference_Array<
      T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,
      T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,
      T, Multi_Array<T, (NUM_OF_UNIQUE_PIECES_PER_PLAYER - 1)>, T, T, T, T, T,
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
        Multi_Array<NLR_Parameters<T>, (NUM_OF_UNIQUE_PIECES_PER_PLAYER - 1)>
            material_NLR_weights,
        Multi_Array<T, (NUM_OF_UNIQUE_PIECES_PER_PLAYER - 1)> material_weights,
        Multi_Array<NLR_Parameters<T>, NUM_OF_UNIQUE_PIECES_PER_PLAYER>
            piece_mobility_NLR_weights,
        T   diagonal_mobility_weight,
        T   orthogonal_mobility_weight,
        T   knight_movement_mobility_weight,
        T   multi_movement_mobility_weight,
        T   backwards_movement_mobility_weight,
        Multi_Array<NLR_Parameters<T>,
                    NUM_OF_PLAYERS,
                    NUM_OF_UNIQUE_PIECES_PER_PLAYER> piece_square_NLR_weights,
        Piece_Square_Table_Type                      piece_square_weights,
        Multi_Array<NLR_Parameters<T>, NUM_OF_PLAYERS>
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
    Multi_Array<NLR_Parameters<T>, (NUM_OF_UNIQUE_PIECES_PER_PLAYER - 1)>
        material_NLR_parameters;
    Multi_Array<T, (NUM_OF_UNIQUE_PIECES_PER_PLAYER - 1)> material;

    // Mobility weights.
    Multi_Array<NLR_Parameters<T>, NUM_OF_UNIQUE_PIECES_PER_PLAYER>
        piece_mobility_NLR_parameters;
    T   diagonal_mobility;
    T   orthogonal_mobility;
    T   knight_movement_mobility;
    T   multi_movement_mobility;
    T   backwards_movement_mobility;

    // Piece Square Tables
    Multi_Array<NLR_Parameters<T>,
                NUM_OF_PLAYERS,
                NUM_OF_UNIQUE_PIECES_PER_PLAYER>
                            piece_square_NLR_parameters;
    Piece_Square_Table_Type piece_square_tables;
    Multi_Array<NLR_Parameters<T>, NUM_OF_PLAYERS>
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
    Evaluation_Weights  operator-() const;

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

// This copy constructor avoids dangling references by first default
// constructing a reference array (creating bounded references to the member
// variables) and then assigning the reference array values from the other
// evaluation weights (keep in mind, raw references cannot be rebound).
template <typename T>
Evaluation_Weights<T>::Evaluation_Weights(const Evaluation_Weights& other) :
    Evaluation_Weights()
{
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

// Move constructor - same as copy constructor.
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
    return m_weight_ref_array[index];
}

template <typename T>
const T& Evaluation_Weights<T>::operator[](std::size_t index) const
{
    return m_weight_ref_array[index];
}

template <typename T>
Evaluation_Weights<T>
Evaluation_Weights<T>::operator+(const Evaluation_Weights& other) const
{
    Evaluation_Weights result;

    for (std::size_t i = 0; i < m_weight_ref_array.size; ++i)
    {
        result[i] = m_weight_ref_array[i] + other.m_weight_ref_array[i];
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
        result[i] = m_weight_ref_array[i] - other.m_weight_ref_array[i];
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
        result[i] = m_weight_ref_array[i] / other.m_weight_ref_array[i];
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
        result[i] = m_weight_ref_array[i] * other.m_weight_ref_array[i];
    }

    return result;
}

template <typename T>
Evaluation_Weights<T>&
Evaluation_Weights<T>::operator+=(const Evaluation_Weights<T>& other)
{
    Evaluation_Weights<T> sum = (*this) + other;

    for (std::size_t i = 0; i < get_size(); ++i) { (*this)[i] = sum[i]; }

    return *this;
}

template <typename T>
Evaluation_Weights<T>&
Evaluation_Weights<T>::operator-=(const Evaluation_Weights<T>& other)
{
    Evaluation_Weights<T> difference = (*this) - other;

    for (std::size_t i = 0; i < get_size(); ++i) { (*this)[i] = difference[i]; }

    return *this;
}

template <typename T>
Evaluation_Weights<T>&
Evaluation_Weights<T>::operator/=(const Evaluation_Weights& other)
{
    Evaluation_Weights<T> quotient = (*this) / other;

    for (std::size_t i = 0; i < get_size(); ++i) { (*this)[i] = quotient[i]; }

    return *this;
}

template <typename T>
Evaluation_Weights<T>&
Evaluation_Weights<T>::operator*=(const Evaluation_Weights& other)
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
        result[i] = -m_weight_ref_array[i];
    }

    return result;
}

template <typename T>
Evaluation_Weights<T> Evaluation_Weights<T>::operator+(const T value) const
{
    Evaluation_Weights result;

    for (std::size_t i = 0; i < m_weight_ref_array.size; ++i)
    {
        result[i] = m_weight_ref_array[i] + value;
    }

    return result;
}

template <typename T>
Evaluation_Weights<T> Evaluation_Weights<T>::operator-(const T value) const
{
    Evaluation_Weights result;

    for (std::size_t i = 0; i < m_weight_ref_array.size; ++i)
    {
        result[i] = m_weight_ref_array[i] - value;
    }

    return result;
}

template <typename T>
Evaluation_Weights<T> Evaluation_Weights<T>::operator*(const T value) const
{
    Evaluation_Weights result;

    for (std::size_t i = 0; i < m_weight_ref_array.size; ++i)
    {
        result[i] = m_weight_ref_array[i] * value;
    }

    return result;
}

template <typename T>
Evaluation_Weights<T> Evaluation_Weights<T>::operator/(const T value) const
{
    Evaluation_Weights result;

    for (std::size_t i = 0; i < m_weight_ref_array.size; ++i)
    {
        result[i] = m_weight_ref_array[i] / value;
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
        result[i] = static_cast<T>(std::sqrt(m_weight_ref_array[i]));
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
