#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <numbers>
#include <stdexcept>

constexpr uint8_t FIXED_POINT_BIT_WIDTH =
    static_cast<uint8_t>(sizeof(int32_t) * 8);

constexpr double LN_2 = 0.69314718056;  // Precomputed value of ln(2).
constexpr double NATURAL_E = std::numbers::e;

constexpr std::size_t LOG2_LOOKUP_TABLE_SIZE = 4096;
using log2_lookup_table_type = std::array<int32_t, LOG2_LOOKUP_TABLE_SIZE>;

constexpr std::size_t EXP2_LOOKUP_TABLE_SIZE = 4096;
using exp2_lookup_table_type = std::array<int32_t, EXP2_LOOKUP_TABLE_SIZE>;

template <uint8_t F>  // F is the number of fractional bits.
class Fixed_Point_Integer {
 public:
  constexpr Fixed_Point_Integer() : m_value(0) {}
  explicit constexpr Fixed_Point_Integer(int32_t value) : m_value(value) {}

  constexpr int32_t get_value() const;
  constexpr double to_double() const;

  constexpr Fixed_Point_Integer floor() const;

  // Arithmetic operators with another fixed-point integer.
  constexpr Fixed_Point_Integer operator+(
      const Fixed_Point_Integer other) const;
  constexpr Fixed_Point_Integer operator-(
      const Fixed_Point_Integer other) const;
  constexpr Fixed_Point_Integer operator/(
      const Fixed_Point_Integer other) const;
  constexpr Fixed_Point_Integer operator*(
      const Fixed_Point_Integer other) const;

  // Arithmetic operators with a non-fixed-point integer.
  constexpr Fixed_Point_Integer operator+(const int32_t other) const;
  constexpr Fixed_Point_Integer operator-(const int32_t other) const;
  constexpr Fixed_Point_Integer operator/(const int32_t other) const;
  constexpr Fixed_Point_Integer operator*(const int32_t other) const;

  // Arithmetic operators with a double.
  constexpr Fixed_Point_Integer operator+(const double other) const;
  constexpr Fixed_Point_Integer operator-(const double other) const;
  constexpr Fixed_Point_Integer operator/(const double other) const;
  constexpr Fixed_Point_Integer operator*(const double other) const;

  constexpr auto operator<=>(const Fixed_Point_Integer other) const;
  constexpr auto operator<=>(const int32_t other) const;
  constexpr auto operator<=>(const double other) const;

  constexpr bool operator==(const Fixed_Point_Integer& other) const;
  constexpr bool operator==(const int32_t other) const;
  constexpr bool operator==(const double other) const;

  template <typename W>
  friend std::ostream& operator<<(std::ostream& os,
                                  const Fixed_Point_Integer& weights);

  // Value is already an integer in fixed-point representation.
  static constexpr Fixed_Point_Integer from_value(int32_t value) {
    Fixed_Point_Integer result;
    result.m_value = value;
    return result;
  }

  static constexpr Fixed_Point_Integer from_integer(int32_t integer) {
    // Scale up by 2^F to convert to fixed-point representation, before doing
    // so clamp the integer to prevent overflow.
    int32_t minimum_integer = -(1 << (FIXED_POINT_BIT_WIDTH - F - 1));
    int32_t maximum_integer = (1 << (FIXED_POINT_BIT_WIDTH - F - 1)) - 1;
    integer = std::clamp(integer, minimum_integer, maximum_integer);
    int32_t value = integer << F;
    return Fixed_Point_Integer::from_value(value);
  }

  static constexpr Fixed_Point_Integer from_double(double real) {
    double rounded = std::llround(real * (1LL << F));
    rounded = std::clamp(
        rounded, static_cast<double>(std::numeric_limits<int32_t>::min()),
        static_cast<double>(std::numeric_limits<int32_t>::max()));
    int32_t value = static_cast<int32_t>(rounded);
    return Fixed_Point_Integer::from_value(value);
  }

  static constexpr int32_t lookup_log2_table(std::size_t index) {
    return m_log2_table[index];
  }

  static constexpr int32_t lookup_exp2_table(std::size_t index) {
    return m_exp2_table[index];
  }

 private:
  int32_t m_value;

  static constexpr log2_lookup_table_type make_log2_table();
  static constexpr exp2_lookup_table_type make_exp2_table();

  static constexpr log2_lookup_table_type m_log2_table = make_log2_table();
  static constexpr exp2_lookup_table_type m_exp2_table = make_exp2_table();
};

// Fixed Pointer Integer Type that Matrex Uses.
using Matrex_FP_Int = Fixed_Point_Integer<16>;

template <uint8_t F>
constexpr int32_t Fixed_Point_Integer<F>::get_value() const {
  return m_value;
}

template <uint8_t F>
constexpr double Fixed_Point_Integer<F>::to_double() const {
  return static_cast<double>(m_value) / (1LL << F);
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F> Fixed_Point_Integer<F>::floor() const {
  int32_t integer_part = m_value & (~((1LL << F) - 1));
  return Fixed_Point_Integer::from_value(integer_part);
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F> Fixed_Point_Integer<F>::operator+(
    const Fixed_Point_Integer other) const {
  return Fixed_Point_Integer::from_value(m_value + other.m_value);
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F> Fixed_Point_Integer<F>::operator-(
    const Fixed_Point_Integer other) const {
  return Fixed_Point_Integer::from_value(m_value - other.m_value);
}

// Helper function to count leading zeros in a 32-bit signed integer based on
// our use case.
constexpr uint8_t bit_width(int32_t value) {
  if (value == 0) {
    // All bits are zero, so no bit width.
    return 0;
  } else if (value < 0) {
    // We want to count leading zeros simply as an extremely fast way to do
    // (floor(log2(value)) + 1) which is the number of bits needed to represent
    // the value in binary. Thus, we can count leading zeros on the absolute
    // value of the number.
    // Value needs to be promoted to int64_t because -INT_MIN is not
    // representable in int32_t.
    return static_cast<uint8_t>(
        FIXED_POINT_BIT_WIDTH -
        __builtin_clz(static_cast<uint32_t>(-static_cast<int64_t>(value))));
  } else {  // Positive value.
    return static_cast<uint8_t>(FIXED_POINT_BIT_WIDTH -
                                __builtin_clz(static_cast<uint32_t>(value)));
  }
}

constexpr uint8_t bit_width(int64_t value) {
  constexpr uint8_t VALUE_BIT_WIDTH = static_cast<uint8_t>(sizeof(int64_t) * 8);
  if (value == 0) {
    return 0;
  } else if (value < 0) {
    // Note: Value is not casted up to __int128 as it is expected to be within
    // the range of int64_t.
    return static_cast<uint8_t>(VALUE_BIT_WIDTH -
                                __builtin_clzll(static_cast<uint64_t>(-value)));
  } else {
    return static_cast<uint8_t>(VALUE_BIT_WIDTH -
                                __builtin_clzll(static_cast<uint64_t>(value)));
  }
}

// This multiplication is not standard fixed-point multiplication - it is
// modified to prevent overflow for any case - however, the overflow logic is
// such that it only scales down values when overflow occurs - this is strictly
// better than allowing overflow. It must be done in the operator for engine
// design considerations. (This also applies to the division operator.)
template <uint8_t F>
constexpr Fixed_Point_Integer<F> Fixed_Point_Integer<F>::operator*(
    const Fixed_Point_Integer other) const {
  int64_t product =
      static_cast<int64_t>(m_value) * static_cast<int64_t>(other.m_value);

  // To prevent overflow - we calculate the number of bits in the  product and
  // calculate the magnitude of the shift necessary to fit the product within
  // (FIXED_POINT_BIT_WIDTH - 1) bits accounting for the sign bit. We subtract
  // off the shift by F we already need to do, described below.
  int8_t anti_overflow_scaling =
      bit_width(product) - F - (FIXED_POINT_BIT_WIDTH - 1);
  anti_overflow_scaling =
      std::max(static_cast<int8_t>(0), anti_overflow_scaling);

  // This rounding method is to drive the expected value of the error produced
  // by truncation towards zero. When you truncate (i.e. scale down by 2^F) you
  // lose precision and drive the error in one-direction (towards zero), with
  // this logic you either round up or down.
  // Adding or subtracting by (1LL << (F - 1)) is the equivalent of 0.5 because:
  // (((V * V) * 2^(2F)) + (2^(F-1))) / 2^F = (V * V) * 2^F + 0.5
  int64_t rounding_offset = 1LL << (F + anti_overflow_scaling - 1);
  if (product >= 0) {
    product += rounding_offset;  // Rounding for positive numbers
  } else {
    product -= rounding_offset;  // Rounding for negative numbers
  }

  // Scale down by 2^F because fixed point integers are real numbers (V) scaled
  // by 2^F. So:
  // (V * (2^F)) * (V * (2^F)) = (V * V) * (2^(2F))
  // To get back to the correct scale we need to divide by 2^F.
  product = product >> (F + anti_overflow_scaling);
  return Fixed_Point_Integer::from_value(static_cast<int32_t>(product));
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F> Fixed_Point_Integer<F>::operator/(
    const Fixed_Point_Integer other) const {
  // Similar to multiplication, we need to calculate the anti-overflow scaling
  // necessary to fit the quotient within (FIXED_POINT_BIT_WIDTH - 1) bits
  // accounting for sign bits. We add F to the shift because we are scaling up
  // the numerator by 2^F, so we need to account for that in our calculations.
  // Also unlike multiplication, we subtract off the bits in the divisor.
  int8_t anti_overflow_scaling = bit_width(m_value) + F -
                                 bit_width(other.m_value) -
                                 (FIXED_POINT_BIT_WIDTH - 1);
  anti_overflow_scaling =
      std::max(static_cast<int8_t>(0), anti_overflow_scaling);

  // We scale the numerator up for the same reason we scale down the product in
  // multiplication.
  int64_t numerator = (static_cast<int64_t>(m_value) << F);
  int32_t denominator = other.m_value >> anti_overflow_scaling;

  // Prevent division by zero by clamping to the smallest denominator.
  if (denominator == 0) {
    denominator = (other.m_value >= 0) ? 1 : -1;
  }

  int64_t half_denominator = std::abs(static_cast<int64_t>(denominator)) / 2;

  // This round method works the same as the multiplication rounding method
  // except we are doing:
  // floor((N/D) + (1/2))
  if ((numerator >= 0) == (denominator >= 0)) {
    numerator += half_denominator;  // Rounding for positive result
  } else {
    numerator -= half_denominator;  // Rounding for negative result
  }

  // We scale both the numerator and denominator (see above) down because this
  // maintains the same result - the scaling factor cancels out - but prevents
  // overflow.
  numerator = numerator >> anti_overflow_scaling;

  int32_t result_value = static_cast<int32_t>(numerator / denominator);
  return Fixed_Point_Integer::from_value(result_value);
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F> Fixed_Point_Integer<F>::operator+(
    const int32_t other) const {
  return *this + Fixed_Point_Integer::from_integer(other);
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F> Fixed_Point_Integer<F>::operator-(
    const int32_t other) const {
  return *this - Fixed_Point_Integer::from_integer(other);
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F> Fixed_Point_Integer<F>::operator/(
    const int32_t other) const {
  return *this / Fixed_Point_Integer::from_integer(other);
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F> Fixed_Point_Integer<F>::operator*(
    const int32_t other) const {
  return *this * Fixed_Point_Integer::from_integer(other);
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F> Fixed_Point_Integer<F>::operator+(
    const double other) const {
  return *this + Fixed_Point_Integer::from_double(other);
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F> Fixed_Point_Integer<F>::operator-(
    const double other) const {
  return *this - Fixed_Point_Integer::from_double(other);
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F> Fixed_Point_Integer<F>::operator/(
    const double other) const {
  return *this / Fixed_Point_Integer::from_double(other);
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F> Fixed_Point_Integer<F>::operator*(
    const double other) const {
  return *this * Fixed_Point_Integer::from_double(other);
}

template <uint8_t F>
constexpr inline auto Fixed_Point_Integer<F>::operator<=>(
    const Fixed_Point_Integer<F> other) const {
  return m_value <=> other.m_value;
}

template <uint8_t F>
constexpr inline auto Fixed_Point_Integer<F>::operator<=>(
    const int32_t other) const {
  return m_value <=> Fixed_Point_Integer<F>::from_integer(other).m_value;
}

template <uint8_t F>
constexpr inline auto Fixed_Point_Integer<F>::operator<=>(
    const double other) const {
  return m_value <=> Fixed_Point_Integer<F>::from_double(other).m_value;
}

template <uint8_t F>
constexpr inline bool Fixed_Point_Integer<F>::operator==(
    const Fixed_Point_Integer& other) const {
  return m_value == other.m_value;
}

template <uint8_t F>
constexpr inline bool Fixed_Point_Integer<F>::operator==(
    const int32_t other) const {
  return m_value == Fixed_Point_Integer<F>::from_integer(other).m_value;
}

template <uint8_t F>
constexpr inline bool Fixed_Point_Integer<F>::operator==(
    const double other) const {
  return m_value == Fixed_Point_Integer<F>::from_double(other).m_value;
}

template <uint8_t F>
constexpr log2_lookup_table_type Fixed_Point_Integer<F>::make_log2_table() {
  log2_lookup_table_type table{};
  for (std::size_t i = 0; i < LOG2_LOOKUP_TABLE_SIZE; ++i) {
    double c = 1.0L + (static_cast<double>(i) / LOG2_LOOKUP_TABLE_SIZE);
    double log_c = std::log2l(c);
    table[i] = static_cast<int32_t>(std::llround(log_c * (1LL << F)));
  }
  return table;
}

template <uint8_t F>
constexpr exp2_lookup_table_type Fixed_Point_Integer<F>::make_exp2_table() {
  exp2_lookup_table_type table{};
  for (std::size_t i = 0; i < EXP2_LOOKUP_TABLE_SIZE; ++i) {
    double c = static_cast<double>(i) / EXP2_LOOKUP_TABLE_SIZE;
    double exp_c = std::exp2l(c);
    table[i] = static_cast<int32_t>(std::llround(exp_c * (1LL << F)));
  }
  return table;
}

namespace Matrex {
template <uint8_t F>
Fixed_Point_Integer<F> tanh(const Fixed_Point_Integer<F> input);
double tanh(double x);
template <uint8_t F>
Fixed_Point_Integer<F> pow(const Fixed_Point_Integer<F> base,
                           const Fixed_Point_Integer<F> exponent);
double pow(double base, double exponent);
template <uint8_t F>
Fixed_Point_Integer<F> log2(const Fixed_Point_Integer<F> input);
template <uint8_t F>
Fixed_Point_Integer<F> exp2(const Fixed_Point_Integer<F> input);
template <uint8_t F>
Fixed_Point_Integer<F> ln1p_approximation(const Fixed_Point_Integer<F> input);
template <uint8_t F>
Fixed_Point_Integer<F> exponential_approximation(
    const Fixed_Point_Integer<F> input);
template <uint8_t F>
Fixed_Point_Integer<F> sqrt(const Fixed_Point_Integer<F> input);
double sqrt(double x);
template <uint8_t F>
Fixed_Point_Integer<F> exp(const Fixed_Point_Integer<F> input);
double exp(double x);
}  // namespace Matrex
