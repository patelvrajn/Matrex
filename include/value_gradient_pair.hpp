#pragma once

#include "evaluate.hpp"
#include "fixed_point.hpp"

template <typename T>
struct Value_Gradient_Pair
{
    T                     value    = 0;
    Evaluation_Weights<T> gradient = Evaluation_Weights<T>();
    ;

    static constexpr Value_Gradient_Pair constant(T constant)
    {
        return {constant, Evaluation_Weights<T>()};
    }

    static constexpr Value_Gradient_Pair
    variable(T variable, const Evaluation_Weights<T>& basis)
    {
        return {variable, basis};
    }

    constexpr Value_Gradient_Pair
    operator+(const Value_Gradient_Pair& other) const
    {
        return {.value    = this->value + other.value,
                .gradient = this->gradient + other.gradient};
    }

    constexpr Value_Gradient_Pair
    operator-(const Value_Gradient_Pair& other) const
    {
        return {.value    = this->value - other.value,
                .gradient = this->gradient - other.gradient};
    }

    constexpr Value_Gradient_Pair
    operator*(const Value_Gradient_Pair& other) const
    {
        return {.value = this->value * other.value,

                // Product rule for derivatives.
                .gradient = (other.gradient * this->value)
                          + (this->gradient * other.value)};
    }

    constexpr Value_Gradient_Pair
    operator/(const Value_Gradient_Pair& other) const
    {
        return {.value = this->value / other.value,

                // Quotient rule for derivatives.
                .gradient = ((this->gradient * other.value)
                             - (other.gradient * this->value))
                          / (other.value * other.value)};
    }

    constexpr Value_Gradient_Pair& operator+=(const Value_Gradient_Pair& other)
    {
        Value_Gradient_Pair<T> sum = (*this) + other;

        value    = sum.value;
        gradient = sum.gradient;

        return *this;
    }

    constexpr Value_Gradient_Pair& operator-=(const Value_Gradient_Pair& other)
    {
        Value_Gradient_Pair<T> difference = (*this) - other;

        value    = difference.value;
        gradient = difference.gradient;

        return *this;
    }

    constexpr Value_Gradient_Pair& operator/=(const Value_Gradient_Pair& other)
    {
        Value_Gradient_Pair<T> quotient = (*this) / other;

        value    = quotient.value;
        gradient = quotient.gradient;

        return *this;
    }

    constexpr Value_Gradient_Pair& operator*=(const Value_Gradient_Pair& other)
    {
        Value_Gradient_Pair<T> product = (*this) * other;

        value    = product.value;
        gradient = product.gradient;

        return *this;
    }

    constexpr Value_Gradient_Pair operator-() const
    {
        return {.value = -this->value, .gradient = -this->gradient};
    }

    constexpr Value_Gradient_Pair operator+(const T other) const
    {
        return {.value = this->value + other, .gradient = this->gradient};
    }

    constexpr Value_Gradient_Pair operator-(const T other) const
    {
        return {.value = this->value - other, .gradient = this->gradient};
    }

    constexpr Value_Gradient_Pair operator/(const T other) const
    {
        return {.value    = this->value / other,
                .gradient = this->gradient / other};
    }

    constexpr Value_Gradient_Pair operator*(const T other) const
    {
        return {.value    = this->value * other,
                .gradient = this->gradient * other};
    }

    constexpr Value_Gradient_Pair& operator+=(const T other)
    {
        Value_Gradient_Pair<T> sum = (*this) + other;

        value    = sum.value;
        gradient = sum.gradient;

        return *this;
    }

    constexpr Value_Gradient_Pair& operator-=(const T other)
    {
        Value_Gradient_Pair<T> difference = (*this) - other;

        value    = difference.value;
        gradient = difference.gradient;

        return *this;
    }

    constexpr Value_Gradient_Pair& operator/=(const T other)
    {
        Value_Gradient_Pair<T> quotient = (*this) / other;

        value    = quotient.value;
        gradient = quotient.gradient;

        return *this;
    }

    constexpr Value_Gradient_Pair& operator*=(const T other)
    {
        Value_Gradient_Pair<T> product = (*this) * other;

        value    = product.value;
        gradient = product.gradient;

        return *this;
    }

    constexpr auto operator<=>(const Value_Gradient_Pair& other) const
    {
        return value <=> other.value;
    }

    constexpr auto operator<=>(T other) const { return value <=> other; }

    constexpr bool operator==(const Value_Gradient_Pair& other) const
    {
        return value == other.value;
    }

    constexpr bool operator==(T other) const { return value == other; }
};

template <typename L, typename T>
constexpr Value_Gradient_Pair<T> operator+(const L                       lhs,
                                           const Value_Gradient_Pair<T>& rhs)
{
    return rhs + static_cast<T>(lhs);
}

template <typename L, typename T>
constexpr Value_Gradient_Pair<T> operator-(const L                       lhs,
                                           const Value_Gradient_Pair<T>& rhs)
{
    return Value_Gradient_Pair<T>::constant(static_cast<T>(lhs)) - rhs;
}

template <typename L, typename T>
constexpr Value_Gradient_Pair<T> operator*(const L                       lhs,
                                           const Value_Gradient_Pair<T>& rhs)
{
    return rhs * static_cast<T>(lhs);
}

template <typename L, typename T>
constexpr Value_Gradient_Pair<T> operator/(const L                       lhs,
                                           const Value_Gradient_Pair<T>& rhs)
{
    return Value_Gradient_Pair<T>::constant(static_cast<T>(lhs)) / rhs;
}

namespace Matrex
{
    template <typename T>
    Value_Gradient_Pair<T> tanh(const Value_Gradient_Pair<T>& x)
    {
        const T tanh_value = std::tanh(x.value);
        const T dy_dx      = (1.0 - (tanh_value * tanh_value));

        return {.value = tanh_value, .gradient = x.gradient * dy_dx};
    }

    template <typename T>
    Value_Gradient_Pair<T> pow(const Value_Gradient_Pair<T>& base,
                               const Value_Gradient_Pair<T>& exponent)
    {
        const T pow_value = std::pow(base.value, exponent.value);
        const T dy_dx =
            exponent.value * std::pow(base.value, (exponent.value - 1.0));

        return {.value = pow_value, .gradient = base.gradient * dy_dx};
    }

    template <typename T>
    Value_Gradient_Pair<T> sqrt(const Value_Gradient_Pair<T> x)
    {
        const T sqrt_value = std::sqrt(x.value);
        const T dy_dx      = 1.0 / (2.0 * sqrt_value);

        return {.value = sqrt_value, .gradient = x.gradient * dy_dx};
    }

    template <typename T>
    Value_Gradient_Pair<T> exp(const Value_Gradient_Pair<T> x)
    {
        const T exp_value = std::exp(x.value);

        return {.value = exp_value, .gradient = x.gradient * exp_value};
    }
} // namespace Matrex
