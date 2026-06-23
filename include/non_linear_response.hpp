#pragma once

#include "fixed_point.hpp"

template <typename T>
struct NLR_Parameters
{ // NLR = Non-Linear Response
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

    Non_Linear_Response(NLR_Parameters<T> params);

    T value(T F) const;

    T calculate_u(T F) const;

    T calculate_function_M(T x) const;
    T calculate_function_G(T F) const;
    T calculate_function_H(T F) const;
    T calculate_function_S(T F) const;
    T calculate_function_P_plus(T F) const;
    T calculate_function_P_minus(T F) const;
    T calculate_function_P(T F) const;
    T calculate_function_B_plus(T F) const;
    T calculate_function_B_minus(T F) const;
    T calculate_function_B(T F) const;

  private:

    T m_h_plus_parameter;
    T m_h_minus_parameter;
    T m_z_parameter;
    T m_k_parameter;
    T m_q_plus_parameter;
    T m_q_minus_parameter;
    T m_r_plus_parameter;
    T m_r_minus_parameter;
    T m_g_plus_parameter;
    T m_g_minus_parameter;
};

template <typename T>
Non_Linear_Response<T>::Non_Linear_Response(NLR_Parameters<T> params) :
    m_h_plus_parameter(params.h_plus),
    m_h_minus_parameter(params.h_minus),
    m_z_parameter(params.z),
    m_k_parameter(params.k),
    m_q_plus_parameter(params.q_plus),
    m_q_minus_parameter(params.q_minus),
    m_r_plus_parameter(params.r_plus),
    m_r_minus_parameter(params.r_minus),
    m_g_plus_parameter(params.g_plus),
    m_g_minus_parameter(params.g_minus)
{
}

template <typename T>
T Non_Linear_Response<T>::value(T F) const
{
    const T H = calculate_function_H(F);
    const T S = calculate_function_S(F);
    const T P = calculate_function_P(F);
    const T B = calculate_function_B(F);

    return (H * S * P * B);
}

template <typename T>
T Non_Linear_Response<T>::calculate_u(T F) const
{
    return (F - m_k_parameter);
}

template <typename T>
T Non_Linear_Response<T>::calculate_function_M(T x) const
{
    if constexpr (std::is_same_v<T, Matrex_FP_Int>)
    {
        constexpr double MAX_SQRT_TERM = std::sqrt(
            static_cast<double>(
                std::numeric_limits<Fixed_Point_Int_Storage_Type>::max())
            / static_cast<double>(1 << MATREX_FP_INT_FRACTIONAL_BITS));
        MATREX_ASSERT(Matrex_FP_Int::is_representable(MAX_SQRT_TERM),
                      "Maximum square root term of value {} in function M is "
                      "out of bounds.",
                      MAX_SQRT_TERM);
        constexpr Matrex_FP_Int FP_MAX_SQRT_TERM =
            Matrex_FP_Int::from_double(MAX_SQRT_TERM);

        const T abs_x = ((x < 0) ? -x : x);

        // This function is used calculate the absolute value of x in a manner
        // where it is differentiable and the return value value is not zero (to
        // support negative exponents). If x is large enough that it will result
        // in overflow we just return the absolute value as long as it is
        // non-zero. It is guaranteed to be non-zero otherwise, it wouldn't be
        // greater than the maximum square root term.
        if (abs_x >= FP_MAX_SQRT_TERM) { return abs_x; }
    }

    const T result = Matrex::sqrt((x * x) + NON_LINEAR_RESPONSE_EPSILON);
    return result;
}

template <typename T>
T Non_Linear_Response<T>::calculate_function_G(T F) const
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
            return AD_Value::constant(u.tape.value(), 0.0);
        else
            return explicit_fp_double_conversion<T>(0.0);
    }
    else if (negative_u <= -G_EXPONENT_CLAMP)
    {
        if constexpr (std::is_same_v<T, AD_Value>)
            return AD_Value::constant(u.tape.value(), 1.0);
        else
            return explicit_fp_double_conversion<T>(1.0);
    }

    const T exponent = (negative_u * NON_LINEAR_RESPONSE_T);
    const T sigmoid  = 1.0 / (Matrex::exp(exponent) + 1.0);
    return sigmoid;
}

template <typename T>
T Non_Linear_Response<T>::calculate_function_H(T F) const
{
    const T g           = calculate_function_G(F);
    const T first_term  = g * m_h_plus_parameter;
    const T second_term = (-g + 1) * m_h_minus_parameter;
    return (first_term + second_term);
}

template <typename T>
T Non_Linear_Response<T>::calculate_function_S(T F) const
{
    const T u           = calculate_u(F);
    const T first_term  = m_z_parameter * u;
    const T second_term = (1 - m_z_parameter) * calculate_function_M(u);
    return (first_term + second_term);
}

template <typename T>
T Non_Linear_Response<T>::calculate_function_P_plus(T F) const
{
    const T u    = calculate_u(F);
    const T term = Matrex::pow(calculate_function_M(u), m_q_plus_parameter);
    return term;
}

template <typename T>
T Non_Linear_Response<T>::calculate_function_P_minus(T F) const
{
    const T u    = calculate_u(F);
    const T term = Matrex::pow(calculate_function_M(u), m_q_minus_parameter);
    return term;
}

template <typename T>
T Non_Linear_Response<T>::calculate_function_P(T F) const
{
    const T first_term = calculate_function_G(F) * calculate_function_P_plus(F);
    const T second_term =
        (1 - calculate_function_G(F)) * calculate_function_P_minus(F);
    return (first_term + second_term);
}

template <typename T>
T Non_Linear_Response<T>::calculate_function_B_plus(T F) const
{
    const T u = calculate_u(F);
    const T v =
        calculate_function_M(u) / calculate_function_M(m_g_plus_parameter);
    const T w = Matrex::pow(v, m_r_plus_parameter);
    return Matrex::tanh(w);
}

template <typename T>
T Non_Linear_Response<T>::calculate_function_B_minus(T F) const
{
    const T u = calculate_u(F);
    const T v =
        calculate_function_M(u) / calculate_function_M(m_g_minus_parameter);
    const T w = Matrex::pow(v, m_r_minus_parameter);
    return Matrex::tanh(w);
}

template <typename T>
T Non_Linear_Response<T>::calculate_function_B(T F) const
{
    const T first_term = calculate_function_G(F) * calculate_function_B_plus(F);
    const T second_term =
        (1 - calculate_function_G(F)) * calculate_function_B_minus(F);
    return (first_term + second_term);
}
