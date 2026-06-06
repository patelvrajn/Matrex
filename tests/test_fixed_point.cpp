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

TEST(fixed_point_tests, value_addition)
{
    Fixed_Point_Int_Storage_Type addend_one   = 210;
    Fixed_Point_Int_Storage_Type addend_two   = 512;
    Fixed_Point_Int_Storage_Type expected_sum = addend_one + addend_two;

    Matrex_FP_Int sum = Matrex_FP_Int::from_value(addend_one)
                      + Matrex_FP_Int::from_value(addend_two);

    EXPECT_EQ(sum.get_value(), expected_sum);
}

TEST(fixed_point_tests, double_addition)
{
    double addend_one   = 210.5;
    double addend_two   = 512.5;
    double expected_sum = addend_one + addend_two;

    Matrex_FP_Int sum = Matrex_FP_Int::from_double(addend_one)
                      + Matrex_FP_Int::from_double(addend_two);

    EXPECT_EQ(sum.to_double(), expected_sum);
}

TEST(fixed_point_tests, integer_addition)
{
    Fixed_Point_Int_Storage_Type scale =
        static_cast<Fixed_Point_Int_Storage_Type>(Matrex_FP_Int::scale());

    Fixed_Point_Int_Storage_Type addend_one = 316;
    Fixed_Point_Int_Storage_Type addend_two = 1217;
    Fixed_Point_Int_Storage_Type expected_sum =
        (addend_one * scale) + (addend_two * scale);

    Matrex_FP_Int sum = Matrex_FP_Int::from_integer(addend_one)
                      + Matrex_FP_Int::from_integer(addend_two);

    EXPECT_EQ(sum.get_value(), expected_sum);
}

TEST(fixed_point_tests, mixed_addition)
{
    double                       addend_one = 12.5;
    Fixed_Point_Int_Storage_Type addend_two = 1217;
    Fixed_Point_Int_Storage_Type expected_sum =
        Matrex_FP_Int::from_double(addend_one).get_value() + addend_two;

    Matrex_FP_Int sum_one = addend_one + Matrex_FP_Int::from_value(addend_two);
    Matrex_FP_Int sum_two = Matrex_FP_Int::from_value(addend_two) + addend_one;

    EXPECT_EQ(sum_one.get_value(), sum_two.get_value());
    EXPECT_EQ(sum_one.get_value(), expected_sum);
}

TEST(fixed_point_tests, test_addition_overflow)
{
    Fixed_Point_Int_Storage_Type addend_one = std::numeric_limits<Fixed_Point_Int_Storage_Type>::max();
    Fixed_Point_Int_Storage_Type addend_two = std::numeric_limits<Fixed_Point_Int_Storage_Type>::max();
    Fixed_Point_Int_Storage_Type sum = std::numeric_limits<Fixed_Point_Int_Storage_Type>::max();

    Matrex_FP_Int fp_addend_one = Matrex_FP_Int::from_value(addend_one);
    Matrex_FP_Int fp_addend_two = Matrex_FP_Int::from_value(addend_two);
    Matrex_FP_Int fp_sum        = fp_addend_one + fp_addend_two;

    EXPECT_EQ(fp_sum.get_value(), sum);
}

TEST(fixed_point_tests, value_subtraction)
{
    Fixed_Point_Int_Storage_Type subtrahend              = 1210;
    Fixed_Point_Int_Storage_Type minuend                 = 512;
    Fixed_Point_Int_Storage_Type expected_difference_one = subtrahend - minuend;
    Fixed_Point_Int_Storage_Type expected_difference_two = minuend - subtrahend;

    Matrex_FP_Int difference_one = Matrex_FP_Int::from_value(subtrahend)
                                 - Matrex_FP_Int::from_value(minuend);
    Matrex_FP_Int difference_two = Matrex_FP_Int::from_value(minuend)
                                 - Matrex_FP_Int::from_value(subtrahend);

    EXPECT_EQ(difference_one.get_value(), expected_difference_one);
    EXPECT_EQ(difference_two.get_value(), expected_difference_two);
    EXPECT_NE(difference_one.get_value(), difference_two.get_value());
}

TEST(fixed_point_tests, double_subtraction)
{
    double subtrahend              = 45.5;
    double minuend                 = 32.5;
    double expected_difference_one = subtrahend - minuend;
    double expected_difference_two = minuend - subtrahend;

    Matrex_FP_Int difference_one = Matrex_FP_Int::from_double(subtrahend)
                                 - Matrex_FP_Int::from_double(minuend);
    Matrex_FP_Int difference_two = Matrex_FP_Int::from_double(minuend)
                                 - Matrex_FP_Int::from_double(subtrahend);

    EXPECT_EQ(difference_one.to_double(), expected_difference_one);
    EXPECT_EQ(difference_two.to_double(), expected_difference_two);
    EXPECT_NE(difference_one, difference_two);
}

TEST(fixed_point_tests, integer_subtraction)
{
    Fixed_Point_Int_Storage_Type scale =
        static_cast<Fixed_Point_Int_Storage_Type>(Matrex_FP_Int::scale());

    Fixed_Point_Int_Storage_Type subtrahend = 316;
    Fixed_Point_Int_Storage_Type minuend    = 1217;
    Fixed_Point_Int_Storage_Type expected_difference_one =
        (subtrahend * scale) - (minuend * scale);
    Fixed_Point_Int_Storage_Type expected_difference_two =
        (minuend * scale) - (subtrahend * scale);

    Matrex_FP_Int difference_one = Matrex_FP_Int::from_integer(subtrahend)
                                 - Matrex_FP_Int::from_integer(minuend);
    Matrex_FP_Int difference_two = Matrex_FP_Int::from_integer(minuend)
                                 - Matrex_FP_Int::from_integer(subtrahend);

    EXPECT_EQ(difference_one.get_value(), expected_difference_one);
    EXPECT_EQ(difference_two.get_value(), expected_difference_two);
    EXPECT_NE(difference_one, difference_two);
}

TEST(fixed_point_tests, mixed_subtraction)
{
    double                       subtrahend = 12.5;
    Fixed_Point_Int_Storage_Type minuend    = 1217;
    Fixed_Point_Int_Storage_Type expected_difference_one =
        Matrex_FP_Int::from_double(subtrahend).get_value() - minuend;
    Fixed_Point_Int_Storage_Type expected_difference_two =
        minuend - Matrex_FP_Int::from_double(subtrahend).get_value();

    Matrex_FP_Int difference_one =
        subtrahend - Matrex_FP_Int::from_value(minuend);
    Matrex_FP_Int difference_two =
        Matrex_FP_Int::from_value(minuend) - subtrahend;

    EXPECT_EQ(difference_one.get_value(), expected_difference_one);
    EXPECT_EQ(difference_two.get_value(), expected_difference_two);
    EXPECT_NE(difference_one, difference_two);
}

TEST(fixed_point_tests, test_subtraction_overflow)
{
    Fixed_Point_Int_Storage_Type subtrahend = std::numeric_limits<Fixed_Point_Int_Storage_Type>::min();
    Fixed_Point_Int_Storage_Type minuend = std::numeric_limits<Fixed_Point_Int_Storage_Type>::max();
    Fixed_Point_Int_Storage_Type difference = std::numeric_limits<Fixed_Point_Int_Storage_Type>::min();

    Matrex_FP_Int fp_subtrahend = Matrex_FP_Int::from_value(subtrahend);
    Matrex_FP_Int fp_minuend = Matrex_FP_Int::from_value(minuend);
    Matrex_FP_Int fp_difference = fp_subtrahend - fp_minuend;

    EXPECT_EQ(fp_difference.get_value(), difference);
}

TEST(fixed_point_tests, value_multiplication)
{
    Fixed_Point_Int_Storage_Type multiplicand = 87219;
    Fixed_Point_Int_Storage_Type multiplier   = 45166;
    int64_t                      expected_product =
        (static_cast<int64_t>(multiplicand) * static_cast<int64_t>(multiplier))
        >> Matrex_FP_Int::fractional_bits();

    Matrex_FP_Int product = Matrex_FP_Int::from_value(multiplicand)
                          * Matrex_FP_Int::from_value(multiplier);

    EXPECT_EQ(product.get_value(), expected_product);
}

TEST(fixed_point_tests, double_multiplication)
{
    double multiplicand     = 45.5;
    double multiplier       = 32.5;
    double expected_product = multiplicand * multiplier;

    Matrex_FP_Int product = Matrex_FP_Int::from_double(multiplicand)
                          * Matrex_FP_Int::from_double(multiplier);

    EXPECT_EQ(product.to_double(), expected_product);
}

TEST(fixed_point_tests, integer_multiplication)
{
    int64_t scale = static_cast<int64_t>(Matrex_FP_Int::scale());

    Fixed_Point_Int_Storage_Type multiplicand = 57;
    Fixed_Point_Int_Storage_Type multiplier   = 13;
    int64_t expected_product = ((static_cast<int64_t>(multiplicand) * scale)
                                * (static_cast<int64_t>(multiplier) * scale))
                            >> Matrex_FP_Int::fractional_bits();

    Matrex_FP_Int product = Matrex_FP_Int::from_integer(multiplicand)
                          * Matrex_FP_Int::from_integer(multiplier);

    EXPECT_EQ(product.get_value(), expected_product);
}

TEST(fixed_point_tests, mixed_multiplication)
{
    double                       multiplicand = 12.5;
    Fixed_Point_Int_Storage_Type multiplier   = 33;
    Fixed_Point_Int_Storage_Type expected_product =
        (Matrex_FP_Int::from_double(multiplicand).get_value() * multiplier)
        >> Matrex_FP_Int::fractional_bits();

    Matrex_FP_Int product_one =
        multiplicand * Matrex_FP_Int::from_value(multiplier);
    Matrex_FP_Int product_two =
        Matrex_FP_Int::from_value(multiplier) * multiplicand;

    EXPECT_EQ(product_one.get_value(), product_two.get_value());
    EXPECT_EQ(product_one.get_value(), expected_product);
}

TEST(fixed_point_tests, test_multiplication_overflow)
{

    Fixed_Point_Int_Storage_Type operand_one = -21598506;
    Fixed_Point_Int_Storage_Type operand_two = 15265664;
    int64_t                      product =
        static_cast<int64_t>(operand_one) * static_cast<int64_t>(operand_two);

    Matrex_FP_Int multiplicand = Matrex_FP_Int::from_value(operand_one);
    Matrex_FP_Int multiplier   = Matrex_FP_Int::from_value(operand_two);
    Matrex_FP_Int fp_product   = multiplicand * multiplier;

    // +1 : We need to account for the floor operation.
    // +1 : We need an additional bit for sign.
    uint8_t product_bit_width =
        static_cast<uint8_t>(std::floor(std::log2(std::abs(product))) + 1)
        + (product < 0);

    uint8_t anti_overflow_scaling = product_bit_width
                                  - (FIXED_POINT_BIT_WIDTH - 1)
                                  - MATREX_FP_INT_FRACTIONAL_BITS;
    anti_overflow_scaling =
        std::max(static_cast<uint8_t>(0), anti_overflow_scaling);

    uint8_t total_scaling =
        anti_overflow_scaling + MATREX_FP_INT_FRACTIONAL_BITS;

    // This is the expected result after overflow handling.
    Fixed_Point_Int_Storage_Type expected_product = product >> total_scaling;

    EXPECT_EQ(fp_product.get_value(), expected_product);
}

TEST(fixed_point_tests, value_division)
{
    Fixed_Point_Int_Storage_Type operand_one = 8279;
    Fixed_Point_Int_Storage_Type operand_two = 5144;
    Fixed_Point_Int_Storage_Type expected_quotient_one =
        (operand_one * Matrex_FP_Int::scale()) / operand_two;
    Fixed_Point_Int_Storage_Type expected_quotient_two =
        (operand_two * Matrex_FP_Int::scale()) / operand_one;

    Matrex_FP_Int quotient_one = Matrex_FP_Int::from_value(operand_one)
                               / Matrex_FP_Int::from_value(operand_two);
    Matrex_FP_Int quotient_two = Matrex_FP_Int::from_value(operand_two)
                               / Matrex_FP_Int::from_value(operand_one);

    EXPECT_EQ(quotient_one.get_value(), expected_quotient_one);
    EXPECT_EQ(quotient_two.get_value(), expected_quotient_two);
    EXPECT_NE(quotient_one.get_value(), quotient_two.get_value());
}

TEST(fixed_point_tests, double_division)
{
    double operand_one           = 2325.3376;
    double operand_two           = 0.15666;
    double expected_quotient_one = operand_one / operand_two;
    double expected_quotient_two = operand_two / operand_one;

    Matrex_FP_Int quotient_one = Matrex_FP_Int::from_double(operand_one)
                               / Matrex_FP_Int::from_double(operand_two);
    Matrex_FP_Int quotient_two = Matrex_FP_Int::from_double(operand_two)
                               / Matrex_FP_Int::from_double(operand_one);

    EXPECT_NEAR(quotient_one.to_double(),
                expected_quotient_one,
                1e-4 * expected_quotient_one);
    EXPECT_NEAR(quotient_two.to_double(),
                expected_quotient_two,
                Matrex_FP_Int::precision());
    EXPECT_NE(quotient_one.get_value(), quotient_two.get_value());
}

TEST(fixed_point_tests, integer_division)
{
    Fixed_Point_Int_Storage_Type scale =
        static_cast<Fixed_Point_Int_Storage_Type>(Matrex_FP_Int::scale());
    int64_t scale_64 = static_cast<int64_t>(scale);

    Fixed_Point_Int_Storage_Type operand_one = 23;
    Fixed_Point_Int_Storage_Type operand_two = 76;
    int64_t                      expected_quotient_one =
        (static_cast<int64_t>(operand_one) * scale_64 * scale_64)
        / (static_cast<int64_t>(operand_two) * scale_64);
    int64_t expected_quotient_two =
        (static_cast<int64_t>(operand_two) * scale_64 * scale_64)
        / (static_cast<int64_t>(operand_one) * scale_64);

    Matrex_FP_Int quotient_one = Matrex_FP_Int::from_integer(operand_one)
                               / Matrex_FP_Int::from_integer(operand_two);
    Matrex_FP_Int quotient_two = Matrex_FP_Int::from_integer(operand_two)
                               / Matrex_FP_Int::from_integer(operand_one);

    EXPECT_NEAR(quotient_one.get_value(),
                expected_quotient_one,
                1e-4 * expected_quotient_one);
    EXPECT_NEAR(quotient_two.get_value(),
                expected_quotient_two,
                Matrex_FP_Int::precision());
    EXPECT_NE(quotient_one.get_value(), quotient_two.get_value());
}

TEST(fixed_point_tests, mixed_division)
{
    Fixed_Point_Int_Storage_Type scale =
        static_cast<Fixed_Point_Int_Storage_Type>(Matrex_FP_Int::scale());
    int64_t scale_64 = static_cast<int64_t>(scale);

    double                       operand_one = 12.5;
    Fixed_Point_Int_Storage_Type operand_two = 1217;
    int64_t                      expected_quotient_one =
        (static_cast<int64_t>(operand_two) * scale_64)
        / static_cast<int64_t>(
            Matrex_FP_Int::from_double(operand_one).get_value());
    int64_t expected_quotient_two =
        (static_cast<int64_t>(
             Matrex_FP_Int::from_double(operand_one).get_value())
         * scale_64)
        / operand_two;

    Matrex_FP_Int quotient_one =
        Matrex_FP_Int::from_value(operand_two) / operand_one;
    Matrex_FP_Int quotient_two =
        operand_one / Matrex_FP_Int::from_value(operand_two);

    EXPECT_EQ(quotient_one.get_value(), expected_quotient_one);
    EXPECT_EQ(quotient_two.get_value(), expected_quotient_two);
    EXPECT_NE(quotient_one.get_value(), quotient_two.get_value());
}

TEST(fixed_point_tests, test_division_overflow)
{
    Fixed_Point_Int_Storage_Type operand_one =
        Matrex_FP_Int::from_double(Matrex_FP_Int::maximum()).get_value();
    Fixed_Point_Int_Storage_Type operand_two =
        Matrex_FP_Int::from_double(Matrex_FP_Int::precision()).get_value();
    int64_t numerator   = (static_cast<int64_t>(operand_one)
                         * static_cast<int64_t>(Matrex_FP_Int::scale()));
    int64_t denominator = static_cast<int64_t>(operand_two);
    int64_t quotient    = numerator / denominator;

    Matrex_FP_Int dividend    = Matrex_FP_Int::from_value(operand_one);
    Matrex_FP_Int divisor     = Matrex_FP_Int::from_value(operand_two);
    Matrex_FP_Int fp_quotient = dividend / divisor;

    // +1 : We need to account for the floor operation.
    // +1 : We need an additional bit for sign.
    uint8_t quotient_bit_width =
        static_cast<uint8_t>(std::floor(std::log2(std::abs(quotient))) + 1)
        + (quotient < 0);

    uint8_t anti_overflow_scaling =
        quotient_bit_width - (FIXED_POINT_BIT_WIDTH - 1);
    anti_overflow_scaling =
        std::max(static_cast<uint8_t>(0), anti_overflow_scaling);

    // This is the expected result after overflow handling.
    denominator = (denominator >> anti_overflow_scaling);
    denominator = (denominator == 0) ? operand_two : denominator;
    Fixed_Point_Int_Storage_Type expected_quotient =
        (numerator >> anti_overflow_scaling) / denominator;

    EXPECT_EQ(fp_quotient.get_value(), expected_quotient);
}

TEST(fixed_point_tests, test_comparisons)
{

    Fixed_Point_Int_Storage_Type operand_one = 8279;
    Fixed_Point_Int_Storage_Type operand_two = 5144;

    Matrex_FP_Int fp_operand_one = Matrex_FP_Int::from_value(operand_one);
    Matrex_FP_Int fp_operand_two = Matrex_FP_Int::from_value(operand_two);

    ASSERT_TRUE(fp_operand_one > fp_operand_two);
    ASSERT_TRUE(fp_operand_one != fp_operand_two);

    double        operand_three   = 12.98;
    Matrex_FP_Int fp_operand_four = Matrex_FP_Int::from_double(16.89);

    ASSERT_TRUE(operand_three < fp_operand_four);
    ASSERT_TRUE(operand_three != fp_operand_four);

    Matrex_FP_Int fp_operand_five = Matrex_FP_Int::from_integer(4095);

    ASSERT_TRUE(operand_one > fp_operand_five);
    ASSERT_TRUE(operand_one != fp_operand_five);
}

TEST(fixed_point_tests, tanh_24_8_test)
{
    constexpr uint64_t NUMBER_OF_TRIALS = 50000;
    constexpr uint64_t SEED             = 69;

    constexpr uint8_t F         = 8;
    constexpr double  precision = 1.0 / (1 << F);
    constexpr double  tolerance = 3 * precision;

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
    constexpr double  precision          = 1.0 / (1 << F);
    constexpr double  absolute_tolerance = 1 * precision; // For small values
    constexpr double  relative_tolerance = 0.25;          // For large values

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

TEST(fixed_point_tests, pow_matrex_fp_test)
{
    constexpr uint64_t NUMBER_OF_TRIALS = 50000;
    constexpr uint64_t SEED             = 69;

    constexpr double absolute_tolerance =
        1 * Matrex_FP_Int::precision();         // For small values
    constexpr double relative_tolerance = 0.25; // For large values

    std::mt19937_64 rng(SEED); // Fixed seed for reproducibility.

    std::uniform_real_distribution<double> base_distribution(
        0,
        Matrex_FP_Int::maximum());
    std::uniform_real_distribution<double> exponent_distribution(
        Matrex_FP_Int::minimum(),
        Matrex_FP_Int::maximum());

    for (uint64_t i = 0; i < NUMBER_OF_TRIALS; ++i)
    {
        double base_value     = base_distribution(rng);
        double exponent_value = exponent_distribution(rng);

        if ((!Matrex_FP_Int::is_representable(base_value))
            || (!Matrex_FP_Int::is_representable(exponent_value)))
        {
            continue;
        }

        Matrex_FP_Int base     = Matrex_FP_Int::from_double(base_value);
        Matrex_FP_Int exponent = Matrex_FP_Int::from_double(exponent_value);
        Matrex_FP_Int result   = Matrex::pow(base, exponent);

        double expected = std::pow(base.to_double(), exponent.to_double());

        if (!Matrex_FP_Int::is_representable(expected)) { continue; }

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
    constexpr uint64_t NUMBER_OF_TRIALS = 1000000;
    constexpr uint64_t SEED             = 512;

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

        // Test function S
        double expected_Mu =
            std::sqrt((double_u * double_u) + NON_LINEAR_RESPONSE_EPSILON);
        double S_first_term = nlr_parameters.z.to_double() * double_u;
        double S_second_term =
            ((1.0 - nlr_parameters.z.to_double()) * expected_Mu);
        double expected_S = S_first_term + S_second_term;
        if (Matrex_FP_Int::is_representable(expected_S)
            && Matrex_FP_Int::is_representable(expected_Mu))
        {
            Matrex_FP_Int result = nlr.calculate_function_S(fp_test_value);
            collect_error(errors,
                          double_test_value,
                          expected_S,
                          result.to_double(),
                          "S");
        }

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
