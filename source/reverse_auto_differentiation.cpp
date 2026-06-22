#include <cmath>

#include "reverse_auto_differentiation.hpp"

AD_Adjoint::AD_Adjoint(double value) : m_value(value) {}

AD_Adjoint::AD_Adjoint(double value, double& parent_node) :
    m_value(value), m_left_node(parent_node)
{
}

AD_Adjoint::AD_Adjoint(double value, double& left_node, double& right_node) :
    m_value(value), m_left_node(left_node), m_right_node(right_node)
{
}

void AD_Adjoint_Addition::operator()(
    MAYBE_UNUSED std::initializer_list<double> args)
{
    left_node().value()  += value();
    right_node().value() += value();
}

void AD_Adjoint_Subtraction::operator()(
    MAYBE_UNUSED std::initializer_list<double> args)
{
    left_node().value()  += value();
    right_node().value() -= value();
}

void AD_Adjoint_Multiplication::operator()(
    MAYBE_UNUSED std::initializer_list<double> args)
{
    MATREX_ASSERT(
        (args.size() == 2),
        "Automatic Differentation Adjoint Multiplication requires 2 arguments; "
        "(1) Left parent node's value (2) Right parent node's value.");

    const double& left_node_value  = (args.begin())[0];
    const double& right_node_value = (args.begin())[1];

    left_node().value()  += (value() * right_node_value);
    right_node().value() += (value() * left_node_value);
}

void AD_Adjoint_Division::operator()(
    MAYBE_UNUSED std::initializer_list<double> args)
{
    MATREX_ASSERT(
        (args.size() == 2),
        "Automatic Differentation Adjoint Division requires 2 arguments; (1) "
        "Left parent node's value (2) Right parent node's value.");

    const double& left_node_value  = (args.begin())[0];
    const double& right_node_value = (args.begin())[1];

    left_node().value() += (value() / right_node_value);
    right_node().value() -=
        (value() * left_node_value) / (right_node_value * right_node_value);
}

void AD_Adjoint_Tanh::operator()(
    MAYBE_UNUSED std::initializer_list<double> args)
{
    MATREX_ASSERT((args.size() == 1),
                  "Automatic Differentation Adjoint tanh requires 1 argument; "
                  "(1) This node's value");

    const double& this_node_value = (args.begin())[0];

    left_node().value() +=
        (value() * (1.0 - (this_node_value * this_node_value)));
}

void AD_Adjoint_Exp::operator()(MAYBE_UNUSED std::initializer_list<double> args)
{
    MATREX_ASSERT((args.size() == 1),
                  "Automatic Differentation Adjoint exp requires 1 argument; "
                  "(1) This node's value");

    const double& this_node_value = (args.begin())[0];

    left_node().value() += (value() * this_node_value);
}

void AD_Adjoint_Sqrt::operator()(
    MAYBE_UNUSED std::initializer_list<double> args)
{
    MATREX_ASSERT((args.size() == 1),
                  "Automatic Differentation Adjoint sqrt requires 1 argument; "
                  "(1) This node's value");

    const double& this_node_value = (args.begin())[0];

    left_node().value() += (value() / (2 * this_node_value));
}

void AD_Adjoint_Pow::operator()(MAYBE_UNUSED std::initializer_list<double> args)
{
    MATREX_ASSERT(
        (args.size() == 3),
        "Automatic Differentation Adjoint pow requires 3 arguments; (1) This "
        "node's value (2) Base node's value (3) Exponents node's value.");

    const double& this_node_value     = (args.begin())[0];
    const double& base_node_value     = (args.begin())[1];
    const double& exponent_node_value = (args.begin())[2];

    left_node().value() +=
        (value() * exponent_node_value * this_node_value / base_node_value);
    left_node().value() +=
        (value() * this_node_value * std::log(base_node_value));
}

AD_Node::AD_Node(double value, AD_Adjoint_Pointer adjoint) :
    m_value(value), m_adjoint(std::move(adjoint))
{
}

AD_Node::AD_Node(double value, AD_Adjoint_Pointer adjoint, double& weight) :
    m_value(value), m_adjoint(std::move(adjoint)), m_weight(weight)
{
}
