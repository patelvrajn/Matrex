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

    Non_Linear_Response(const NLR_Parameters<T>& params);

    FORCE_INLINE T value(const T F) const;

    T calculate_u(const T F) const;
    T calculate_l(const T u) const;
    T calculate_function_M(const T l) const;
    T calculate_function_G(const T F) const;
    T calculate_function_H(const T g) const;
    T calculate_function_S(const T F, const T m) const;
    T calculate_function_P_plus(const T l) const;
    T calculate_function_P_minus(const T l) const;
    T calculate_function_P(const T l, const T g) const;
    T calculate_function_B_plus(const T l) const;
    T calculate_function_B_minus(const T l) const;
    T calculate_function_B(const T l, const T g) const;

  private:

    const NLR_Parameters<T>& m_parameters;
};

template <typename T>
Non_Linear_Response<T>::Non_Linear_Response(const NLR_Parameters<T>& params) :
    m_parameters(params)
{
}

template <typename T>
FORCE_INLINE T Non_Linear_Response<T>::value(const T F) const
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
T Non_Linear_Response<T>::calculate_u(const T F) const
{
    return (F - m_parameters.k);
}

template <typename T>
T Non_Linear_Response<T>::calculate_l(const T u) const
{
    return 0.5 * Matrex::log2((u * u) + NON_LINEAR_RESPONSE_EPSILON);
}

template <typename T>
T Non_Linear_Response<T>::calculate_function_M(const T l) const
{
    const T result = Matrex::exp2(l);
    return result;
}

template <typename T>
T Non_Linear_Response<T>::calculate_function_G(const T F) const
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
T Non_Linear_Response<T>::calculate_function_H(const T g) const
{
    const T first_term  = g * m_parameters.h_plus;
    const T second_term = (-g + 1) * m_parameters.h_minus;
    return (first_term + second_term);
}

template <typename T>
T Non_Linear_Response<T>::calculate_function_S(const T F, const T m) const
{
    const T u           = calculate_u(F);
    const T first_term  = m_parameters.z * u;
    const T second_term = (1 - m_parameters.z) * m;
    return (first_term + second_term);
}

template <typename T>
T Non_Linear_Response<T>::calculate_function_P_plus(const T l) const
{
    const T term = Matrex::exp2(m_parameters.q_plus * l);
    return term;
}

template <typename T>
T Non_Linear_Response<T>::calculate_function_P_minus(const T l) const
{
    const T term = Matrex::exp2(m_parameters.q_minus * l);
    return term;
}

template <typename T>
T Non_Linear_Response<T>::calculate_function_P(const T l, const T g) const
{
    if (g == 1) { return calculate_function_P_plus(l); }
    else if (g == 0) { return calculate_function_P_minus(l); }

    const T first_term  = g * calculate_function_P_plus(l);
    const T second_term = (1 - g) * calculate_function_P_minus(l);
    return (first_term + second_term);
}

template <typename T>
T Non_Linear_Response<T>::calculate_function_B_plus(const T l) const
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
T Non_Linear_Response<T>::calculate_function_B_minus(const T l) const
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
T Non_Linear_Response<T>::calculate_function_B(const T l, const T g) const
{
    if (g == 1) { return calculate_function_B_plus(l); }
    else if (g == 0) { return calculate_function_B_minus(l); }

    const T first_term  = g * calculate_function_B_plus(l);
    const T second_term = (1 - g) * calculate_function_B_minus(l);
    return (first_term + second_term);
}
