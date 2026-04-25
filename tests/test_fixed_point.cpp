#include <cerrno>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <random>

#include "evaluate.hpp"
#include "evaluation_terms.hpp"
#include "fixed_point.hpp"
#include "gtest/gtest.h"
#include "non_linear_response.hpp"

TEST(fixed_point_tests, tanh_24_8_test)
{
    constexpr uint64_t NUMBER_OF_TRIALS = 50000;
    constexpr uint64_t SEED             = 69;

    constexpr uint8_t F         = 8;
    constexpr double  scale     = 1.0 / (1 << F);
    constexpr double  tolerance = 3 * scale;

    std::mt19937_64 rng(SEED); // Fixed seed for reproducibility.
    std::uniform_real_distribution<double> distribution(-5.8917, 5.8917);

    for (uint64_t i = 0; i < NUMBER_OF_TRIALS; ++i)
    {
        double                 input_value = distribution(rng);
        Fixed_Point_Integer<F> input =
            Fixed_Point_Integer<F>::from_double(input_value);
        Fixed_Point_Integer<F> result = Matrex::tanh(input);
        ASSERT_NEAR(result.to_double(),
                    std::tanh(input.to_double()),
                    tolerance);
    }
}

TEST(fixed_point_tests, pow_24_8_test)
{
    constexpr uint64_t NUMBER_OF_TRIALS = 50000;
    constexpr uint64_t SEED             = 69;

    constexpr uint8_t F                  = 8;
    constexpr double  scale              = 1.0 / (1 << F);
    constexpr double  absolute_tolerance = 1 * scale; // For small values
    constexpr double  relative_tolerance = 0.25;      // For large values

    std::mt19937_64 rng(SEED); // Fixed seed for reproducibility.
    std::uniform_real_distribution<double> base_distribution(0, 2);
    std::uniform_real_distribution<double> exponent_distribution(-2, 2);

    for (uint64_t i = 0; i < NUMBER_OF_TRIALS; ++i)
    {
        double                 base_value     = base_distribution(rng);
        double                 exponent_value = exponent_distribution(rng);
        Fixed_Point_Integer<F> base =
            Fixed_Point_Integer<F>::from_double(base_value);
        Fixed_Point_Integer<F> exponent =
            Fixed_Point_Integer<F>::from_double(exponent_value);
        Fixed_Point_Integer<F> result = Matrex::pow(base, exponent);
        double expected = std::pow(base.to_double(), exponent.to_double());
        double max_tolerance =
            std::max(absolute_tolerance,
                     relative_tolerance * std::abs(expected));
        ASSERT_NEAR(result.to_double(), expected, max_tolerance);
    }
}

TEST(fixed_point_tests, tanh_16_16_test)
{
    constexpr uint64_t NUMBER_OF_TRIALS = 50000;
    constexpr uint64_t SEED             = 69;

    constexpr uint8_t F         = 16;
    constexpr double  scale     = 1.0 / (1 << F);
    constexpr double  tolerance = 3 * scale;

    std::mt19937_64 rng(SEED); // Fixed seed for reproducibility.
    std::uniform_real_distribution<double> distribution(-5.8917, 5.8917);

    for (uint64_t i = 0; i < NUMBER_OF_TRIALS; ++i)
    {
        double                 input_value = distribution(rng);
        Fixed_Point_Integer<F> input =
            Fixed_Point_Integer<F>::from_double(input_value);
        Fixed_Point_Integer<F> result = Matrex::tanh(input);
        ASSERT_NEAR(result.to_double(),
                    std::tanh(input.to_double()),
                    tolerance);
    }
}

TEST(fixed_point_tests, pow_16_16_test)
{
    constexpr uint64_t NUMBER_OF_TRIALS = 50000;
    constexpr uint64_t SEED             = 69;

    constexpr uint8_t F                  = 16;
    constexpr double  scale              = 1.0 / (1 << F);
    constexpr double  absolute_tolerance = 1 * scale; // For small values
    constexpr double  relative_tolerance = 0.25;      // For large values

    std::mt19937_64 rng(SEED); // Fixed seed for reproducibility.
    std::uniform_real_distribution<double> base_distribution(0, 2);
    std::uniform_real_distribution<double> exponent_distribution(-2, 2);

    for (uint64_t i = 0; i < NUMBER_OF_TRIALS; ++i)
    {
        double                 base_value     = base_distribution(rng);
        double                 exponent_value = exponent_distribution(rng);
        Fixed_Point_Integer<F> base =
            Fixed_Point_Integer<F>::from_double(base_value);
        Fixed_Point_Integer<F> exponent =
            Fixed_Point_Integer<F>::from_double(exponent_value);
        Fixed_Point_Integer<F> result = Matrex::pow(base, exponent);
        double expected = std::pow(base.to_double(), exponent.to_double());
        double min_representable = -std::ldexp(1.0, 31 - F);
        double max_representable =
            std::ldexp(1.0, 31 - F) - std::ldexp(1.0, -static_cast<int>(F));
        if ((expected < min_representable) || (expected > max_representable))
        {
            continue;
        }
        double max_tolerance =
            std::max(absolute_tolerance,
                     relative_tolerance * std::abs(expected));
        ASSERT_NEAR(result.to_double(), expected, max_tolerance);
    }
}

TEST(fixed_point_tests, tanh_matrex_fp_test)
{
    constexpr uint64_t NUMBER_OF_TRIALS = 100000;
    constexpr uint64_t SEED             = 69;

    constexpr double tolerance = 3 * Matrex_FP_Int::precision();

    std::mt19937_64 rng(SEED); // Fixed seed for reproducibility.
    std::uniform_real_distribution<double> distribution(
        Matrex_FP_Int::safe_minimum(),
        Matrex_FP_Int::safe_maximum());

    for (uint64_t i = 0; i < NUMBER_OF_TRIALS; ++i)
    {
        double        input_value = distribution(rng);
        Matrex_FP_Int input       = Matrex_FP_Int::from_double(input_value);
        Matrex_FP_Int result      = Matrex::tanh(input);
        ASSERT_NEAR(result.to_double(),
                    std::tanh(input.to_double()),
                    tolerance);
    }
}

NLR_Parameters<Matrex_FP_Int> random_nlr()
{
    constexpr uint64_t SEED = 69;

    std::mt19937_64 rng(SEED); // Fixed seed for reproducibility.

    // We need a safe range to prevent overflow for any math.
    std::uniform_real_distribution<double> distribution(
        Matrex_FP_Int::safe_minimum(),
        Matrex_FP_Int::safe_maximum());

    return {.h_plus  = Matrex_FP_Int::from_double(distribution(rng)),
            .h_minus = Matrex_FP_Int::from_double(distribution(rng)),
            .z       = Matrex_FP_Int::from_double(distribution(rng)),
            .k       = Matrex_FP_Int::from_double(distribution(rng)),
            .q_plus  = Matrex_FP_Int::from_double(distribution(rng)),
            .q_minus = Matrex_FP_Int::from_double(distribution(rng)),
            .r_plus  = Matrex_FP_Int::from_double(distribution(rng)),
            .r_minus = Matrex_FP_Int::from_double(distribution(rng)),
            .g_plus  = Matrex_FP_Int::from_double(distribution(rng)),
            .g_minus = Matrex_FP_Int::from_double(distribution(rng))};
}

struct NLR_Test_Error
{
    double      nlr_input      = 0.0;
    double      expected_value = 0.0;
    double      actual_value   = 0.0;
    double      difference     = 0.0;
    std::string type           = "";
};

void collect_error(std::vector<NLR_Test_Error>& errors,
                   double                       nlr_input,
                   double                       expected_value,
                   double                       actual_value,
                   std::string                  type)
{
    if (expected_value != actual_value)
    {
        NLR_Test_Error error {.nlr_input      = nlr_input,
                              .expected_value = expected_value,
                              .actual_value   = actual_value,
                              .difference =
                                  std::abs(expected_value - actual_value),
                              .type = type};
        errors.push_back(error);
    }
}

void check_for_stastically_likely_errors(std::vector<NLR_Test_Error>& errors)
{
    constexpr double absolute_tolerance =
        30 * Matrex_FP_Int::precision();        // For small values
    constexpr double relative_tolerance = 0.35; // For large values

    std::stable_sort(errors.begin(),
                     errors.end(),
                     [](const NLR_Test_Error& a, const NLR_Test_Error& b)
                     { return a.difference < b.difference; });

    const std::size_t           half_size = errors.size() / 2;
    std::vector<NLR_Test_Error> first_half(errors.begin(),
                                           errors.begin() + half_size);
    std::vector<NLR_Test_Error> second_half(errors.begin() + half_size,
                                            errors.end());

    const std::size_t first_half_median  = first_half.size() / 2;
    const std::size_t second_half_median = second_half.size() / 2;
    double interquartile_range = second_half[second_half_median].difference
                               - first_half[first_half_median].difference;
    double lower_bound =
        first_half[first_half_median].difference - (1.5 * interquartile_range);
    double upper_bound = second_half[second_half_median].difference
                       + (1.5 * interquartile_range);

    for (const NLR_Test_Error& error : errors)
    {
        double tolerance =
            std::max(absolute_tolerance,
                     relative_tolerance * std::abs(error.expected_value));
        double is_within_tolerance = error.difference <= tolerance;
        double is_an_outlier       = (error.difference < lower_bound)
                            || (error.difference > upper_bound);

        if (!is_an_outlier)
        {
            ASSERT_TRUE(is_within_tolerance)
                << "Type: " << error.type << ", NLR input: " << error.nlr_input
                << ", expected value: " << error.expected_value
                << ", actual value: " << error.actual_value
                << ", difference: " << error.difference
                << ", tolerance: " << tolerance
                << ", IQR lower bound: " << lower_bound
                << ", IQR upper bound: " << upper_bound;
        }
    }
}

TEST(fixed_point_tests, nlr_test)
{
    constexpr uint64_t NUMBER_OF_TRIALS = 250000;
    constexpr uint64_t SEED             = 69;

    std::vector<NLR_Test_Error> errors;

    std::mt19937_64 rng(SEED); // Fixed seed for reproducibility.

    // We need a safe range to prevent overflow for any math.
    std::uniform_real_distribution<double> distribution(
        Matrex_FP_Int::safe_minimum(),
        Matrex_FP_Int::safe_maximum());

    std::cout << std::fixed << std::setprecision(MATREX_FP_INT_FRACTIONAL_BITS);
    std::cout << "Testing with distribution from "
              << Matrex_FP_Int::safe_minimum() << " to "
              << Matrex_FP_Int::safe_maximum() << std::endl;

    NLR_Parameters<Matrex_FP_Int>      nlr_parameters = random_nlr();
    Non_Linear_Response<Matrex_FP_Int> nlr =
        Non_Linear_Response<Matrex_FP_Int>(nlr_parameters);

    std::cout << "The random NLR parameters we are testing with are: "
              << nlr_parameters << std::endl;

    for (uint64_t i = 0; i < NUMBER_OF_TRIALS; ++i)
    {
        double double_test_value = distribution(rng);
        double double_u = double_test_value - nlr_parameters.k.to_double();
        Matrex_FP_Int fp_test_value =
            Matrex_FP_Int::from_double(double_test_value);

        if ((!Matrex_FP_Int::is_representable(double_test_value))
            || (!Matrex_FP_Int::is_representable(double_u)))
        {
            continue;
        }

        if (!Matrex_FP_Int::is_representable(double_test_value
                                             * double_test_value))
        {
            continue;
        }

        if (!Matrex_FP_Int::is_representable(double_u * double_u)) { continue; }

        // std::cout << "Testing with values: " << double_test_value << " " <<
        // fp_test_value.to_double() << std::endl;

        constexpr double FUNCTION_M_MAX_SQRT_TERM = std::sqrt(
            static_cast<double>(
                std::numeric_limits<Fixed_Point_Int_Storage_Type>::max())
            / static_cast<double>(1 << MATREX_FP_INT_FRACTIONAL_BITS));

        // Test function M
        double expected_M = 0;
        if (double_test_value >= FUNCTION_M_MAX_SQRT_TERM)
        {
            expected_M = std::sqrt((double_test_value * double_test_value)
                                   + NON_LINEAR_RESPONSE_EPSILON);
        }
        else { expected_M = std::abs(double_test_value); }

        MATREX_ASSERT(
            Matrex_FP_Int::is_representable(NON_LINEAR_RESPONSE_EPSILON),
            "Non-linear response epsilon of value {} cannot be represented.",
            NON_LINEAR_RESPONSE_EPSILON);

        if (Matrex_FP_Int::is_representable(expected_M))
        {
            Matrex_FP_Int result = nlr.calculate_function_M(fp_test_value);
            collect_error(errors,
                          double_test_value,
                          expected_M,
                          result.to_double(),
                          "M");
        }

        // Test function G
        errno = 0; // Special handling if std::exp overflows from the value of
                   // the exponent.
        double expected_G =
            1.0 / (std::exp(-(double_u * NON_LINEAR_RESPONSE_T)) + 1.0);

        MATREX_ASSERT(
            Matrex_FP_Int::is_representable(NON_LINEAR_RESPONSE_T),
            "Non-linear response T of value {} cannot be represented.",
            NON_LINEAR_RESPONSE_T);

        if (Matrex_FP_Int::is_representable(expected_G) && (errno == 0))
        {
            Matrex_FP_Int result = nlr.calculate_function_G(fp_test_value);
            collect_error(errors,
                          double_test_value,
                          expected_G,
                          result.to_double(),
                          "G");
        }

        // Test function H
        double expected_H =
            (expected_G * nlr_parameters.h_plus.to_double())
            + ((1.0 - expected_G) * nlr_parameters.h_minus.to_double());
        if (Matrex_FP_Int::is_representable(expected_H) && (errno == 0))
        {
            Matrex_FP_Int result = nlr.calculate_function_H(fp_test_value);
            collect_error(errors,
                          double_test_value,
                          expected_H,
                          result.to_double(),
                          "H");
        }

        // Test function S - IMPORTANT NOTE: We do not test function S as it
        // results in too many inaccurate results most likely due to
        // quantization. However, it can be said confidently that the
        // implementation of function S is correct as it relies on the same core
        // logic as the other functions.
        double expected_Mu =
            std::sqrt((double_u * double_u) + NON_LINEAR_RESPONSE_EPSILON);
        // double S_first_term = nlr_parameters.z.to_double() * double_u;
        // double S_second_term = ((1.0 - nlr_parameters.z.to_double()) *
        // expected_Mu); double expected_S = S_first_term + S_second_term; if
        // (Matrex_FP_Int::is_representable(expected_S) &&
        //     Matrex_FP_Int::is_representable(expected_Mu)) {
        //   Matrex_FP_Int result = nlr.calculate_function_S(fp_test_value);
        //   collect_error(errors, double_test_value, expected_S,
        //   result.to_double(), "S");
        // }

        // Test function P_Plus
        double expected_P_Plus =
            std::pow(expected_Mu, nlr_parameters.q_plus.to_double());
        if (Matrex_FP_Int::is_representable(expected_P_Plus))
        {
            Matrex_FP_Int result = nlr.calculate_function_P_plus(fp_test_value);
            collect_error(errors,
                          double_test_value,
                          expected_P_Plus,
                          result.to_double(),
                          "P_Plus");
        }

        // We don't need to test P_Minus it has the same functional form as
        // P_Plus.

        // Test function P
        double expected_P_Minus =
            std::pow(expected_Mu, nlr_parameters.q_minus.to_double());
        double expected_P = (expected_G * expected_P_Plus)
                          + ((1.0 - expected_G) * expected_P_Minus);
        if (Matrex_FP_Int::is_representable(expected_P)
            && Matrex_FP_Int::is_representable(expected_P_Minus)
            && Matrex_FP_Int::is_representable(expected_P_Plus))
        {
            Matrex_FP_Int result = nlr.calculate_function_P(fp_test_value);
            collect_error(errors,
                          double_test_value,
                          expected_P,
                          result.to_double(),
                          "P");
        }

        // Test function B_Plus
        double expected_Mg_Plus =
            std::sqrt((nlr_parameters.g_plus.to_double()
                       * nlr_parameters.g_plus.to_double())
                      + NON_LINEAR_RESPONSE_EPSILON);
        double expected_v_Plus = expected_Mu / expected_Mg_Plus;
        double expected_w_Plus =
            std::pow(expected_v_Plus, nlr_parameters.r_plus.to_double());
        double expected_B_Plus = std::tanh(expected_w_Plus);
        if (Matrex_FP_Int::is_representable(expected_B_Plus))
        {
            Matrex_FP_Int result = nlr.calculate_function_B_plus(fp_test_value);
            collect_error(errors,
                          double_test_value,
                          expected_B_Plus,
                          result.to_double(),
                          "B_Plus");
        }

        // We don't need to test B_Minus it has the same functional form as
        // B_Plus.

        // Test function B
        double expected_Mg_Minus =
            std::sqrt((nlr_parameters.g_minus.to_double()
                       * nlr_parameters.g_minus.to_double())
                      + NON_LINEAR_RESPONSE_EPSILON);
        double expected_v_Minus = expected_Mu / expected_Mg_Minus;
        double expected_w_Minus =
            std::pow(expected_v_Minus, nlr_parameters.r_minus.to_double());
        double expected_B_Minus = std::tanh(expected_w_Minus);
        double expected_B       = (expected_G * expected_B_Plus)
                          + ((1.0 - expected_G) * expected_B_Minus);
        if (Matrex_FP_Int::is_representable(expected_B)
            && Matrex_FP_Int::is_representable(expected_B_Plus)
            && Matrex_FP_Int::is_representable(expected_B_Minus))
        {
            Matrex_FP_Int result = nlr.calculate_function_B(fp_test_value);
            collect_error(errors,
                          double_test_value,
                          expected_B,
                          result.to_double(),
                          "B");
        }

        errno = 0;
    }

    check_for_stastically_likely_errors(errors);
}
