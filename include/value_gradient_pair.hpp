#include "evaluate.hpp"

template<typename T>
struct Value_Gradient_Pair
{
    T value;
    Evaluation_Weights<T> gradient;

    static constexpr Value_Gradient_Pair constant(T constant) {
        return {constant, Evaluation_Weights<T>()};
    }

    static constexpr Value_Gradient_Pair variable(T variable, const Evaluation_Weights<T>& basis) 
    {
        return {variable, basis};
    }

    constexpr Value_Gradient_Pair operator+(const Value_Gradient_Pair& other) const
    {
        return 
        {
            .value = this->value + other.value, 
            .gradient = this->gradient + other.gradient
        };
    }

    constexpr Value_Gradient_Pair operator-(const Value_Gradient_Pair& other) const
    {
        return 
        {
            .value = this->value - other.value, 
            .gradient = this->gradient - other.gradient
        };
    }

    constexpr Value_Gradient_Pair operator*(const Value_Gradient_Pair& other) const
    {
        return 
        {
            .value = this->value * other.value,

            // Product rule for derivatives. 
            .gradient = 
                (this->value * other.gradient) + (this->gradient * other.value)
        };
    }

    constexpr Value_Gradient_Pair operator/(const Value_Gradient_Pair& other) const
    {
        return 
        {
            .value = this->value / other.value,

            // Quotient rule for derivatives. 
            .gradient = 
                ((this->gradient * other.value) - (this->value * other.gradient)) / (other.value * other.value)
        };
    }

    constexpr Value_Gradient_Pair& operator+=(const Value_Gradient_Pair& other)
    {
        Value_Gradient_Pair<T> sum = (*this) + other;
        
        value = sum.value;
        gradient = sum.gradient;

        return *this;
    }

    constexpr Value_Gradient_Pair& operator-=(const Value_Gradient_Pair& other)
    {
        Value_Gradient_Pair<T> difference = (*this) - other;
        
        value = difference.value;
        gradient = difference.gradient;

        return *this;
    }
    
    constexpr Value_Gradient_Pair& operator/=(const Value_Gradient_Pair& other)
    {
        Value_Gradient_Pair<T> quotient = (*this) / other;
        
        value = quotient.value;
        gradient = quotient.gradient;

        return *this;
    }

    constexpr Value_Gradient_Pair& operator*=(const Value_Gradient_Pair& other)
    {
        Value_Gradient_Pair<T> product = (*this) * other;
        
        value = product.value;
        gradient = product.gradient;

        return *this;
    }

    constexpr Value_Gradient_Pair operator-() const
    {
        return 
        {
            .value = -this->value,
            .gradient = -this->gradient
        };
    }

    constexpr Value_Gradient_Pair operator+(const T other) const
    {
        return 
        {
            .value = this->value + other, 
            .gradient = this->gradient
        };
    }
    
    constexpr Value_Gradient_Pair operator-(const T other) const
    {
        return 
        {
            .value = this->value - other, 
            .gradient = this->gradient
        };
    }
    
    constexpr Value_Gradient_Pair operator/(const T other) const
    {
        return 
        {
            .value = this->value / other, 
            .gradient = this->gradient / other
        };
    }
    
    constexpr Value_Gradient_Pair operator*(const T other) const
    {
        return 
        {
            .value = this->value * other, 
            .gradient = this->gradient * other
        };
    }

    constexpr Value_Gradient_Pair& operator+=(const T other)
    {
        Value_Gradient_Pair<T> sum = (*this) + other;
        
        value = sum.value;
        gradient = sum.gradient;

        return *this;
    }
    
    constexpr Value_Gradient_Pair& operator-=(const T other)
    {
        Value_Gradient_Pair<T> difference = (*this) - other;
        
        value = difference.value;
        gradient = difference.gradient;

        return *this;
    }
    
    constexpr Value_Gradient_Pair& operator/=(const T other)
    {
        Value_Gradient_Pair<T> quotient = (*this) / other;
        
        value = quotient.value;
        gradient = quotient.gradient;

        return *this;
    }
    
    constexpr Value_Gradient_Pair& operator*=(const T other)
    {
        Value_Gradient_Pair<T> product = (*this) * other;
        
        value = product.value;
        gradient = product.gradient;

        return *this;
    }
};
