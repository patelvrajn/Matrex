#include "fixed_point.hpp"

namespace Matrex {
// NOTE: Do not use a pade approximation, this method is exponentially more
// accurate.
template <uint8_t F>
Fixed_Point_Integer<F> tanh(const Fixed_Point_Integer<F> input) {
  constexpr Fixed_Point_Integer<F> FIXED_NATURAL_E =
      Fixed_Point_Integer<F>::from_double(NATURAL_E);
  const Fixed_Point_Integer<F> exponent =
      (input.get_value() >= 0) ? (input * -2) : (input * 2);
  const Fixed_Point_Integer<F> pow_result = pow(FIXED_NATURAL_E, exponent);

  Fixed_Point_Integer<F> numerator = (pow_result * -1) + 1;
  Fixed_Point_Integer<F> denominator = pow_result + 1;
  Fixed_Point_Integer<F> result = numerator / denominator;
  result = (input.get_value() >= 0) ? result : (result * -1);

  return result;
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
  Fixed_Point_Integer<F> result = fractional_part_result;
  if (shift >= 0) {
    // Guard against overflow by using fixed point integer multiplication.
    Fixed_Point_Integer<F> shift_fixed =
        Fixed_Point_Integer<F>::from_integer((1 << shift));
    result = result * shift_fixed;
  } else {
    result = Fixed_Point_Integer<F>::from_value(result.get_value() >> (-shift));
  }

  return result;
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

template <uint8_t F>
Fixed_Point_Integer<F> sqrt(const Fixed_Point_Integer<F> input) {
  return Matrex::pow(input, Fixed_Point_Integer<F>::from_double(0.5));
}

double sqrt(double x) { return std::sqrt(x); }

template <uint8_t F>
Fixed_Point_Integer<F> exp(const Fixed_Point_Integer<F> input) {
  return Matrex::pow(Fixed_Point_Integer<F>::from_double(NATURAL_E), input);
}

double exp(double x) { return std::exp(x); }
}  // namespace Matrex