#include <random>

#include "fixed_point.hpp"
#include "gtest/gtest.h"

TEST(fixed_point_tests, tanh_test) {
  constexpr uint64_t NUMBER_OF_TRIALS = 50000;
  constexpr uint64_t SEED = 69;

  constexpr uint8_t F = 8;
  constexpr double scale = 1.0 / (1 << F);
  constexpr double tolerance = 1 * scale;

  std::mt19937_64 rng(SEED);  // Fixed seed for reproducibility.
  std::uniform_real_distribution<double> distribution(-5.8917, 5.8917);

  for (uint64_t i = 0; i < NUMBER_OF_TRIALS; ++i) {
    double input_value = distribution(rng);
    Fixed_Point_Integer<F> input =
        Fixed_Point_Integer<F>::from_double(input_value);
    Fixed_Point_Integer<F> result = Matrex::tanh(input);
    EXPECT_NEAR(result.to_double(), std::tanh(input.to_double()), tolerance);
  }
}

TEST(fixed_point_tests, pow_test) {
  constexpr uint64_t NUMBER_OF_TRIALS = 50000;
  constexpr uint64_t SEED = 69;

  constexpr uint8_t F = 8;
  constexpr double scale = 1.0 / (1 << F);
  constexpr double absolute_tolerance = 1 * scale;  // For small values
  constexpr double relative_tolerance = 0.25;       // For large values

  std::mt19937_64 rng(SEED);  // Fixed seed for reproducibility.
  std::uniform_real_distribution<double> base_distribution(0, 2);
  std::uniform_real_distribution<double> exponent_distribution(-2, 2);

  for (uint64_t i = 0; i < NUMBER_OF_TRIALS; ++i) {
    double base_value = base_distribution(rng);
    double exponent_value = exponent_distribution(rng);
    Fixed_Point_Integer<F> base =
        Fixed_Point_Integer<F>::from_double(base_value);
    Fixed_Point_Integer<F> exponent =
        Fixed_Point_Integer<F>::from_double(exponent_value);
    Fixed_Point_Integer<F> result = Matrex::pow(base, exponent);
    double expected = std::pow(base.to_double(), exponent.to_double());
    double max_tolerance =
        std::max(absolute_tolerance, relative_tolerance * std::abs(expected));
    EXPECT_NEAR(result.to_double(), expected, max_tolerance);
  }
}
