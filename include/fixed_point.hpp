#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>

constexpr double LN_2 = 0.69314718056;  // Precomputed value of ln(2).

constexpr std::size_t LOG2_LOOKUP_TABLE_SIZE = 4096;
using log2_lookup_table_type = std::array<int32_t, LOG2_LOOKUP_TABLE_SIZE>;

constexpr std::size_t EXP2_LOOKUP_TABLE_SIZE = 4096;
using exp2_lookup_table_type = std::array<int32_t, EXP2_LOOKUP_TABLE_SIZE>;

template <uint8_t F>  // F is the number of fractional bits.
class Fixed_Point_Integer {
 public:
  Fixed_Point_Integer() : m_value(0) {}
  explicit Fixed_Point_Integer(int32_t value) : m_value(value) {}

  int32_t get_value() const;
  double to_double() const;

  Fixed_Point_Integer floor() const;

  // Arithmetic operators with another fixed-point integer.
  Fixed_Point_Integer operator+(const Fixed_Point_Integer other) const;
  Fixed_Point_Integer operator-(const Fixed_Point_Integer other) const;
  Fixed_Point_Integer operator/(const Fixed_Point_Integer other) const;
  Fixed_Point_Integer operator*(const Fixed_Point_Integer other) const;

  // Arithmetic operators with a non-fixed-point integer.
  Fixed_Point_Integer operator+(const int32_t other) const;
  Fixed_Point_Integer operator-(const int32_t other) const;
  Fixed_Point_Integer operator/(const int32_t other) const;
  Fixed_Point_Integer operator*(const int32_t other) const;

  // Arithmetic operators with a double.
  Fixed_Point_Integer operator+(const double other) const;
  Fixed_Point_Integer operator-(const double other) const;
  Fixed_Point_Integer operator/(const double other) const;
  Fixed_Point_Integer operator*(const double other) const;

  inline auto operator<=>(const Fixed_Point_Integer other) const;
  inline auto operator<=>(const int32_t other) const;
  inline auto operator<=>(const double other) const;

  inline bool operator==(const Fixed_Point_Integer& other) const;
  inline bool operator==(const int32_t other) const;
  inline bool operator==(const double other) const;

  template <typename W>
  friend std::ostream& operator<<(std::ostream& os,
                                  const Fixed_Point_Integer& weights);

  // Value is already an integer in fixed-point representation.
  static Fixed_Point_Integer from_value(int32_t value) {
    Fixed_Point_Integer result;
    result.m_value = value;
    return result;
  }

  static Fixed_Point_Integer from_integer(int32_t integer) {
    // Scale up by 2^F to convert to fixed-point representation.
    int32_t value = integer << F;
    return Fixed_Point_Integer::from_value(value);
  }

  static Fixed_Point_Integer from_double(double real) {
    int32_t value = static_cast<int32_t>(std::llround(real * (1LL << F)));
    return Fixed_Point_Integer::from_value(value);
  }

  static int32_t lookup_log2_table(std::size_t index) {
    return m_log2_table[index];
  }

  static int32_t lookup_exp2_table(std::size_t index) {
    return m_exp2_table[index];
  }

 private:
  int32_t m_value;

  static constexpr log2_lookup_table_type make_log2_table();
  static constexpr exp2_lookup_table_type make_exp2_table();

  static constexpr log2_lookup_table_type m_log2_table = make_log2_table();
  static constexpr exp2_lookup_table_type m_exp2_table = make_exp2_table();
};

template <uint8_t F>
int32_t Fixed_Point_Integer<F>::get_value() const {
  return m_value;
}

template <uint8_t F>
double Fixed_Point_Integer<F>::to_double() const {
  return static_cast<double>(m_value) / (1LL << F);
}

template <uint8_t F>
Fixed_Point_Integer<F> Fixed_Point_Integer<F>::floor() const {
  int32_t integer_part = m_value & (~((1LL << F) - 1));
  return Fixed_Point_Integer::from_value(integer_part);
}

template <uint8_t F>
Fixed_Point_Integer<F> Fixed_Point_Integer<F>::operator+(
    const Fixed_Point_Integer other) const {
  return Fixed_Point_Integer::from_value(m_value + other.m_value);
}

template <uint8_t F>
Fixed_Point_Integer<F> Fixed_Point_Integer<F>::operator-(
    const Fixed_Point_Integer other) const {
  return Fixed_Point_Integer::from_value(m_value - other.m_value);
}

template <uint8_t F>
Fixed_Point_Integer<F> Fixed_Point_Integer<F>::operator*(
    const Fixed_Point_Integer other) const {
  // Multiplication of two fixed-point integers requires scaling down by 2^F.
  int64_t product =
      static_cast<int64_t>(m_value) * static_cast<int64_t>(other.m_value);

  // This rounding method is to drive the expected value of the error produced
  // by truncation towards zero. When you truncate (i.e. scale down by 2^F) you
  // lose precision and drive the error in one-direction (towards zero), with
  // this logic you either round up or down.
  // Adding or subtracting by (1LL << (F - 1)) is the equivalent of 0.5 because:
  // (((V * V) * 2^(2F)) + (2^(F-1))) / 2^F = (V * V) * 2^F + 0.5
  constexpr int64_t rounding_offset = 1LL << (F - 1);
  if (product >= 0) {
    product += rounding_offset;  // Rounding for positive numbers
  } else {
    product -= rounding_offset;  // Rounding for negative numbers
  }

  // Scale down by 2^F because fixed point integers are real numbers (V) scaled
  // by 2^F. So:
  // (V * (2^F)) * (V * (2^F)) = (V * V) * (2^(2F))
  // To get back to the correct scale we need to divide by 2^F,
  int32_t result_value = static_cast<int32_t>(product >> F);
  return Fixed_Point_Integer::from_value(result_value);
}

template <uint8_t F>
Fixed_Point_Integer<F> Fixed_Point_Integer<F>::operator/(
    const Fixed_Point_Integer other) const {
  if (other.m_value == 0) {
    throw std::runtime_error("Division by zero in Fixed_Point_Integer.");
  }

  // We scale the numerator up for the same reason we scale down the product in
  // multiplication.
  int64_t numerator = (static_cast<int64_t>(m_value) << F);

  int64_t half_denominator = std::abs(static_cast<int64_t>(other.m_value)) / 2;

  // This round method works the same as the multiplication rounding method
  // except we are doing:
  // floor((N/D) + (1/2))
  if ((numerator >= 0) == (other.m_value >= 0)) {
    numerator += half_denominator;  // Rounding for positive result
  } else {
    numerator -= half_denominator;  // Rounding for negative result
  }

  int32_t result_value = static_cast<int32_t>(numerator / other.m_value);
  return Fixed_Point_Integer::from_value(result_value);
}

template <uint8_t F>
Fixed_Point_Integer<F> Fixed_Point_Integer<F>::operator+(
    const int32_t other) const {
  return *this + Fixed_Point_Integer::from_integer(other);
}

template <uint8_t F>
Fixed_Point_Integer<F> Fixed_Point_Integer<F>::operator-(
    const int32_t other) const {
  return *this - Fixed_Point_Integer::from_integer(other);
}

template <uint8_t F>
Fixed_Point_Integer<F> Fixed_Point_Integer<F>::operator/(
    const int32_t other) const {
  return *this / Fixed_Point_Integer::from_integer(other);
}

template <uint8_t F>
Fixed_Point_Integer<F> Fixed_Point_Integer<F>::operator*(
    const int32_t other) const {
  return *this * Fixed_Point_Integer::from_integer(other);
}

template <uint8_t F>
Fixed_Point_Integer<F> Fixed_Point_Integer<F>::operator+(
    const double other) const {
  return *this + Fixed_Point_Integer::from_double(other);
}

template <uint8_t F>
Fixed_Point_Integer<F> Fixed_Point_Integer<F>::operator-(
    const double other) const {
  return *this - Fixed_Point_Integer::from_double(other);
}

template <uint8_t F>
Fixed_Point_Integer<F> Fixed_Point_Integer<F>::operator/(
    const double other) const {
  return *this / Fixed_Point_Integer::from_double(other);
}

template <uint8_t F>
Fixed_Point_Integer<F> Fixed_Point_Integer<F>::operator*(
    const double other) const {
  return *this * Fixed_Point_Integer::from_double(other);
}

template <uint8_t F>
inline auto Fixed_Point_Integer<F>::operator<=>(
    const Fixed_Point_Integer<F> other) const {
  return m_value <=> other.m_value;
}

template <uint8_t F>
inline auto Fixed_Point_Integer<F>::operator<=>(const int32_t other) const {
  return m_value <=> Fixed_Point_Integer<F>::from_integer(other).m_value;
}

template <uint8_t F>
inline auto Fixed_Point_Integer<F>::operator<=>(const double other) const {
  return m_value <=> Fixed_Point_Integer<F>::from_double(other).m_value;
}

template <uint8_t F>
inline bool Fixed_Point_Integer<F>::operator==(
    const Fixed_Point_Integer& other) const {
  return m_value == other.m_value;
}

template <uint8_t F>
inline bool Fixed_Point_Integer<F>::operator==(const int32_t other) const {
  return m_value == Fixed_Point_Integer<F>::from_integer(other).m_value;
}

template <uint8_t F>
inline bool Fixed_Point_Integer<F>::operator==(const double other) const {
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

// Function prototypes.
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

// 3/3 Pade Approximation of tanh(x) near 0.
template <uint8_t F>
Fixed_Point_Integer<F> tanh(const Fixed_Point_Integer<F> input) {
  if (input <= -5.8917) {
    return Fixed_Point_Integer<F>::from_integer(-1);
  }

  if (input >= 5.8917) {
    return Fixed_Point_Integer<F>::from_integer(1);
  }

  const Fixed_Point_Integer<F> input_power_two = input * input;
  const Fixed_Point_Integer<F> input_power_four =
      input_power_two * input_power_two;
  const Fixed_Point_Integer<F> input_power_six =
      input_power_four * input_power_two;

  const Fixed_Point_Integer<F> numerator =
      input * ((input_power_two * 17325) + (input_power_four * 378) +
               input_power_six + 135135);
  const Fixed_Point_Integer<F> denominator =
      ((input_power_two * 62370) + (input_power_four * 3150) +
       (input_power_six * 28) + 135135);

  return (numerator / denominator);
}

double tanh(double x) { return std::tanh(x); }

// A pow function for fixed-point integers that uses the identity:
// base^exponent = exp2(log2(base) * exponent)
template <uint8_t F>
Fixed_Point_Integer<F> pow(const Fixed_Point_Integer<F> base,
                           const Fixed_Point_Integer<F> exponent) {
  if ((base == Fixed_Point_Integer<F>::from_integer(0)) ||
      (exponent == Fixed_Point_Integer<F>::from_integer(1))) {
    return base;
  }

  if (exponent == Fixed_Point_Integer<F>::from_integer(0)) {
    return Fixed_Point_Integer<F>::from_integer(1);
  }

  return Matrex::exp2(Matrex::log2(base) * exponent);
}

double pow(double base, double exponent) { return std::pow(base, exponent); }

// NOTE: The input must be non-negative and non-zero.
template <uint8_t F>
Fixed_Point_Integer<F> log2(const Fixed_Point_Integer<F> input) {
  // Factor the input into (2^e * m) where m is in [1, 2) where e is an
  // integer. This may look like a familaur factorization because it's the
  // same way that floating point numbers are represented.
  // We can determine e simply by knowing the position of the first high bit
  // in the fixed-point representation of the input and subtracting off F.
  // This is equivalent to floor(log2(input)) or the closest power of two
  // to the input without exceeding the input.
  const int32_t e = 31 - __builtin_clz(input.get_value()) - F;
  int32_t m_raw;
  if (e >= 0) {
    m_raw = input.get_value() >> e;
  } else {
    m_raw = input.get_value() << (-e);
  }
  const Fixed_Point_Integer<F> m = Fixed_Point_Integer<F>::from_value(m_raw);

  // Here we want to calculate the index of that we will be using to calculate
  // the value of c and the index in which log(c) is stored in the lookup
  // table. By design, we want m to be very close to c, this will make t a
  // very small number which is ideal for a Pade approximation around 0 of
  // ln(1+t). To calculate the index, we simply map the m we found which is a
  // number in the range [1, 2) to an index in the range [0, N) (N being the
  // number of entries in the lookup table) - this index is the "bucket" from
  // which m is contained. Since, we want c close to m, we map the index we
  // found in the range of [0, N) back to a number in the range [1, 2) and use
  // that as c - c is thus the left edge of the bucket that m is contained in
  // (given how we calculate it).
  const Fixed_Point_Integer<F> log2_lookup_table_size_in_fp =
      Fixed_Point_Integer<F>::from_integer(
          static_cast<int32_t>(LOG2_LOOKUP_TABLE_SIZE));
  std::size_t index = ((m - Fixed_Point_Integer<F>::from_integer(1)) *
                       log2_lookup_table_size_in_fp)
                          .get_value() >>
                      F;
  index =
      std::clamp(index, static_cast<std::size_t>(0),
                 (LOG2_LOOKUP_TABLE_SIZE -
                  1));  // Clamp index to be within bounds because of rounding.
  const Fixed_Point_Integer<F> c =
      Fixed_Point_Integer<F>::from_integer(1) +
      (Fixed_Point_Integer<F>::from_integer(static_cast<int32_t>(index)) /
       log2_lookup_table_size_in_fp);

  // t is the residual component of m where m is c * (1 + t). Note, that we
  // use (1 + t) instead of t because in computing t = m/c (from m = ct), t is
  // bounded between [0.5, 2) however, if we do t = ((m/c) - 1), t's upper
  // bound is 1/N (N being the size of the lookup table) which is ideal for a
  // Pade approximation around 0 - we know this because:
  //  t = (m/c) - 1 which is (m - c) / c
  //  and we know (m - c) must be less than (1/N) because m and c are in the
  //  same bucket of size (1/N) and c is the left edge of the bucket. Thus,
  //  t is strictly less than (1 / N).
  const Fixed_Point_Integer<F> t =
      (m / c) - Fixed_Point_Integer<F>::from_integer(1);

  return Fixed_Point_Integer<F>::from_integer(e) +
         Fixed_Point_Integer<F>::from_value(
             Fixed_Point_Integer<F>::lookup_log2_table(index)) +
         (Matrex::ln1p_approximation(t) /
          Fixed_Point_Integer<F>::from_double(LN_2));
}

template <uint8_t F>
Fixed_Point_Integer<F> exp2(const Fixed_Point_Integer<F> input) {
  // What we are doing here is taking the input decomposing it into its
  // integer and fractional parts.
  // 2^x = 2^(integer_part + fractional_part) = 2^integer_part *
  // 2^fractional_part
  // The integer part (2^integer_part) can be calculated simply by bit
  // shifting.
  // (NOTE: The integer part is not shifted here, only masked!)
  Fixed_Point_Integer<F> integer_part = input.floor();
  Fixed_Point_Integer<F> fractional_part = input - integer_part;

  // We can break the fractional part further:
  // 2^fractional_part = 2^((i/N) + r) = 2^(i/N) * 2^(r)
  // This is the same logic we applied in log2 to get a small enough residual
  // r to approximate 2^r with a Pade approximation around 0.
  // The fractional part f is a number in the range [0, 1). So to get a number
  // i in the range [0, N) we can simply floor the multiplication of the
  // fractional part by N (we floor as we don't want 2^(i/n) > 2^f). So:
  // f = (i/N) + r where i = floor(f * N)
  // r = f - (i/N) where i = floor(f * N)
  // Remembering we have to divide i by N to get it back to the correct scale
  // because i is in the range [0, N) but we want (i/N) to be in the range
  // [0, 1).
  const Fixed_Point_Integer<F> i =
      (fractional_part * Fixed_Point_Integer<F>::from_integer(
                             static_cast<int32_t>(EXP2_LOOKUP_TABLE_SIZE)))
          .floor();
  const Fixed_Point_Integer<F> r =
      fractional_part - (i / Fixed_Point_Integer<F>::from_integer(
                                 static_cast<int32_t>(EXP2_LOOKUP_TABLE_SIZE)));

  // The clamped integer part of i becomes the index for the lookup table and
  // we perform the lookup.
  std::size_t index = i.get_value() >> F;
  index =
      std::clamp(index, static_cast<std::size_t>(0),
                 (EXP2_LOOKUP_TABLE_SIZE -
                  1));  // Clamp index to be within bounds because of rounding.
  Fixed_Point_Integer<F> table_lookup_value =
      Fixed_Point_Integer<F>::from_value(
          Fixed_Point_Integer<F>::lookup_exp2_table(index));

  // For r: we rewrite 2^r as e^(r * ln(2)).
  Fixed_Point_Integer<F> exp2_r = Matrex::exponential_approximation(
      Fixed_Point_Integer<F>::from_double(LN_2) * r);
  Fixed_Point_Integer<F> fractional_part_result = (table_lookup_value * exp2_r);

  // Now we calculate 2^(integer_part) for both positive and negative
  // integer_part which simply translates into a bit shift of the fractional
  // part.
  int32_t shift = (integer_part.get_value() >> F);
  int32_t result = fractional_part_result.get_value();
  if (shift >= 0) {
    result = result << shift;
  } else {
    result = result >> (-shift);
  }

  return Fixed_Point_Integer<F>::from_value(result);
}

// 3/3 Pade Approximation of ln(1+t) around 0.
template <uint8_t F>
Fixed_Point_Integer<F> ln1p_approximation(const Fixed_Point_Integer<F> input) {
  const Fixed_Point_Integer<F> input_power_two = input * input;
  const Fixed_Point_Integer<F> input_power_three = input_power_two * input;

  const Fixed_Point_Integer<F> numerator =
      (input * 60) + (input_power_two * 60) + (input_power_three * 11);
  const Fixed_Point_Integer<F> denominator =
      (input * 90) + (input_power_two * 36) + (input_power_three * 3) + 60;

  return numerator / denominator;
}

// 3/3 Pade Approximation of e^x around 0.
template <uint8_t F>
Fixed_Point_Integer<F> exponential_approximation(
    const Fixed_Point_Integer<F> input) {
  const Fixed_Point_Integer<F> input_power_two = input * input;
  const Fixed_Point_Integer<F> input_power_three = input_power_two * input;

  const Fixed_Point_Integer<F> numerator =
      (input * 60) + (input_power_two * 12) + input_power_three + 120;
  const Fixed_Point_Integer<F> denominator =
      (input * -60) + (input_power_two * 12) - input_power_three + 120;

  return numerator / denominator;
}
}  // namespace Matrex
