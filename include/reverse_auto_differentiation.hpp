#include <memory>

#include "globals.hpp"

class AD_Adjoint
{
  public:

    AD_Adjoint();

    AD_Adjoint(double value);

    AD_Adjoint(double value, double& parent_node);

    AD_Adjoint(double value, double& left_node, double& right_node);

    virtual void
    operator()(MAYBE_UNUSED std::initializer_list<double> args) = 0;

    double& value() { return m_value; } // Value of this node's adjoint.

    auto& left_node() // Value of the left parent's adjoint.
    {
        return m_left_node;
    }

    auto& right_node() // Value of the right parent's adjoint.
    {
        return m_right_node;
    }

  private:

    double                                        m_value = 0.0;
    std::optional<std::reference_wrapper<double>> m_left_node;
    std::optional<std::reference_wrapper<double>> m_right_node;
};

class AD_Adjoint_No_Op : public AD_Adjoint
{
  public:

    using AD_Adjoint::AD_Adjoint;

    virtual void
    operator()(MAYBE_UNUSED std::initializer_list<double> args = {}) override;
};

class AD_Adjoint_Addition : public AD_Adjoint
{
  public:

    using AD_Adjoint::AD_Adjoint;

    virtual void
    operator()(MAYBE_UNUSED std::initializer_list<double> args = {}) override;
};

class AD_Adjoint_Subtraction : public AD_Adjoint
{
  public:

    using AD_Adjoint::AD_Adjoint;

    virtual void
    operator()(MAYBE_UNUSED std::initializer_list<double> args = {}) override;
};

class AD_Adjoint_Multiplication : public AD_Adjoint
{
  public:

    using AD_Adjoint::AD_Adjoint;

    virtual void
    operator()(MAYBE_UNUSED std::initializer_list<double> args) override;
};

class AD_Adjoint_Division : public AD_Adjoint
{
  public:

    using AD_Adjoint::AD_Adjoint;

    virtual void
    operator()(MAYBE_UNUSED std::initializer_list<double> args) override;
};

class AD_Adjoint_Tanh : public AD_Adjoint
{
  public:

    using AD_Adjoint::AD_Adjoint;

    virtual void
    operator()(MAYBE_UNUSED std::initializer_list<double> args) override;
};

class AD_Adjoint_Exp : public AD_Adjoint
{
  public:

    using AD_Adjoint::AD_Adjoint;

    virtual void
    operator()(MAYBE_UNUSED std::initializer_list<double> args) override;
};

class AD_Adjoint_Sqrt : public AD_Adjoint
{
  public:

    using AD_Adjoint::AD_Adjoint;

    virtual void
    operator()(MAYBE_UNUSED std::initializer_list<double> args) override;
};

class AD_Adjoint_Pow : public AD_Adjoint
{
  public:

    using AD_Adjoint::AD_Adjoint;

    virtual void
    operator()(MAYBE_UNUSED std::initializer_list<double> args) override;
};

using AD_Adjoint_Pointer = std::unique_ptr<AD_Adjoint>;

class AD_Node
{
  public:

    AD_Node();

    AD_Node(double value);

    AD_Node(double value, AD_Adjoint_Pointer adjoint);

    AD_Node(double value, AD_Adjoint_Pointer adjoint, double& weight);

    double value();

  private:

    double             m_value   = 0.0;
    AD_Adjoint_Pointer m_adjoint = nullptr;

    std::optional<std::reference_wrapper<double>> m_weight;
};

constexpr std::size_t AD_TAPE_RESERVE_SIZE = 32;

class AD_Tape
{
  public:

    AD_Tape();

    std::reference_wrapper<AD_Node> push(AD_Node&& node);

  private:

    std::vector<AD_Node> m_tape;
};

struct AD_Value
{
    std::reference_wrapper<AD_Tape> tape;
    std::reference_wrapper<AD_Node> node;

    double value() { return node.get().value(); }

    static AD_Value constant(AD_Tape& tape, double value)
    {
        return {tape, tape.push(AD_Node(value))};
    }

    static AD_Value variable(AD_Tape& tape, double value, double& weight)
    {
        return {
            tape,
            tape.push(
                AD_Node(value, std::make_unique<AD_Adjoint_No_Op>(), weight))};
    }
};
