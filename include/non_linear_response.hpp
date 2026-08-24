#pragma once

#include "fixed_point.hpp"

template <typename T>
struct NLR_Parameters // NLR = Non-Linear Response
{
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

    friend std::ostream& operator<<(std::ostream& os, const NLR_Parameters& nlr)
    {
        os << "{h_plus = " << nlr.h_plus << ", " << "h_minus = " << nlr.h_minus
           << ", " << "z = " << nlr.z << ", " << "k = " << nlr.k << ", "
           << "q_plus = " << nlr.q_plus << ", " << "q_minus = " << nlr.q_minus
           << ", " << "r_plus = " << nlr.r_plus << ", "
           << "r_minus = " << nlr.r_minus << ", " << "g_plus = " << nlr.g_plus
           << ", " << "g_minus = " << nlr.g_minus << "}";
        return os;
    }
};

#define NLR_ARRAY_FIELDS(arr, idx)                                             \
    (arr)[(idx)].h_plus, (arr)[(idx)].h_minus, (arr)[(idx)].z, (arr)[(idx)].k, \
        (arr)[(idx)].q_plus, (arr)[(idx)].q_minus, (arr)[(idx)].r_plus,        \
        (arr)[(idx)].r_minus, (arr)[(idx)].g_plus, (arr)[(idx)].g_minus

#define NLR_2D_ARRAY_FIELDS(arr, outer_idx, inner_idx)                         \
    (arr)[(outer_idx)][(inner_idx)].h_plus,                                    \
        (arr)[(outer_idx)][(inner_idx)].h_minus,                               \
        (arr)[(outer_idx)][(inner_idx)].z, (arr)[(outer_idx)][(inner_idx)].k,  \
        (arr)[(outer_idx)][(inner_idx)].q_plus,                                \
        (arr)[(outer_idx)][(inner_idx)].q_minus,                               \
        (arr)[(outer_idx)][(inner_idx)].r_plus,                                \
        (arr)[(outer_idx)][(inner_idx)].r_minus,                               \
        (arr)[(outer_idx)][(inner_idx)].g_plus,                                \
        (arr)[(outer_idx)][(inner_idx)].g_minus

#define NLR_FIELDS(x)                                                          \
    (x).h_plus, (x).h_minus, (x).z, (x).k, (x).q_plus, (x).q_minus,            \
        (x).r_plus, (x).r_minus, (x).g_plus, (x).g_minus

constexpr double NON_LINEAR_RESPONSE_EPSILON = Matrex_FP_Int::precision();
constexpr double NON_LINEAR_RESPONSE_T       = Matrex_FP_Int::safe_maximum();

template <typename T>
class Non_Linear_Response
{
  public:

    constexpr Non_Linear_Response(const NLR_Parameters<T>& params);

    FORCE_INLINE constexpr T value(const T F) const;

    constexpr T calculate_u(const T F) const;
    constexpr T calculate_l(const T u) const;
    constexpr T calculate_function_M(const T l) const;
    constexpr T calculate_function_G(const T F) const;
    constexpr T calculate_function_H(const T g) const;
    constexpr T calculate_function_S(const T F, const T m) const;
    constexpr T calculate_function_P_plus(const T l) const;
    constexpr T calculate_function_P_minus(const T l) const;
    constexpr T calculate_function_P(const T l, const T g) const;
    constexpr T calculate_function_B_plus(const T l) const;
    constexpr T calculate_function_B_minus(const T l) const;
    constexpr T calculate_function_B(const T l, const T g) const;

  private:

    const NLR_Parameters<T>& m_parameters;
};

template <typename T>
constexpr Non_Linear_Response<T>::Non_Linear_Response(
    const NLR_Parameters<T>& params) :
    m_parameters(params)
{
}

template <typename T>
FORCE_INLINE constexpr T Non_Linear_Response<T>::value(const T F) const
{
    const T u = calculate_u(F);
    const T l = calculate_l(u);

    const T m = calculate_function_M(l);
    const T g = calculate_function_G(F);

    const T H = calculate_function_H(g);
    const T S = calculate_function_S(F, m);
    const T P = calculate_function_P(l, g);
    const T B = calculate_function_B(l, g);

    return (H * S * P * B);
}

template <typename T>
constexpr T Non_Linear_Response<T>::calculate_u(const T F) const
{
    return (F - m_parameters.k);
}

template <typename T>
constexpr T Non_Linear_Response<T>::calculate_l(const T u) const
{
    return 0.5 * Matrex::log2((u * u) + NON_LINEAR_RESPONSE_EPSILON);
}

template <typename T>
constexpr T Non_Linear_Response<T>::calculate_function_M(const T l) const
{
    const T result = Matrex::exp2(l);
    return result;
}

template <typename T>
constexpr T Non_Linear_Response<T>::calculate_function_G(const T F) const
{
    const T u = calculate_u(F);

    // A conversative clamp such that the shifts used in calculating exp2()
    // doesn't produce undefined behavior. This does not result in changing the
    // partials since the exponent clamp is large enough that it's in the
    // saturating region of sigmoid - where the derivatives are close to zero.
    constexpr double G_EXPONENT_CLAMP =
        15.0 / static_cast<double>(NON_LINEAR_RESPONSE_T);

    const T negative_u = -u;

    // -u > (positive clamp) means u is negative thus the denominator of
    // function G gets large and goes to zero.
    if (negative_u >= G_EXPONENT_CLAMP)
    {
        if constexpr (std::is_same_v<T, AD_Value>)
        {
            return AD_Value::constant(u.tape, 0.0);
        }
        else
        {
            return explicit_fp_double_conversion<T>(0.0);
        }
    }
    else if (negative_u <= -G_EXPONENT_CLAMP)
    {
        if constexpr (std::is_same_v<T, AD_Value>)
        {
            return AD_Value::constant(u.tape, 1.0);
        }
        else
        {
            return explicit_fp_double_conversion<T>(1.0);
        }
    }

    const T exponent = (negative_u * NON_LINEAR_RESPONSE_T) / LN_2;
    const T sigmoid  = 1.0 / (Matrex::exp2(exponent) + 1.0);
    return sigmoid;
}

template <typename T>
constexpr T Non_Linear_Response<T>::calculate_function_H(const T g) const
{
    const T first_term  = g * m_parameters.h_plus;
    const T second_term = (-g + 1) * m_parameters.h_minus;
    return (first_term + second_term);
}

template <typename T>
constexpr T Non_Linear_Response<T>::calculate_function_S(const T F,
                                                         const T m) const
{
    const T u           = calculate_u(F);
    const T first_term  = m_parameters.z * u;
    const T second_term = (1 - m_parameters.z) * m;
    return (first_term + second_term);
}

template <typename T>
constexpr T Non_Linear_Response<T>::calculate_function_P_plus(const T l) const
{
    const T term = Matrex::exp2(m_parameters.q_plus * l);
    return term;
}

template <typename T>
constexpr T Non_Linear_Response<T>::calculate_function_P_minus(const T l) const
{
    const T term = Matrex::exp2(m_parameters.q_minus * l);
    return term;
}

template <typename T>
constexpr T Non_Linear_Response<T>::calculate_function_P(const T l,
                                                         const T g) const
{
    if (g == 1) { return calculate_function_P_plus(l); }
    else if (g == 0) { return calculate_function_P_minus(l); }

    const T first_term  = g * calculate_function_P_plus(l);
    const T second_term = (1 - g) * calculate_function_P_minus(l);
    return (first_term + second_term);
}

template <typename T>
constexpr T Non_Linear_Response<T>::calculate_function_B_plus(const T l) const
{
    const T d = 0.5
              * Matrex::log2((m_parameters.g_plus * m_parameters.g_plus)
                             + NON_LINEAR_RESPONSE_EPSILON);

    const T w = Matrex::exp2(m_parameters.r_plus * (l - d));

    const T common_term = Matrex::exp2((-2 * w) / LN_2);

    const T numerator   = 1 - common_term;
    const T denominator = 1 + common_term;

    return (numerator / denominator);
}

template <typename T>
constexpr T Non_Linear_Response<T>::calculate_function_B_minus(const T l) const
{
    const T d = 0.5
              * Matrex::log2((m_parameters.g_minus * m_parameters.g_minus)
                             + NON_LINEAR_RESPONSE_EPSILON);

    const T w = Matrex::exp2(m_parameters.r_minus * (l - d));

    const T common_term = Matrex::exp2((-2 * w) / LN_2);

    const T numerator   = 1 - common_term;
    const T denominator = 1 + common_term;

    return (numerator / denominator);
}

template <typename T>
constexpr T Non_Linear_Response<T>::calculate_function_B(const T l,
                                                         const T g) const
{
    if (g == 1) { return calculate_function_B_plus(l); }
    else if (g == 0) { return calculate_function_B_minus(l); }

    const T first_term  = g * calculate_function_B_plus(l);
    const T second_term = (1 - g) * calculate_function_B_minus(l);
    return (first_term + second_term);
}

class Non_Linear_Response_Table // Only for Matrex fixed-point type.
{
  public:

    constexpr static std::size_t NON_LINEAR_RESPONSE_TABLE_INTEGER_BIT_WIDTH =
        16;
    constexpr static std::size_t
        NON_LINEAR_RESPONSE_TABLE_FRACTIONAL_BIT_WIDTH = 4;
    constexpr static std::size_t NON_LINEAR_RESPONSE_TABLE_BIT_WIDTH =
        NON_LINEAR_RESPONSE_TABLE_INTEGER_BIT_WIDTH
        + NON_LINEAR_RESPONSE_TABLE_FRACTIONAL_BIT_WIDTH + 1;
    constexpr static std::size_t NON_LINEAR_RESPONSE_TABLE_SIZE =
        std::exp2(NON_LINEAR_RESPONSE_TABLE_BIT_WIDTH);

    constexpr static double NON_LINEAR_RESPONSE_TABLE_FP_SCALE =
        Fixed_Point_Integer<
            NON_LINEAR_RESPONSE_TABLE_FRACTIONAL_BIT_WIDTH>::scale();
    constexpr static double NON_LINEAR_RESPONSE_TABLE_FP_PRECISION =
        Fixed_Point_Integer<
            NON_LINEAR_RESPONSE_TABLE_FRACTIONAL_BIT_WIDTH>::precision();

    constexpr static double NON_LINEAR_RESPONSE_TABLE_FP_MAX =
        std::exp2(NON_LINEAR_RESPONSE_TABLE_INTEGER_BIT_WIDTH)
        + Fixed_Point_Integer<NON_LINEAR_RESPONSE_TABLE_FRACTIONAL_BIT_WIDTH>::
            maximum_fractional();
    constexpr static double NON_LINEAR_RESPONSE_TABLE_FP_MIN =
        -1 * NON_LINEAR_RESPONSE_TABLE_FP_MAX;

    using Table_Type =
        Multi_Array<Matrex_FP_Int, NON_LINEAR_RESPONSE_TABLE_SIZE>;

    constexpr Non_Linear_Response_Table() :
        m_parameters {}, m_table(std::make_unique<Table_Type>())
    {
    }

    constexpr Non_Linear_Response_Table(
        const NLR_Parameters<Matrex_FP_Int>& params) :
        m_parameters(params), m_table(std::make_unique<Table_Type>())
    {
        Matrex_FP_Int value =
            Matrex_FP_Int::from_double(NON_LINEAR_RESPONSE_TABLE_FP_MIN);

        for (std::size_t i = 0; i < NON_LINEAR_RESPONSE_TABLE_SIZE; ++i)
        {
            Non_Linear_Response<Matrex_FP_Int> nlr(params);
            (*m_table)[i]  = nlr.value(value);
            value         += NON_LINEAR_RESPONSE_TABLE_FP_PRECISION;
        }
    }

    constexpr Matrex_FP_Int lookup(const Matrex_FP_Int value) const
    {
        if (value <= NON_LINEAR_RESPONSE_TABLE_FP_MIN) { return (*m_table)[0]; }

        if (value >= NON_LINEAR_RESPONSE_TABLE_FP_MAX)
        {
            return (*m_table)[NON_LINEAR_RESPONSE_TABLE_SIZE - 1];
        }

        std::size_t index =
            (value - NON_LINEAR_RESPONSE_TABLE_FP_MIN).get_value()
            >> (FIXED_POINT_BIT_WIDTH - NON_LINEAR_RESPONSE_TABLE_BIT_WIDTH);

        if (index >= (NON_LINEAR_RESPONSE_TABLE_SIZE - 1))
        {
            return (*m_table)[NON_LINEAR_RESPONSE_TABLE_SIZE - 1];
        }

        // The bottom bits of the fraction tell us where in between the indices
        // we are.
        const Matrex_FP_Int fraction = Matrex_FP_Int::from_value(extract_bits(
            value.get_value(),
            0,
            (FIXED_POINT_BIT_WIDTH - NON_LINEAR_RESPONSE_TABLE_BIT_WIDTH - 1)));

        const Matrex_FP_Int y1 = (*m_table)[index];
        const Matrex_FP_Int y2 = (*m_table)[index + 1];

        // Linear interpolation.
        const Matrex_FP_Int result =
            y1
            + (((y2 - y1) * fraction) / NON_LINEAR_RESPONSE_TABLE_FP_PRECISION);

        return result;
    }

  private:

    NLR_Parameters<Matrex_FP_Int> m_parameters;

    std::unique_ptr<Table_Type> m_table;
};
