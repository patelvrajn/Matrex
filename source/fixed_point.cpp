#include "fixed_point.hpp"

namespace Matrex
{
    double tanh(double x) { return std::tanh(x); }

    double pow(double base, double exponent)
    {
        return std::pow(base, exponent);
    }

    double sqrt(double x) { return std::sqrt(x); }

    double exp(double x) { return std::exp(x); }

    AD_Value tanh(AD_Value x)
    {
        const double result_value = std::tanh(x.value());

        return {.tape = x.tape,
                .node = x.tape.get_ref().push(
                    AD_Node(result_value,
                            std::make_unique<AD_Adjoint_Tanh>(x.node)))};
    }

    AD_Value pow(AD_Value base, AD_Value exponent)
    {
        const double result_value = std::pow(base.value(), exponent.value());

        return {
            .tape = base.tape,
            .node = base.tape.get_ref().push(AD_Node(
                result_value,
                std::make_unique<AD_Adjoint_Pow>(base.node, exponent.node)))};
    }

    AD_Value sqrt(AD_Value x)
    {
        const double result_value = std::sqrt(x.value());

        return {.tape = x.tape,
                .node = x.tape.get_ref().push(
                    AD_Node(result_value,
                            std::make_unique<AD_Adjoint_Sqrt>(x.node)))};
    }

    AD_Value exp(AD_Value x)
    {
        const double result_value = std::exp(x.value());

        return {.tape = x.tape,
                .node = x.tape.get_ref().push(
                    AD_Node(result_value,
                            std::make_unique<AD_Adjoint_Exp>(x.node)))};
    }
} // namespace Matrex
