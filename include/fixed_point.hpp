#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <numbers>
#include <stdexcept>

#include "globals.hpp"

using Fixed_Point_Int_Storage_Type = int32_t;

constexpr uint8_t FIXED_POINT_BIT_WIDTH =
    static_cast<uint8_t>(sizeof(Fixed_Point_Int_Storage_Type) * 8);

constexpr double LN_2      = 0.69314718056; // Precomputed value of ln(2).
constexpr double NATURAL_E = std::numbers::e;

constexpr std::size_t LOG2_LOOKUP_TABLE_SIZE = 4096;
using log2_lookup_table_type =
    std::array<Fixed_Point_Int_Storage_Type, LOG2_LOOKUP_TABLE_SIZE>;

constexpr std::size_t EXP2_LOOKUP_TABLE_SIZE = 4096;
using exp2_lookup_table_type =
    std::array<Fixed_Point_Int_Storage_Type, EXP2_LOOKUP_TABLE_SIZE>;

template <uint8_t F> // F is the number of fractional bits.
class Fixed_Point_Integer
{
  public:

    constexpr Fixed_Point_Integer() : m_value(0) {}

    explicit constexpr Fixed_Point_Integer(Fixed_Point_Int_Storage_Type value) :
        m_value(value)
    {
    }

    constexpr Fixed_Point_Int_Storage_Type get_value() const;
    constexpr double                       to_double() const;

    constexpr Fixed_Point_Integer floor() const;

    constexpr Fixed_Point_Int_Storage_Type get_integer() const;
    constexpr Fixed_Point_Int_Storage_Type get_fractional() const;

    // Arithmetic operators with another fixed-point integer.
    constexpr Fixed_Point_Integer
    operator+(const Fixed_Point_Integer other) const;
    constexpr Fixed_Point_Integer
    operator-(const Fixed_Point_Integer other) const;
    FORCE_INLINE constexpr Fixed_Point_Integer
    operator/(const Fixed_Point_Integer other) const;
    FORCE_INLINE constexpr Fixed_Point_Integer
    operator*(const Fixed_Point_Integer other) const;
    constexpr Fixed_Point_Integer operator+=(const Fixed_Point_Integer other);
    constexpr Fixed_Point_Integer operator-=(const Fixed_Point_Integer other);
    constexpr Fixed_Point_Integer operator/=(const Fixed_Point_Integer other);
    constexpr Fixed_Point_Integer operator*=(const Fixed_Point_Integer other);
    constexpr Fixed_Point_Integer operator-() const;

    // Arithmetic operators with a non-fixed-point integer.
    constexpr Fixed_Point_Integer
    operator+(const Fixed_Point_Int_Storage_Type other) const;
    constexpr Fixed_Point_Integer
    operator-(const Fixed_Point_Int_Storage_Type other) const;
    constexpr Fixed_Point_Integer
    operator/(const Fixed_Point_Int_Storage_Type other) const;
    constexpr Fixed_Point_Integer
    operator*(const Fixed_Point_Int_Storage_Type other) const;
    constexpr Fixed_Point_Integer
    operator+=(const Fixed_Point_Int_Storage_Type other);
    constexpr Fixed_Point_Integer
    operator-=(const Fixed_Point_Int_Storage_Type other);
    constexpr Fixed_Point_Integer
    operator/=(const Fixed_Point_Int_Storage_Type other);
    constexpr Fixed_Point_Integer
    operator*=(const Fixed_Point_Int_Storage_Type other);

    // Arithmetic operators with a double.
    constexpr Fixed_Point_Integer operator+(const double other) const;
    constexpr Fixed_Point_Integer operator-(const double other) const;
    constexpr Fixed_Point_Integer operator/(const double other) const;
    constexpr Fixed_Point_Integer operator*(const double other) const;
    constexpr Fixed_Point_Integer operator+=(const double other);
    constexpr Fixed_Point_Integer operator-=(const double other);
    constexpr Fixed_Point_Integer operator/=(const double other);
    constexpr Fixed_Point_Integer operator*=(const double other);

    constexpr auto operator<=>(const Fixed_Point_Integer other) const;
    constexpr auto operator<=>(const Fixed_Point_Int_Storage_Type other) const;
    constexpr auto operator<=>(const double other) const;

    constexpr bool operator==(const Fixed_Point_Integer& other) const;
    constexpr bool operator==(const Fixed_Point_Int_Storage_Type other) const;
    constexpr bool operator==(const double other) const;

    template <uint8_t X>
    friend std::ostream& operator<<(std::ostream&                 os,
                                    const Fixed_Point_Integer<X>& fp);

    static consteval uint8_t fractional_bits() { return F; }

    static consteval uint8_t integer_bits()
    {
        return (FIXED_POINT_BIT_WIDTH - F);
    }

    static consteval double scale() { return static_cast<double>(1 << F); }

    static consteval double precision() { return (1.0 / scale()); }

    // The maximum value the fractional component can have.
    static consteval double maximum_fractional()
    {
        double max_fractional = 0;
        for (uint8_t exponent = 1; exponent <= F; ++exponent)
        {
            max_fractional += 1.0 / static_cast<double>(1 << exponent);
        }
        return max_fractional;
    }

    // The maximum value the integer component can have.
    static consteval double maximum_integer()
    {
        Fixed_Point_Int_Storage_Type max_integer =
            (1 << (FIXED_POINT_BIT_WIDTH - F - 1)) - 1;
        return static_cast<double>(max_integer);
    }

    static consteval uint8_t safe_maximum_integer_bits()
    {
        return std::floor(std::log2(std::pow(
            static_cast<double>(
                std::numeric_limits<Fixed_Point_Int_Storage_Type>::max())
                / scale(),
            (1.0 / 4.0))));
    }

    static consteval double safe_maximum_integer()
    {
        Fixed_Point_Int_Storage_Type max_integer =
            (1 << safe_maximum_integer_bits()) - 1;
        return static_cast<double>(max_integer);
    }

    static consteval double maximum()
    {
        double max = maximum_integer() + maximum_fractional();
        max        = std::min(
            max,
            static_cast<double>(
                std::numeric_limits<Fixed_Point_Int_Storage_Type>::max()
                * precision()));
        return max;
    }

    static consteval double safe_maximum()
    {
        double max = safe_maximum_integer() + maximum_fractional();
        max        = std::min(
            max,
            static_cast<double>(
                std::numeric_limits<Fixed_Point_Int_Storage_Type>::max()
                * precision()));
        return max;
    }

    static consteval double minimum()
    {
        double min = -(maximum());
        min        = std::max(
            min,
            static_cast<double>(
                std::numeric_limits<Fixed_Point_Int_Storage_Type>::min()
                * precision()));
        return min;
    }

    static consteval double safe_minimum()
    {
        double min = -(safe_maximum());
        min        = std::max(
            min,
            static_cast<double>(
                std::numeric_limits<Fixed_Point_Int_Storage_Type>::min()
                * precision()));
        return min;
    }

    static constexpr bool is_representable(double value)
    {
        // Is this value representable with the precision of the fixed point
        // integer?
        if ((std::abs(value) < precision()) && (value != 0)) { return false; }

        // Is this value within the bounds of what the fixed point integer can
        // represent?
        return (value == std::clamp(value, minimum(), maximum()));
    }

    // We define safe as the range in which performing multiplication will not
    // result in an overflow. We only need to consider multiplication because it
    // has the largest gain in bit-width and magnitude. We can calculate the
    // safe range with:
    // (x * y) / 2^F <= 2^31 - 1 where x and y are the fixed point integers
    // being
    //                           multiplied and F is the number of fractional
    //                           bits.
    // (x * y) <= (2^31 - 1) * 2^F
    // For symmetric clamping of both operands:
    // z^2 <= (2^31 - 1) * 2^F
    // z <= sqrt((2^31 - 1) * 2^F)
    // That means z must be at most floor(log2(sqrt((2^31 - 1) * 2^F))) bits to
    // avoid overflow. (Since, the construction of the NLR is the multiplication
    // of four functions we do the 4th root instead of the square root.)
    // Since z is a fixed point integer, the real value is z/2^F.
    static constexpr bool is_safe(double value)
    {
        return (value == std::clamp(value, safe_minimum(), safe_maximum()));
    }

    // Clamp a double to be within range of the fixed point representation.
    static constexpr double clamp(double value)
    {
        return std::clamp(value, minimum(), maximum());
    }

    static constexpr double safe_clamp(double value)
    {
        return std::clamp(value, safe_minimum(), safe_maximum());
    }

    // Value is already an integer in fixed-point representation.
    static constexpr Fixed_Point_Integer
    from_value(Fixed_Point_Int_Storage_Type value)
    {
        Fixed_Point_Integer result;
        result.m_value = value;
        return result;
    }

    static constexpr Fixed_Point_Integer
    from_integer(Fixed_Point_Int_Storage_Type integer)
    {
        // Scale up by 2^F to convert to fixed-point representation, before
        // doing so clamp the integer to prevent overflow.
        Fixed_Point_Int_Storage_Type minimum_integer =
            -(1 << (FIXED_POINT_BIT_WIDTH - F - 1));
        Fixed_Point_Int_Storage_Type maximum_integer =
            (1 << (FIXED_POINT_BIT_WIDTH - F - 1)) - 1;
        integer = std::clamp(integer, minimum_integer, maximum_integer);
        Fixed_Point_Int_Storage_Type value = integer * scale();
        return Fixed_Point_Integer::from_value(value);
    }

    static constexpr Fixed_Point_Integer from_double(double real)
    {
        double rounded = std::llround(real * scale());
        rounded        = std::clamp(
            rounded,
            static_cast<double>(
                std::numeric_limits<Fixed_Point_Int_Storage_Type>::min()),
            static_cast<double>(
                std::numeric_limits<Fixed_Point_Int_Storage_Type>::max()));
        Fixed_Point_Int_Storage_Type value =
            static_cast<Fixed_Point_Int_Storage_Type>(rounded);
        return Fixed_Point_Integer::from_value(value);
    }

    static constexpr Fixed_Point_Int_Storage_Type
    lookup_log2_table(std::size_t index)
    {
        return m_log2_table[index];
    }

    static constexpr Fixed_Point_Int_Storage_Type
    lookup_exp2_table(std::size_t index)
    {
        return m_exp2_table[index];
    }

    static constexpr Fixed_Point_Integer<F> FP_ONE =
        Fixed_Point_Integer<F>::from_integer(1);
    static constexpr Fixed_Point_Integer<F> FP_TWO =
        Fixed_Point_Integer<F>::from_integer(2);
    static constexpr Fixed_Point_Integer<F> FP_12 =
        Fixed_Point_Integer<F>::from_integer(12);
    static constexpr Fixed_Point_Integer<F> FP_SIXTY =
        Fixed_Point_Integer<F>::from_integer(60);
    static constexpr Fixed_Point_Integer<F> FP_120 =
        Fixed_Point_Integer<F>::from_integer(120);
    static constexpr Fixed_Point_Integer<F> FP_NATURAL_E =
        Fixed_Point_Integer<F>::from_double(NATURAL_E);
    static constexpr Fixed_Point_Integer<F> FP_LN_2 =
        Fixed_Point_Integer<F>::from_double(LN_2);

  private:

    Fixed_Point_Int_Storage_Type m_value;

    static constexpr log2_lookup_table_type make_log2_table();
    static constexpr exp2_lookup_table_type make_exp2_table();

    static constexpr log2_lookup_table_type m_log2_table = make_log2_table();
    static constexpr exp2_lookup_table_type m_exp2_table = make_exp2_table();
};

// Fixed Pointer Integer Type that Matrex Uses.
constexpr uint8_t MATREX_FP_INT_FRACTIONAL_BITS = 16;
using Matrex_FP_Int = Fixed_Point_Integer<MATREX_FP_INT_FRACTIONAL_BITS>;

template <typename T>
consteval T explicit_fp_integer_conversion(Fixed_Point_Int_Storage_Type value)
{
    if constexpr (std::is_same_v<T, Fixed_Point_Int_Storage_Type>)
    {
        return value;
    }
    else if constexpr (std::is_same_v<T, Matrex_FP_Int>)
    {
        return T::from_integer(value);
    }
}

template <typename T>
consteval T explicit_fp_double_conversion(double value)
{
    if constexpr (std::is_same_v<T, double>) { return value; }
    else if constexpr (std::is_same_v<T, Matrex_FP_Int>)
    {
        return T::from_double(value);
    }
}

template <uint8_t F>
constexpr Fixed_Point_Int_Storage_Type Fixed_Point_Integer<F>::get_value() const
{
    return m_value;
}

template <uint8_t F>
constexpr double Fixed_Point_Integer<F>::to_double() const
{
    return static_cast<double>(m_value) / static_cast<double>(1LL << F);
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F> Fixed_Point_Integer<F>::floor() const
{
    constexpr Fixed_Point_Int_Storage_Type mask         = (~((1 << F) - 1));
    Fixed_Point_Int_Storage_Type           integer_part = m_value & mask;
    return Fixed_Point_Integer::from_value(integer_part);
}

template <uint8_t F>
constexpr Fixed_Point_Int_Storage_Type
Fixed_Point_Integer<F>::get_integer() const
{
    return (get_value() >> F);
}

template <uint8_t F>
constexpr Fixed_Point_Int_Storage_Type
Fixed_Point_Integer<F>::get_fractional() const
{
    return (*this).get_value() - (*this).floor().get_value();
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F>
Fixed_Point_Integer<F>::operator+(const Fixed_Point_Integer other) const
{
    // Clamp in case of overflow.
    int64_t result = static_cast<int64_t>(m_value) + static_cast<int64_t>(other.m_value);
    Fixed_Point_Int_Storage_Type return_value =
        std::clamp(result,
                   static_cast<int64_t>(std::numeric_limits<Fixed_Point_Int_Storage_Type>::min()),
                   static_cast<int64_t>(std::numeric_limits<Fixed_Point_Int_Storage_Type>::max()));
    return Fixed_Point_Integer::from_value(return_value);
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F>
Fixed_Point_Integer<F>::operator-(const Fixed_Point_Integer other) const
{
    // Clamp in case of overflow.
    int64_t result = static_cast<int64_t>(m_value) - static_cast<int64_t>(other.m_value);
    Fixed_Point_Int_Storage_Type return_value =
        std::clamp(result,
                   static_cast<int64_t>(std::numeric_limits<Fixed_Point_Int_Storage_Type>::min()),
                   static_cast<int64_t>(std::numeric_limits<Fixed_Point_Int_Storage_Type>::max()));
    return Fixed_Point_Integer::from_value(return_value);
}

// Helper function to count leading zeros in a 32-bit signed integer based on
// our use case.
FORCE_INLINE constexpr uint8_t bit_width(Fixed_Point_Int_Storage_Type value)
{
    if (value == 0)
    {
        // All bits are zero, so no bit width.
        return 0;
    }
    else if (value < 0)
    {
        // We want to count leading zeros simply as an extremely fast way to do
        // (floor(log2(abs(value))) + 1) which is the number of bits needed to
        // represent the value in binary. Thus, we can count leading zeros on
        // the absolute value of the number. Value needs to be promoted to
        // int64_t because -INT_MIN is not representable in
        // Fixed_Point_Int_Storage_Type.
        return static_cast<uint8_t>(FIXED_POINT_BIT_WIDTH
                                    - __builtin_clz(static_cast<uint32_t>(
                                        -static_cast<int64_t>(value))));
    }
    else
    { // Positive value.
        return static_cast<uint8_t>(
            FIXED_POINT_BIT_WIDTH
            - __builtin_clz(static_cast<uint32_t>(value)));
    }
}

FORCE_INLINE constexpr uint8_t bit_width(int64_t value)
{
    constexpr uint8_t VALUE_BIT_WIDTH =
        static_cast<uint8_t>(sizeof(int64_t) * 8);
    if (value == 0) { return 0; }
    else if (value < 0)
    {
        // Note: Value is not casted up to __int128 as it is expected to be
        // within the range of int64_t.
        return static_cast<uint8_t>(
            VALUE_BIT_WIDTH - __builtin_clzll(static_cast<uint64_t>(-value)));
    }
    else
    {
        return static_cast<uint8_t>(
            VALUE_BIT_WIDTH - __builtin_clzll(static_cast<uint64_t>(value)));
    }
}

// This multiplication is not standard fixed-point multiplication - it is
// modified to prevent overflow for any case - however, the overflow logic is
// such that it only scales down values when overflow occurs - this is strictly
// better than allowing overflow. It must be done in the operator for engine
// design considerations. (This also applies to the division operator.)
template <uint8_t F>
FORCE_INLINE constexpr Fixed_Point_Integer<F>
Fixed_Point_Integer<F>::operator*(const Fixed_Point_Integer other) const
{
    int64_t product =
        static_cast<int64_t>(m_value) * static_cast<int64_t>(other.m_value);

    // To prevent overflow - we calculate the number of bits in the  product and
    // calculate the magnitude of the shift necessary to fit the product within
    // (FIXED_POINT_BIT_WIDTH - 1) bits accounting for the sign bit. We subtract
    // off the shift by F we already need to do, described below.
    int8_t anti_overflow_scaling =
        bit_width(product) - F - (FIXED_POINT_BIT_WIDTH - 1);

    // An additional bit of scaling is needed if the product is negative because
    // bit_width doesn't account for the sign of the product.
    if (product < 0) { anti_overflow_scaling++; }

    anti_overflow_scaling =
        std::max(static_cast<int8_t>(0), anti_overflow_scaling);

    // Scale down by 2^F because fixed point integers are real numbers (V)
    // scaled by 2^F. So: (V * (2^F)) * (V * (2^F)) = (V * V) * (2^(2F)) To get
    // back to the correct scale we need to divide by 2^F.
    const uint8_t total_scaling = F + anti_overflow_scaling;
    product                     = product >> total_scaling;

    MATREX_ASSERT(Fixed_Point_Integer<F>::is_representable(
                      static_cast<double>(product) * precision()),
                  "FIXED POINT MULTIPLICATION ERROR: Product exceeded what was "
                  "representable.");

    return Fixed_Point_Integer::from_value(
        static_cast<Fixed_Point_Int_Storage_Type>(product));
}

template <uint8_t F>
FORCE_INLINE constexpr Fixed_Point_Integer<F>
Fixed_Point_Integer<F>::operator/(const Fixed_Point_Integer other) const
{
    // Similar to multiplication, we need to calculate the anti-overflow scaling
    // necessary to fit the quotient within (FIXED_POINT_BIT_WIDTH - 1) bits
    // accounting for sign bits. We add F to the shift because we are scaling up
    // the numerator by 2^F, so we need to account for that in our calculations.
    // Also unlike multiplication, we subtract off the bits in the divisor.
    int8_t anti_overflow_scaling = bit_width(m_value) + F
                                 - bit_width(other.m_value)
                                 - (FIXED_POINT_BIT_WIDTH - 1) + 1;

    // An additional bit of scaling is needed if the product is negative because
    // bit_width doesn't account for the sign of the product.
    if (((m_value < 0) || (other.m_value < 0))
        && (!((m_value < 0) && (other.m_value < 0))))
    {
        anti_overflow_scaling++;
    }

    anti_overflow_scaling =
        std::max(static_cast<int8_t>(0), anti_overflow_scaling);

    // We scale the numerator up for the same reason we scale down the product
    // in multiplication.
    int64_t numerator = static_cast<int64_t>(m_value) * scale();
    int64_t denominator =
        static_cast<int64_t>(other.m_value) >> anti_overflow_scaling;

    // Prevent division by zero by clamping to the smallest denominator.
    if (denominator == 0) { denominator = (other.m_value >= 0) ? 1 : -1; }

    // We scale both the numerator and denominator (see above) down because this
    // maintains the same result - the scaling factor cancels out - but prevents
    // overflow.
    numerator = numerator >> anti_overflow_scaling;

    int64_t quotient = numerator / denominator;

    MATREX_ASSERT(Fixed_Point_Integer<F>::is_representable(
                      static_cast<double>(quotient) * precision()),
                  "FIXED POINT DIVISION ERROR: Quotient exceeded what was "
                  "representable.");

    return Fixed_Point_Integer::from_value(
        static_cast<Fixed_Point_Int_Storage_Type>(quotient));
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F>
Fixed_Point_Integer<F>::operator+=(const Fixed_Point_Integer<F> other)
{
    Fixed_Point_Integer<F> sum = (*this) + other;
    m_value                    = sum.m_value;

    return *this;
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F>
Fixed_Point_Integer<F>::operator-=(const Fixed_Point_Integer<F> other)
{
    Fixed_Point_Integer<F> difference = (*this) - other;
    m_value                           = difference.m_value;

    return *this;
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F>
Fixed_Point_Integer<F>::operator/=(const Fixed_Point_Integer<F> other)
{
    Fixed_Point_Integer<F> quotient = (*this) / other;
    m_value                         = quotient.m_value;

    return *this;
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F>
Fixed_Point_Integer<F>::operator*=(const Fixed_Point_Integer<F> other)
{
    Fixed_Point_Integer<F> product = (*this) * other;
    m_value                        = product.m_value;

    return *this;
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F> Fixed_Point_Integer<F>::operator-() const
{
    // We need to check if m_value is the minimum value for an integer because
    // if we negate it, it will be higher than the maximum value for an integer.
    // So we need to clamp it to the negative of the maximum value.
    if (m_value == std::numeric_limits<Fixed_Point_Int_Storage_Type>::min())
    {
        return Fixed_Point_Integer<F>::from_double(
            -Fixed_Point_Integer<F>::maximum());
    }
    else { return Fixed_Point_Integer<F>::from_value(-m_value); }
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F> Fixed_Point_Integer<F>::operator+(
    const Fixed_Point_Int_Storage_Type other) const
{
    return *this + Fixed_Point_Integer::from_integer(other);
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F> Fixed_Point_Integer<F>::operator-(
    const Fixed_Point_Int_Storage_Type other) const
{
    return *this - Fixed_Point_Integer::from_integer(other);
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F> Fixed_Point_Integer<F>::operator/(
    const Fixed_Point_Int_Storage_Type other) const
{
    return *this / Fixed_Point_Integer::from_integer(other);
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F> Fixed_Point_Integer<F>::operator*(
    const Fixed_Point_Int_Storage_Type other) const
{
    return *this * Fixed_Point_Integer::from_integer(other);
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F>
Fixed_Point_Integer<F>::operator+=(const Fixed_Point_Int_Storage_Type other)
{
    Fixed_Point_Integer<F> sum =
        (*this) + Fixed_Point_Integer::from_integer(other);
    m_value = sum.m_value;

    return *this;
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F>
Fixed_Point_Integer<F>::operator-=(const Fixed_Point_Int_Storage_Type other)
{
    Fixed_Point_Integer<F> difference =
        (*this) - Fixed_Point_Integer::from_integer(other);
    m_value = difference.m_value;

    return *this;
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F>
Fixed_Point_Integer<F>::operator/=(const Fixed_Point_Int_Storage_Type other)
{
    Fixed_Point_Integer<F> quotient =
        (*this) / Fixed_Point_Integer::from_integer(other);
    m_value = quotient.m_value;

    return *this;
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F>
Fixed_Point_Integer<F>::operator*=(const Fixed_Point_Int_Storage_Type other)
{
    Fixed_Point_Integer<F> product =
        (*this) * Fixed_Point_Integer::from_integer(other);
    m_value = product.m_value;

    return *this;
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F>
Fixed_Point_Integer<F>::operator+(const double other) const
{
    return *this + Fixed_Point_Integer::from_double(other);
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F>
Fixed_Point_Integer<F>::operator-(const double other) const
{
    return *this - Fixed_Point_Integer::from_double(other);
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F>
Fixed_Point_Integer<F>::operator/(const double other) const
{
    return *this / Fixed_Point_Integer::from_double(other);
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F>
Fixed_Point_Integer<F>::operator*(const double other) const
{
    return *this * Fixed_Point_Integer::from_double(other);
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F>
Fixed_Point_Integer<F>::operator+=(const double other)
{
    Fixed_Point_Integer<F> sum =
        (*this) + Fixed_Point_Integer::from_double(other);
    m_value = sum.m_value;

    return *this;
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F>
Fixed_Point_Integer<F>::operator-=(const double other)
{
    Fixed_Point_Integer<F> difference =
        (*this) - Fixed_Point_Integer::from_double(other);
    m_value = difference.m_value;

    return *this;
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F>
Fixed_Point_Integer<F>::operator/=(const double other)
{
    Fixed_Point_Integer<F> quotient =
        (*this) / Fixed_Point_Integer::from_double(other);
    m_value = quotient.m_value;

    return *this;
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F>
Fixed_Point_Integer<F>::operator*=(const double other)
{
    Fixed_Point_Integer<F> product =
        (*this) * Fixed_Point_Integer::from_double(other);
    m_value = product.m_value;

    return *this;
}

template <uint8_t F>
constexpr inline auto
Fixed_Point_Integer<F>::operator<=>(const Fixed_Point_Integer<F> other) const
{
    return m_value <=> other.m_value;
}

template <uint8_t F>
constexpr inline auto Fixed_Point_Integer<F>::operator<=>(
    const Fixed_Point_Int_Storage_Type other) const
{
    return m_value <=> Fixed_Point_Integer<F>::from_integer(other).m_value;
}

template <uint8_t F>
constexpr inline auto
Fixed_Point_Integer<F>::operator<=>(const double other) const
{
    return m_value <=> Fixed_Point_Integer<F>::from_double(other).m_value;
}

template <uint8_t F>
constexpr inline bool
Fixed_Point_Integer<F>::operator==(const Fixed_Point_Integer& other) const
{
    return m_value == other.m_value;
}

template <uint8_t F>
constexpr inline bool Fixed_Point_Integer<F>::operator==(
    const Fixed_Point_Int_Storage_Type other) const
{
    return m_value == Fixed_Point_Integer<F>::from_integer(other).m_value;
}

template <uint8_t F>
constexpr inline bool
Fixed_Point_Integer<F>::operator==(const double other) const
{
    return m_value == Fixed_Point_Integer<F>::from_double(other).m_value;
}

template <uint8_t X>
std::ostream& operator<<(std::ostream& os, const Fixed_Point_Integer<X>& fp)
{
    os << fp.get_value();
    return os;
}

template <uint8_t F>
constexpr log2_lookup_table_type Fixed_Point_Integer<F>::make_log2_table()
{
    log2_lookup_table_type table {};
    for (std::size_t i = 0; i < LOG2_LOOKUP_TABLE_SIZE; ++i)
    {
        double c     = 1.0L + (static_cast<double>(i) / LOG2_LOOKUP_TABLE_SIZE);
        double log_c = std::log2l(c);
        table[i]     = static_cast<Fixed_Point_Int_Storage_Type>(
            std::llround(log_c * (1LL << F)));
    }
    return table;
}

template <uint8_t F>
constexpr exp2_lookup_table_type Fixed_Point_Integer<F>::make_exp2_table()
{
    exp2_lookup_table_type table {};
    for (std::size_t i = 0; i < EXP2_LOOKUP_TABLE_SIZE; ++i)
    {
        double c     = static_cast<double>(i) / EXP2_LOOKUP_TABLE_SIZE;
        double exp_c = std::exp2l(c);
        table[i]     = static_cast<Fixed_Point_Int_Storage_Type>(
            std::llround(exp_c * (1LL << F)));
    }
    return table;
}

namespace Matrex
{
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
    Fixed_Point_Integer<F>
    ln1p_approximation(const Fixed_Point_Integer<F> input);

    template <uint8_t F>
    Fixed_Point_Integer<F>
    exponential_approximation(const Fixed_Point_Integer<F> input);

    template <uint8_t F>
    Fixed_Point_Integer<F> sqrt(const Fixed_Point_Integer<F> input);

    double sqrt(double x);

    template <uint8_t F>
    Fixed_Point_Integer<F> exp(const Fixed_Point_Integer<F> input);

    double exp(double x);

    // NOTE: Do not use a pade approximation, this method is exponentially more
    // accurate.
    template <uint8_t F>
    Fixed_Point_Integer<F> tanh(const Fixed_Point_Integer<F> input)
    {
        const bool is_positive = (input.get_value() >= 0);

        const Fixed_Point_Integer<F> exponent =
            is_positive ? (-input * Fixed_Point_Integer<F>::FP_TWO)
                        : (input * Fixed_Point_Integer<F>::FP_TWO);
        const Fixed_Point_Integer<F> pow_result =
            pow(Fixed_Point_Integer<F>::FP_NATURAL_E, exponent);

        Fixed_Point_Integer<F> numerator =
            (-pow_result) + Fixed_Point_Integer<F>::FP_ONE;
        Fixed_Point_Integer<F> denominator =
            pow_result + Fixed_Point_Integer<F>::FP_ONE;
        Fixed_Point_Integer<F> result = numerator / denominator;
        result                        = is_positive ? result : (-result);

        return result;
    }

    // A pow function for fixed-point integers that uses the identity:
    // base^exponent = exp2(log2(base) * exponent)
    template <uint8_t F>
    Fixed_Point_Integer<F> pow(const Fixed_Point_Integer<F> base,
                               const Fixed_Point_Integer<F> exponent)
    {
        if ((base == Fixed_Point_Integer<F>::from_integer(0))
            || (exponent == Fixed_Point_Integer<F>::from_integer(1)))
        {
            return base;
        }

        if (exponent == Fixed_Point_Integer<F>::from_integer(0))
        {
            return Fixed_Point_Integer<F>::from_integer(1);
        }

        return Matrex::exp2(Matrex::log2(base) * exponent);
    }

    template <uint8_t F>
    Fixed_Point_Integer<F> log2(const Fixed_Point_Integer<F> input)
    {
        MATREX_ASSERT(
            input > 0,
            "LOG2 ERROR: Input must be non-negative and non-zero, input is {}",
            input.to_double());

        // Factor the input into (2^e * m) where m is in [1, 2) where e is an
        // integer. This may look like a familaur factorization because it's the
        // same way that floating point numbers are represented.
        // We can determine e simply by knowing the position of the first high
        // bit in the fixed-point representation of the input and subtracting
        // off F. This is equivalent to floor(log2(input)) or the closest power
        // of two to the input without exceeding the input.
        const int8_t e = 31 - __builtin_clz(input.get_value()) - F;
        Fixed_Point_Int_Storage_Type m_raw;
        if (e >= 0)
        {
            MATREX_ASSERT(e < FIXED_POINT_BIT_WIDTH,
                          "LOG2 SHIFT ERROR: e is not in bounds, e is {}",
                          e);
            m_raw = input.get_value() >> e;
        }
        else
        {
            MATREX_ASSERT((-e) <= (FIXED_POINT_BIT_WIDTH - 2),
                          "LOG2 SHIFT ERROR: e is not in bounds, e is {}",
                          e);
            m_raw = input.get_value() << (-e);
        }

        const Fixed_Point_Integer<F> m =
            Fixed_Point_Integer<F>::from_value(m_raw);

        MATREX_ASSERT(
            (m.to_double() >= 1.0 && m.to_double() < 2.0),
            "LOG2 FACTORIZATION ERROR: m is not in the range [1, 2), m is {}",
            m.to_double());

        // Here we want to calculate the index in which log(c) is stored in the
        // lookup table that we will be using to calculate the value of c. By
        // design, we want m to be very close to c, this will make t a very
        // small number which is ideal for a Pade approximation around 0 of
        // ln(1+t). To calculate the index, we simply map the m we found which
        // is a number in the range [1, 2) to an index in the range [0, N) (N
        // being the number of entries in the lookup table) - this index is the
        // "bucket" from which m is contained. Since, we want c close to m, we
        // map the index we found in the range of [0, N) back to a number in the
        // range [1, 2) and use that as c - c is thus the left edge of the
        // bucket that m is contained in (given how we calculate it).
        constexpr Fixed_Point_Integer<F> log2_lookup_table_size_in_fp =
            Fixed_Point_Integer<F>::from_integer(
                static_cast<Fixed_Point_Int_Storage_Type>(
                    LOG2_LOOKUP_TABLE_SIZE));
        std::size_t index = ((m - Fixed_Point_Integer<F>::FP_ONE)
                             * log2_lookup_table_size_in_fp)
                                .get_integer();

        // Clamp index to be within bounds because of rounding.
        index = std::clamp(index,
                           static_cast<std::size_t>(0),
                           (LOG2_LOOKUP_TABLE_SIZE - 1));

        const Fixed_Point_Integer<F> c =
            Fixed_Point_Integer<F>::FP_ONE
            + (Fixed_Point_Integer<F>::from_integer(
                   static_cast<Fixed_Point_Int_Storage_Type>(index))
               / log2_lookup_table_size_in_fp);

        MATREX_ASSERT(
            (c.to_double() >= 1.0 && c.to_double() < 2.0),
            "LOG2 FACTORIZATION ERROR: c is not in the range [1, 2), c is {}",
            c.to_double());

        // t is the residual component of m where m is c * (1 + t). Note, that
        // we use (1 + t) instead of t because in computing t = m/c (from m =
        // ct), t is bounded between [0.5, 2) however, if we do t = ((m/c) - 1),
        // t's upper bound is 1/N (N being the size of the lookup table) which
        // is ideal for a Pade approximation around 0 - we know this because:
        //  t = (m/c) - 1 which is (m - c) / c
        //  and we know (m - c) must be less than (1/N) because m and c are in
        //  the same bucket of size (1/N) and c is the left edge of the bucket.
        //  Thus, t is strictly less than or equal to (1 / N).
        const Fixed_Point_Integer<F> t =
            (m / c) - Fixed_Point_Integer<F>::FP_ONE;

        MATREX_ASSERT(
            ((t.to_double()
              <= (1.0 / static_cast<double>(LOG2_LOOKUP_TABLE_SIZE)))
             && (t.to_double() >= 0.0)),
            "LOG2 FACTORIZATION ERROR: c is not in the range [1, 2), c is {}",
            c.to_double());

        // Returns log2(2^e * m) = e + log2(m) = e + log2(c) + log2(1+t) which
        // is the same as log2(input).
        return Fixed_Point_Integer<F>::from_integer(e)
             + Fixed_Point_Integer<F>::from_value(
                   Fixed_Point_Integer<F>::lookup_log2_table(index))
             + (Matrex::ln1p_approximation(t)
                / Fixed_Point_Integer<F>::FP_LN_2);
    }

    template <uint8_t F>
    Fixed_Point_Integer<F> exp2(const Fixed_Point_Integer<F> input)
    {
        // What we are doing here is taking the input decomposing it into its
        // integer and fractional parts.
        // 2^x = 2^(integer_part + fractional_part) = 2^integer_part *
        // 2^fractional_part
        // The integer part (2^integer_part) can be calculated simply by bit
        // shifting.
        // (NOTE: The integer part is not shifted here, only masked!)
        Fixed_Point_Integer<F> integer_part    = input.floor();
        Fixed_Point_Integer<F> fractional_part = input - integer_part;

        MATREX_ASSERT(fractional_part.to_double() >= 0.0
                          && fractional_part.to_double() < 1.0,
                      "EXP2 ERROR: Fractional part is not in the range [0, 1), "
                      "fractional part is {}",
                      fractional_part.to_double());

        MATREX_ASSERT((integer_part + fractional_part) == input,
                      "EXP2 ERROR: Integer part and fractional part do not sum "
                      "to input, integer part is {}, fractional part is {}, "
                      "the sum is {}, input is {}",
                      integer_part.to_double(),
                      fractional_part.to_double(),
                      (integer_part + fractional_part).to_double(),
                      input.to_double());

        // We can break the fractional part further:
        // 2^fractional_part = 2^((i/N) + r) = 2^(i/N) * 2^(r)
        // This is the same logic we applied in log2 to get a small enough
        // residual r to approximate 2^r with a Pade approximation around 0. The
        // fractional part f is a number in the range [0, 1). So to get a number
        // i in the range [0, N) we can simply floor the multiplication of the
        // fractional part by N (we floor as we don't want 2^(i/n) > 2^f). So:
        // f = (i/N) + r where i = floor(f * N)
        // r = f - (i/N) where i = floor(f * N)
        // Remembering we have to divide i by N to get it back to the correct
        // scale because i is in the range [0, N) but we want (i/N) to be in the
        // range [0, 1).
        const Fixed_Point_Integer<F> i =
            (fractional_part
             * Fixed_Point_Integer<F>::from_integer(
                 static_cast<Fixed_Point_Int_Storage_Type>(
                     EXP2_LOOKUP_TABLE_SIZE)))
                .floor();
        const Fixed_Point_Integer<F> r =
            fractional_part
            - (i
               / Fixed_Point_Integer<F>::from_integer(
                   static_cast<Fixed_Point_Int_Storage_Type>(
                       EXP2_LOOKUP_TABLE_SIZE)));

        // The clamped integer part of i becomes the index for the lookup table
        // and we perform the lookup.
        std::size_t index = i.get_integer();

        // Clamp index to be within bounds because of rounding.
        index = std::clamp(index,
                           static_cast<std::size_t>(0),
                           (EXP2_LOOKUP_TABLE_SIZE - 1));

        Fixed_Point_Integer<F> table_lookup_value =
            Fixed_Point_Integer<F>::from_value(
                Fixed_Point_Integer<F>::lookup_exp2_table(index));

        // For r: we rewrite 2^r as e^(r * ln(2)).
        Fixed_Point_Integer<F> exp2_r = Matrex::exponential_approximation(
            Fixed_Point_Integer<F>::FP_LN_2 * r);
        Fixed_Point_Integer<F> fractional_part_result =
            (table_lookup_value * exp2_r);

        // Now we calculate 2^(integer_part) for both positive and negative
        // integer_part which simply translates into a bit shift of the
        // fractional part.
        Fixed_Point_Int_Storage_Type shift  = integer_part.get_integer();
        Fixed_Point_Integer<F>       result = fractional_part_result;
        if (shift >= 0)
        {
            // Clamp the shift by what is safe to shift a signed integer left
            // by.
            shift = std::clamp(shift, 0, (FIXED_POINT_BIT_WIDTH - 2));
            // Guard against overflow by using fixed point integer
            // multiplication.
            Fixed_Point_Integer<F> shift_fixed =
                Fixed_Point_Integer<F>::from_integer((1 << shift));
            result = result * shift_fixed;
        }
        else
        {
            // We need to return a 0 for any shift bigger than the bit width of
            // our fixed point because shifting larger than that would be
            // undefined behavior.
            if ((-shift) >= FIXED_POINT_BIT_WIDTH)
            {
                return Fixed_Point_Integer<F>::from_value(0);
            }
            result = Fixed_Point_Integer<F>::from_value(result.get_value()
                                                        >> (-shift));
        }

        return result;
    }

    // 3/3 Pade Approximation of ln(1+t) around 0.
    template <uint8_t F>
    Fixed_Point_Integer<F>
    ln1p_approximation(const Fixed_Point_Integer<F> input)
    {
        const Fixed_Point_Integer<F> input_power_two = input * input;
        const Fixed_Point_Integer<F> input_power_three =
            input_power_two * input;

        const Fixed_Point_Integer<F> numerator =
            (input * Fixed_Point_Integer<F>::FP_SIXTY)
            + (input_power_two * Fixed_Point_Integer<F>::FP_SIXTY)
            + (input_power_three * 11);
        const Fixed_Point_Integer<F> denominator =
            (input * 90) + (input_power_two * 36) + (input_power_three * 3)
            + Fixed_Point_Integer<F>::FP_SIXTY;

        return numerator / denominator;
    }

    // 3/3 Pade Approximation of e^x around 0.
    template <uint8_t F>
    Fixed_Point_Integer<F>
    exponential_approximation(const Fixed_Point_Integer<F> input)
    {
        const Fixed_Point_Integer<F> input_power_two = input * input;
        const Fixed_Point_Integer<F> input_power_three =
            input_power_two * input;

        const Fixed_Point_Integer<F> numerator =
            (input * Fixed_Point_Integer<F>::FP_SIXTY)
            + (input_power_two * Fixed_Point_Integer<F>::FP_12)
            + input_power_three + Fixed_Point_Integer<F>::FP_120;
        const Fixed_Point_Integer<F> denominator =
            (input * -Fixed_Point_Integer<F>::FP_SIXTY)
            + (input_power_two * Fixed_Point_Integer<F>::FP_12)
            - input_power_three + Fixed_Point_Integer<F>::FP_120;

        return numerator / denominator;
    }

    template <uint8_t F>
    Fixed_Point_Integer<F> sqrt(const Fixed_Point_Integer<F> input)
    {
        return Matrex::pow(input, Fixed_Point_Integer<F>::from_double(0.5));
    }

    template <uint8_t F>
    Fixed_Point_Integer<F> exp(const Fixed_Point_Integer<F> input)
    {
        return Matrex::pow(Fixed_Point_Integer<F>::FP_NATURAL_E, input);
    }
} // namespace Matrex

template <uint8_t F>
constexpr Fixed_Point_Integer<F>
operator+(const Fixed_Point_Int_Storage_Type other,
          const Fixed_Point_Integer<F>       fp)
{
    return Fixed_Point_Integer<F>::from_integer(other) + fp;
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F>
operator-(const Fixed_Point_Int_Storage_Type other,
          const Fixed_Point_Integer<F>       fp)
{
    return Fixed_Point_Integer<F>::from_integer(other) - fp;
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F>
operator/(const Fixed_Point_Int_Storage_Type other,
          const Fixed_Point_Integer<F>       fp)
{
    return Fixed_Point_Integer<F>::from_integer(other) / fp;
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F>
operator*(const Fixed_Point_Int_Storage_Type other,
          const Fixed_Point_Integer<F>       fp)
{
    return Fixed_Point_Integer<F>::from_integer(other) * fp;
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F> operator+(const double                 other,
                                           const Fixed_Point_Integer<F> fp)
{
    return Fixed_Point_Integer<F>::from_double(other) + fp;
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F> operator-(const double                 other,
                                           const Fixed_Point_Integer<F> fp)
{
    return Fixed_Point_Integer<F>::from_double(other) - fp;
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F> operator/(const double                 other,
                                           const Fixed_Point_Integer<F> fp)
{
    return Fixed_Point_Integer<F>::from_double(other) / fp;
}

template <uint8_t F>
constexpr Fixed_Point_Integer<F> operator*(const double                 other,
                                           const Fixed_Point_Integer<F> fp)
{
    return Fixed_Point_Integer<F>::from_double(other) * fp;
}
