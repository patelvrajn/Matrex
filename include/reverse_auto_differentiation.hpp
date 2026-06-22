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

    AD_Node(double value, AD_Adjoint_Pointer adjoint);

    AD_Node(double value, AD_Adjoint_Pointer adjoint, double& weight);

  private:

    double             m_value = 0.0;
    AD_Adjoint_Pointer m_adjoint;

    std::optional<std::reference_wrapper<double>> m_weight;
};

constexpr std::size_t AD_TAPE_RESERVE_SIZE = 32;

class AD_Tape
{
  public:

    AD_Tape();

    void push(AD_Node&& node);

  private:

    std::vector<AD_Node> m_tape;
};
