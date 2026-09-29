#pragma once

#include "fixed_point.hpp"

#include <unordered_map>
#include <vector>

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
constexpr double NON_LINEAR_RESPONSE_T       = 1.0;

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
        15;
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
        - NON_LINEAR_RESPONSE_TABLE_FP_PRECISION;
    constexpr static double NON_LINEAR_RESPONSE_TABLE_FP_MIN =
        -std::exp2(NON_LINEAR_RESPONSE_TABLE_INTEGER_BIT_WIDTH);

    using Table_Type =
        Multi_Array<Matrex_FP_Int, NON_LINEAR_RESPONSE_TABLE_SIZE>;

    // Target absolute error in evaluation units, checked at interior probes.
    // Refinement stops at adjacent representable inputs, so all sampling
    // happens during construction. This is not a formal bound for every
    // unsampled input of an arbitrary nonlinear curve.
    constexpr static double INTERPOLATION_ERROR_TOLERANCE = 0.01;

    Non_Linear_Response_Table() :
        m_parameters {}, m_table(std::make_unique<Table_Type>())
    {
    }

    Non_Linear_Response_Table(
        const NLR_Parameters<Matrex_FP_Int>& params) :
        m_parameters(params), m_table(std::make_unique<Table_Type>())
    {
        const NLR_Parameters<double> double_parameters {
            .h_plus  = params.h_plus.to_double(),
            .h_minus = params.h_minus.to_double(),
            .z       = params.z.to_double(),
            .k       = params.k.to_double(),
            .q_plus  = params.q_plus.to_double(),
            .q_minus = params.q_minus.to_double(),
            .r_plus  = params.r_plus.to_double(),
            .r_minus = params.r_minus.to_double(),
            .g_plus  = params.g_plus.to_double(),
            .g_minus = params.g_minus.to_double()};

        double value = NON_LINEAR_RESPONSE_TABLE_FP_MIN;

        for (std::size_t i = 0; i < NON_LINEAR_RESPONSE_TABLE_SIZE; ++i)
        {
            (*m_table)[i]  = sample(value, double_parameters);
            value         += NON_LINEAR_RESPONSE_TABLE_FP_PRECISION;
        }

        constexpr int64_t step = static_cast<int64_t>(
            NON_LINEAR_RESPONSE_TABLE_FP_PRECISION * Matrex_FP_Int::scale());
        const int64_t first = Matrex_FP_Int::from_double(
            NON_LINEAR_RESPONSE_TABLE_FP_MIN).get_value();
        for (std::size_t i = 0; i + 1 < NON_LINEAR_RESPONSE_TABLE_SIZE; ++i)
        {
            const int64_t x = first + static_cast<int64_t>(i) * step;
            const Sample left {x, (*m_table)[i].get_value()};
            const Sample right {x + step, (*m_table)[i + 1].get_value()};
            if (!accurate(left, right, double_parameters))
            {
                auto& samples = m_refinements[i];
                refine(left, right, samples, double_parameters);
                samples.push_back(right);
            }
        }
    }

    Matrex_FP_Int lookup(const Matrex_FP_Int value) const
    {
        constexpr uint8_t index_shift =
            FIXED_POINT_BIT_WIDTH - NON_LINEAR_RESPONSE_TABLE_BIT_WIDTH;
        constexpr int64_t minimum_value =
            std::numeric_limits<Fixed_Point_Int_Storage_Type>::min();
        constexpr int64_t maximum_value = minimum_value
            + ((static_cast<int64_t>(NON_LINEAR_RESPONSE_TABLE_SIZE) - 1)
               << index_shift);
        const int64_t x = value.get_value();
        if (x <= minimum_value) { return (*m_table)[0]; }
        if (x >= maximum_value)
        {
            return (*m_table)[NON_LINEAR_RESPONSE_TABLE_SIZE - 1];
        }

        const uint64_t biased_value = static_cast<uint64_t>(
            static_cast<int64_t>(value.get_value()) - minimum_value);
        const std::size_t index = biased_value >> index_shift;

        if (index >= (NON_LINEAR_RESPONSE_TABLE_SIZE - 1))
        {
            return (*m_table)[NON_LINEAR_RESPONSE_TABLE_SIZE - 1];
        }

        if (const auto it = m_refinements.find(index); it != m_refinements.end())
        {
            const auto& samples = it->second;
            const auto right = std::upper_bound(
                samples.begin(), samples.end(), x,
                [](int64_t input, const Sample& point) { return input < point.x; });
            const auto& left = *(right - 1);
            return interpolate(left, *right, x);
        }

        const int64_t left_x =
            minimum_value + (static_cast<int64_t>(index) << index_shift);
        return interpolate({left_x, (*m_table)[index].get_value()},
                           {left_x + (int64_t {1} << index_shift),
                            (*m_table)[index + 1].get_value()}, x);
    }

  private:

    struct Sample
    {
        int64_t x;
        int64_t y;
    };

    static Matrex_FP_Int sample(
        double x, const NLR_Parameters<double>& double_parameters)
    {
        const double y = Non_Linear_Response(double_parameters).value(x);
        // Clamp before conversion so llround cannot overflow on steep tails.
        return Matrex_FP_Int::from_double(std::clamp(
            y, Matrex_FP_Int::minimum(), Matrex_FP_Int::maximum()));
    }

    static Matrex_FP_Int interpolate(const Sample& left, const Sample& right,
                                    int64_t x)
    {
        // Wide intermediates preserve the scale even across steep intervals.
        return Matrex_FP_Int::from_value(static_cast<Fixed_Point_Int_Storage_Type>(
            left.y + (right.y - left.y) * (x - left.x) / (right.x - left.x)));
    }

    static bool accurate(const Sample& left, const Sample& right,
                         const NLR_Parameters<double>& double_parameters)
    {
        const auto acceptable = [&](int64_t x)
        {
            if (x <= left.x || x >= right.x) { return true; }
            return std::abs(sample(x * Matrex_FP_Int::precision(),
                                   double_parameters).to_double()
                            - interpolate(left, right, x).to_double())
                   <= INTERPOLATION_ERROR_TOLERANCE / 2;
        };
        for (int fraction = 1; fraction <= 3; ++fraction)
        {
            if (!acceptable(left.x + (right.x - left.x) * fraction / 4))
            { return false; }
        }
        // Uniform probes can miss a narrow spike: also check its known center
        // and the shoulders set by the smoothing epsilon.
        const double k = double_parameters.k;
        for (double x : {k, k - std::sqrt(NON_LINEAR_RESPONSE_EPSILON),
                         k + std::sqrt(NON_LINEAR_RESPONSE_EPSILON)})
        {
            if (!acceptable(Matrex_FP_Int::from_double(x).get_value()))
            { return false; }
        }
        return true;
    }

    static void refine(const Sample& left, const Sample& right,
                       std::vector<Sample>& samples,
                       const NLR_Parameters<double>& double_parameters)
    {
        if (right.x - left.x <= 1 || accurate(left, right, double_parameters))
        {
            samples.push_back(left);
            return;
        }
        const int64_t x = left.x + (right.x - left.x) / 2;
        const Sample middle {
            x, sample(x * Matrex_FP_Int::precision(), double_parameters).get_value()};
        refine(left, middle, samples, double_parameters);
        refine(middle, right, samples, double_parameters);
    }

    NLR_Parameters<Matrex_FP_Int> m_parameters;
    std::unordered_map<std::size_t, std::vector<Sample>> m_refinements;
    std::unique_ptr<Table_Type> m_table;
};
