#include <cmath>
#include "globals.hpp"

#include "reverse_auto_differentiation.hpp"

AD_Adjoint::AD_Adjoint() {}

AD_Adjoint::AD_Adjoint(double value) : m_value(value) {}

AD_Adjoint::AD_Adjoint(Optional_Reference<AD_Node> parent_node) :
    m_left_node(parent_node)
{
}

AD_Adjoint::AD_Adjoint(Optional_Reference<AD_Node> left_node,
                       Optional_Reference<AD_Node> right_node) :
    m_left_node(left_node), m_right_node(right_node)
{
}

void AD_Adjoint::set_value(double value) { m_value = value; }

AD_Adjoint AD_Adjoint::operator+=(const AD_Adjoint& other)
{
    this->m_value += other.m_value;
    return (*this);
}

AD_Adjoint AD_Adjoint::operator-=(const AD_Adjoint& other)
{
    this->m_value -= other.m_value;
    return (*this);
}

void AD_Adjoint_No_Op::operator()(
    MAYBE_UNUSED std::initializer_list<double> args)
{
}

void AD_Adjoint_Addition::operator()(
    MAYBE_UNUSED std::initializer_list<double> args)
{
    left_node().adjoint()  += value();
    right_node().adjoint() += value();
}

void AD_Adjoint_Subtraction::operator()(
    MAYBE_UNUSED std::initializer_list<double> args)
{
    left_node().adjoint()  += value();
    right_node().adjoint() -= value();
}

void AD_Adjoint_Multiplication::operator()(
    MAYBE_UNUSED std::initializer_list<double> args)
{
    left_node().adjoint()  += (value() * right_node().value());
    right_node().adjoint() += (value() * left_node().value());
}

void AD_Adjoint_Division::operator()(
    MAYBE_UNUSED std::initializer_list<double> args)
{
    left_node().adjoint()  += (value() / right_node().value());
    right_node().adjoint() -= (value() * left_node().value())
                            / (right_node().value() * right_node().value());
}

void AD_Adjoint_Negation::operator()(
    MAYBE_UNUSED std::initializer_list<double> args)
{
    left_node().adjoint() -= value();
}

void AD_Adjoint_Tanh::operator()(
    MAYBE_UNUSED std::initializer_list<double> args)
{
    MATREX_ASSERT((args.size() == 1),
                  "Automatic Differentation Adjoint tanh requires 1 argument; "
                  "(1) This node's value");

    const double& this_node_value = (args.begin())[0];

    left_node().adjoint() +=
        (value() * (1.0 - (this_node_value * this_node_value)));
}

void AD_Adjoint_Exp::operator()(MAYBE_UNUSED std::initializer_list<double> args)
{
    MATREX_ASSERT((args.size() == 1),
                  "Automatic Differentation Adjoint exp requires 1 argument; "
                  "(1) This node's value");

    const double& this_node_value = (args.begin())[0];

    left_node().adjoint() += (value() * this_node_value);
}

void AD_Adjoint_Sqrt::operator()(
    MAYBE_UNUSED std::initializer_list<double> args)
{
    MATREX_ASSERT((args.size() == 1),
                  "Automatic Differentation Adjoint sqrt requires 1 argument; "
                  "(1) This node's value");

    const double& this_node_value = (args.begin())[0];

    left_node().adjoint() += (value() / (2 * this_node_value));
}

void AD_Adjoint_Pow::operator()(MAYBE_UNUSED std::initializer_list<double> args)
{
    MATREX_ASSERT(
        (args.size() == 1),
        "Automatic Differentation Adjoint pow requires 1 argument; (1) This "
        "node's value");

    const double& this_node_value = (args.begin())[0];

    left_node().adjoint() += (value() * right_node().value() * this_node_value
                              / left_node().value());
    right_node().adjoint() +=
        (value() * this_node_value * std::log(left_node().value()));
}

AD_Node::AD_Node() : m_adjoint(nullptr) {}

AD_Node::AD_Node(double value) : m_value(value), m_adjoint(nullptr) {}

AD_Node::AD_Node(double value, AD_Adjoint_Pointer adjoint) :
    m_value(value), m_adjoint(std::move(adjoint))
{
}

AD_Node::AD_Node(double             value,
                 AD_Adjoint_Pointer adjoint,
                 std::size_t        weight_index) :
    m_value(value), m_adjoint(std::move(adjoint)), m_weight_index(weight_index)
{
}

double& AD_Node::value() { return m_value; }

const double& AD_Node::value() const { return m_value; }

AD_Tape::AD_Tape() {}

std::reference_wrapper<AD_Node> AD_Tape::push(AD_Node&& node)
{
    m_tape.push_back(std::move(node));
    return std::ref(m_tape.back());
}

void AD_Tape::clear() { m_tape.clear(); }
