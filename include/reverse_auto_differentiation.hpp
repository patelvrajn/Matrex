#pragma once

#include <memory>
#include <deque>

#include "globals.hpp"

// =============================================================================
// Reverse-Mode Automatic Differentiation
//
// Automatic differentiation is a set of techniques used to find the gradient of
// a function in a computer program. Reverse-mode specifically finds the
// gradient in O(m) time where m is the number of outputs of the function. It
// does this by propagating adjoint values from the output node(s) of the
// computational graph (which is represented as a tape or Wengert list) of the
// function to it's parent nodes, following a reverse topological ordering of
// graph traversal. An adjoint for any node u in the graph is by definition;
// u_bar = dv/du.
//
// Now, suppose an output function L(y, z) exists and in the expression's
// dependency graph we perform two operations:
//    node y = f(x)
//    node z = g(x)
// Then we can say that the adjoints for nodes x, y, and z are
//    x_bar = dL/dx
//    y_bar = dL/dy
//    z_bar = dL/dz
// Using the chain rule we can determine dL/dx:
//    dL/dx = (dL/dy) * (dy/dx) + (dL/dz) (dz/dx)
// So in terms of adjoints the chain rule says,
//    x_bar = y_bar * (dy/dx) + z_bar * (dz/dx)
// Notice that in general for any intermediates h_i between x and L(y, z), the
// update to adjoint x_bar is:
//    x_bar += h_i * (dh_i/dx)
// This is the general form of the rule for backpropagating adjoints and once
// the adjoints backpropagate to the start of the computational graph, the
// vector of adjoint values of the variables at the start of the computational
// graph are the partial derivative values with respect to that variable for the
// gradient.
//
// Our use case for reverse automatic differentiation in Matrex is for our NADAM
// tuner which needs to calculate the gradient of the evaluation function.
// =============================================================================

class AD_Node;

//==============================================================================
// Automatic Differentiation Adjoint Class
//
// This class is an abstraction of an adjoint update - it takes references to
// the parent nodes and when called as a functor performs the backpropagation.
// Notice, this class is just the parent class of the derived classes which
// specify the functor definition depending on the operation in the
// computational graph (e.g. pow, tanh, addition, multiplication, etc).
//==============================================================================
class AD_Adjoint
{
  public:

    AD_Adjoint();

    AD_Adjoint(const double value);

    AD_Adjoint(Optional_Reference<AD_Node> parent_node);

    AD_Adjoint(Optional_Reference<AD_Node> left_node,
               Optional_Reference<AD_Node> right_node);

    virtual void operator()(MAYBE_UNUSED std::initializer_list<double> args) {};

    double& value() { return m_value; } // Value of this node's adjoint.

    const double& value() const { return m_value; }

    auto& left_node() // Value of the left parent's adjoint.
    {
        MATREX_ASSERT(m_left_node.has_ref(),
                      "Accessed AD Adjoint's left node that has an optional "
                      "reference of no value.");
        return m_left_node.get_ref();
    }

    auto& right_node() // Value of the right parent's adjoint.
    {
        MATREX_ASSERT(m_right_node.has_ref(),
                      "Accessed AD Adjoint's right node that has an optional "
                      "reference of no value.");
        return m_right_node.get_ref();
    }

    void set_value(const double value);

    AD_Adjoint operator+=(const AD_Adjoint& other);
    AD_Adjoint operator-=(const AD_Adjoint& other);

  private:

    double                      m_value = 0.0;
    Optional_Reference<AD_Node> m_left_node;
    Optional_Reference<AD_Node> m_right_node;
};

class AD_Adjoint_No_Op : public AD_Adjoint
{
  public:

    using AD_Adjoint::AD_Adjoint;

    virtual void operator()(
        MAYBE_UNUSED const std::initializer_list<double> args = {}) override;
};

class AD_Adjoint_Addition : public AD_Adjoint
{
  public:

    using AD_Adjoint::AD_Adjoint;

    virtual void operator()(
        MAYBE_UNUSED const std::initializer_list<double> args = {}) override;
};

class AD_Adjoint_Subtraction : public AD_Adjoint
{
  public:

    using AD_Adjoint::AD_Adjoint;

    virtual void operator()(
        MAYBE_UNUSED const std::initializer_list<double> args = {}) override;
};

class AD_Adjoint_Multiplication : public AD_Adjoint
{
  public:

    using AD_Adjoint::AD_Adjoint;

    virtual void operator()(
        MAYBE_UNUSED const std::initializer_list<double> args = {}) override;
};

class AD_Adjoint_Division : public AD_Adjoint
{
  public:

    using AD_Adjoint::AD_Adjoint;

    virtual void operator()(
        MAYBE_UNUSED const std::initializer_list<double> args = {}) override;
};

class AD_Adjoint_Negation : public AD_Adjoint
{
  public:

    using AD_Adjoint::AD_Adjoint;

    virtual void operator()(
        MAYBE_UNUSED const std::initializer_list<double> args = {}) override;
};

class AD_Adjoint_Tanh : public AD_Adjoint
{
  public:

    using AD_Adjoint::AD_Adjoint;

    virtual void
    operator()(MAYBE_UNUSED const std::initializer_list<double> args) override;
};

class AD_Adjoint_Exp : public AD_Adjoint
{
  public:

    using AD_Adjoint::AD_Adjoint;

    virtual void
    operator()(MAYBE_UNUSED const std::initializer_list<double> args) override;
};

class AD_Adjoint_Sqrt : public AD_Adjoint
{
  public:

    using AD_Adjoint::AD_Adjoint;

    virtual void
    operator()(MAYBE_UNUSED const std::initializer_list<double> args) override;
};

class AD_Adjoint_Pow : public AD_Adjoint
{
  public:

    using AD_Adjoint::AD_Adjoint;

    virtual void
    operator()(MAYBE_UNUSED const std::initializer_list<double> args) override;
};

using AD_Adjoint_Pointer = std::unique_ptr<AD_Adjoint>;

//==============================================================================
// Automatic Differentiation Node Class
//
// This class is an abstraction of a computational graph's node which is an
// element of the AD Tape class.
//==============================================================================
class AD_Node
{
  public:

    AD_Node();

    AD_Node(const double value);

    AD_Node(const double value, AD_Adjoint_Pointer adjoint);

    AD_Node(const double       value,
            AD_Adjoint_Pointer adjoint,
            const std::size_t  weight_index);

    double& value();

    const double& value() const;

    auto& adjoint() { return (*m_adjoint); }

    const auto& adjoint() const { return (*m_adjoint); }

    const auto& weight_index() const { return m_weight_index; }

  private:

    double             m_value   = 0.0;
    AD_Adjoint_Pointer m_adjoint = nullptr;

    int64_t m_weight_index = -1;
};

//==============================================================================
// Automatic Differentiation Node Class
//
// This class is an abstraction of a computational graph.
//==============================================================================
class AD_Tape
{
  public:

    AD_Tape();

    std::reference_wrapper<AD_Node> push(AD_Node&& node);

    void clear();

    auto begin() { return m_tape.begin(); }

    auto end() { return m_tape.end(); }

    auto begin() const { return m_tape.begin(); }

    auto end() const { return m_tape.end(); }

  private:

    std::deque<AD_Node> m_tape;
};

//==============================================================================
// Automatic Differentiation Node Class
//
// This class is the data type for each evaluation weight when passed into the
// evaluator. Through operator and function overloading, it automatically fills
// the tape with the nodes that represent the computational graph of the
// evaluation function.
//==============================================================================
struct AD_Value
{
    mutable Optional_Reference<AD_Tape> tape;
    Optional_Reference<AD_Node>         node;

    double& value()
    {
        MATREX_ASSERT(
            node.has_ref(),
            "Tried to access a node's value via AD Value but it has no value.");

        return node.get_ref().value();
    }

    const double& value() const
    {
        MATREX_ASSERT(
            node.has_ref(),
            "Tried to access a node's value via AD Value but it has no value.");

        return node.get_ref().value();
    }

    static AD_Value constant(Optional_Reference<AD_Tape> tape, double value)
    {
        return {.tape = tape,
                .node = tape.get_ref().push(
                    AD_Node(value, std::make_unique<AD_Adjoint_No_Op>()))};
    }

    static AD_Value variable(Optional_Reference<AD_Tape> tape,
                             double                      value,
                             std::size_t                 weight_index)
    {
        return {.tape = tape,
                .node = tape.get_ref().push(
                    AD_Node(value,
                            std::make_unique<AD_Adjoint_No_Op>(),
                            weight_index))};
    }

    AD_Value operator+(const AD_Value& other) const
    {
        double result_value = value() + other.value();

        return {.tape = this->tape,
                .node = tape.get_ref().push(AD_Node(
                    result_value,
                    std::make_unique<AD_Adjoint_Addition>(node, other.node)))};
    }

    AD_Value operator-(const AD_Value& other) const
    {
        double result_value = value() - other.value();

        return {
            .tape = this->tape,
            .node = tape.get_ref().push(AD_Node(
                result_value,
                std::make_unique<AD_Adjoint_Subtraction>(node, other.node)))};
    }

    AD_Value operator/(const AD_Value& other) const
    {
        double result_value = value() / other.value();

        return {.tape = this->tape,
                .node = tape.get_ref().push(AD_Node(
                    result_value,
                    std::make_unique<AD_Adjoint_Division>(node, other.node)))};
    }

    AD_Value operator*(const AD_Value& other) const
    {
        double result_value = value() * other.value();

        return {.tape = this->tape,
                .node = tape.get_ref().push(AD_Node(
                    result_value,
                    std::make_unique<AD_Adjoint_Multiplication>(node,
                                                                other.node)))};
    }

    AD_Value operator-() const
    {
        double result_value = -value();

        return {.tape = this->tape,
                .node = tape.get_ref().push(
                    AD_Node(result_value,
                            std::make_unique<AD_Adjoint_Negation>(node)))};
    }

    AD_Value operator+=(const AD_Value& other)
    {
        (*this) = (*this) + other;

        return (*this);
    }

    AD_Value operator-=(const AD_Value& other)
    {
        (*this) = (*this) - other;

        return (*this);
    }

    AD_Value operator/=(const AD_Value& other)
    {
        (*this) = (*this) / other;

        return (*this);
    }

    AD_Value operator*=(const AD_Value& other)
    {
        (*this) = (*this) * other;

        return (*this);
    }

    template <typename T>
    AD_Value operator+(const T other) const
    {
        AD_Value c = AD_Value::constant(this->tape, other);

        return ((*this) + c);
    }

    template <typename T>
    AD_Value operator-(const T other) const
    {
        AD_Value c = AD_Value::constant(this->tape, other);

        return ((*this) - c);
    }

    template <typename T>
    AD_Value operator/(const T other) const
    {
        AD_Value c = AD_Value::constant(this->tape, other);

        return ((*this) / c);
    }

    template <typename T>
    AD_Value operator*(const T other) const
    {
        AD_Value c = AD_Value::constant(this->tape, other);

        return ((*this) * c);
    }

    template <typename T>
    AD_Value operator+=(const T other)
    {
        AD_Value c = AD_Value::constant(this->tape, other);

        (*this) = (*this) + c;

        return (*this);
    }

    template <typename T>
    AD_Value operator-=(const T other)
    {
        AD_Value c = AD_Value::constant(this->tape, other);

        (*this) = (*this) - c;

        return (*this);
    }

    template <typename T>
    AD_Value operator/=(const T other)
    {
        AD_Value c = AD_Value::constant(this->tape, other);

        (*this) = (*this) / c;

        return (*this);
    }

    template <typename T>
    AD_Value operator*=(const T other)
    {
        AD_Value c = AD_Value::constant(this->tape, other);

        (*this) = (*this) * c;

        return (*this);
    }

    auto operator<=>(const AD_Value& other) const
    {
        return value() <=> other.value();
    }

    template <typename T>
    auto operator<=>(T other) const
    {
        return value() <=> other;
    }

    bool operator==(const AD_Value& other) const
    {
        return value() == other.value();
    }

    template <typename T>
    bool operator==(T other) const
    {
        return value() == other;
    }
};

template <typename T>
constexpr AD_Value operator+(const T other, const AD_Value& value)
{
    return AD_Value::constant(value.tape, other) + value;
}

template <typename T>
constexpr AD_Value operator-(const T other, const AD_Value& value)
{
    return AD_Value::constant(value.tape, other) - value;
}

template <typename T>
constexpr AD_Value operator/(const T other, const AD_Value& value)
{
    return AD_Value::constant(value.tape, other) / value;
}

template <typename T>
constexpr AD_Value operator*(const T other, const AD_Value& value)
{
    return AD_Value::constant(value.tape, other) * value;
}
