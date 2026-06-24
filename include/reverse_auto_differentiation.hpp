#include <memory>
#include <deque>

#include "globals.hpp"

class AD_Node;

class AD_Adjoint
{
  public:

    AD_Adjoint();

    AD_Adjoint(double value);

    AD_Adjoint(AD_Node& parent_node);

    AD_Adjoint(AD_Node& left_node, AD_Node& right_node);

    virtual void operator()(MAYBE_UNUSED std::initializer_list<double> args) {};

    double& value() { return m_value; } // Value of this node's adjoint.
    const double& value() const { return m_value; }

    auto& left_node() // Value of the left parent's adjoint.
    {
        MATREX_ASSERT(m_left_node.has_value(), "Accessed AD Adjoint's left node that has an optional reference of no value.");
        return m_left_node.value().get();
    }

    auto& right_node() // Value of the right parent's adjoint.
    {
        MATREX_ASSERT(m_right_node.has_value(), "Accessed AD Adjoint's right node that has an optional reference of no value.");
        return m_right_node.value().get();
    }

    void set_value(double value);

    AD_Adjoint operator+= (const AD_Adjoint& other);
    AD_Adjoint operator-= (const AD_Adjoint& other);

  private:

    double                                         m_value = 0.0;
    std::optional<std::reference_wrapper<AD_Node>> m_left_node;
    std::optional<std::reference_wrapper<AD_Node>> m_right_node;
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
    operator()(MAYBE_UNUSED std::initializer_list<double> args = {}) override;
};

class AD_Adjoint_Division : public AD_Adjoint
{
  public:

    using AD_Adjoint::AD_Adjoint;

    virtual void
    operator()(MAYBE_UNUSED std::initializer_list<double> args = {}) override;
};

class AD_Adjoint_Negation : public AD_Adjoint
{
  public:

    using AD_Adjoint::AD_Adjoint;

    virtual void
    operator()(MAYBE_UNUSED std::initializer_list<double> args = {}) override;
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

    AD_Node(double value, AD_Adjoint_Pointer adjoint, std::size_t weight_index);

    double& value();

    auto& adjoint()
    {
        return (*m_adjoint);
    }

    const auto& adjoint() const
    {
        return (*m_adjoint);
    }

    const auto& weight_index() const
    {
        return m_weight_index;
    }

  private:

    double             m_value   = 0.0;
    AD_Adjoint_Pointer m_adjoint = nullptr;

    int64_t m_weight_index = -1;
};

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

struct AD_Value
{
    std::optional<std::reference_wrapper<AD_Tape>> tape;
    std::optional<std::reference_wrapper<AD_Node>> node;

    double& value() 
    { 
        MATREX_ASSERT(node.has_value(), "Tried to access a node's value via AD Value but it has no value.");

        return node.value().get().value(); 
    }

    const double& value() const 
    {
        MATREX_ASSERT(node.has_value(), "Tried to access a node's value via AD Value but it has no value.");

        return node.value().get().value(); 
    }

    static AD_Value constant(std::optional<std::reference_wrapper<AD_Tape>> tape, double value)
    {
        return {.tape = tape, .node = tape.value().get().push(AD_Node(value, std::make_unique<AD_Adjoint_No_Op>()))};
    }

    static AD_Value variable(std::optional<std::reference_wrapper<AD_Tape>> tape, double value, std::size_t weight_index)
    {
        return {
            .tape = tape,
            .node = tape.value().get().push(
                AD_Node(value, std::make_unique<AD_Adjoint_No_Op>(), weight_index))};
    }

    AD_Value operator+(const AD_Value& other) const
    {
        double result_value = value() + other.value();

        return {
            .tape = this->tape,
            .node = tape.value().get().push(
                AD_Node(result_value, std::make_unique<AD_Adjoint_Addition>(node.value(), other.node.value())))};
    }

    AD_Value operator-(const AD_Value& other) const
    {
        double result_value = value() - other.value();

        return {
            .tape = this->tape,
            .node = tape.value().get().push(
                AD_Node(result_value, std::make_unique<AD_Adjoint_Subtraction>(node.value(), other.node.value())))};
    }

    AD_Value operator/(const AD_Value& other) const
    {
        double result_value = value() / other.value();

        return {
            .tape = this->tape,
            .node = tape.value().get().push(
                AD_Node(result_value, std::make_unique<AD_Adjoint_Division>(node.value(), other.node.value())))};
    }

    AD_Value operator*(const AD_Value& other) const
    {
        double result_value = value() * other.value();

        return {
            .tape = this->tape,
            .node = tape.value().get().push(
                AD_Node(result_value, std::make_unique<AD_Adjoint_Multiplication>(node.value(), other.node.value())))};
    }

    AD_Value operator-() const
    {
        double result_value = -value();

        return {
            .tape = this->tape,
            .node = tape.value().get().push(
                AD_Node(result_value, std::make_unique<AD_Adjoint_Negation>(node.value())))};
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

    template<typename T>
    AD_Value operator+(const T other) const 
    {
        AD_Value c = AD_Value::constant(this->tape, other);

        return ((*this) + c);
    }

    template<typename T>
    AD_Value operator-(const T other) const 
    {
        AD_Value c = AD_Value::constant(this->tape, other);

        return ((*this) - c);
    }

    template<typename T>
    AD_Value operator/(const T other) const 
    {
        AD_Value c = AD_Value::constant(this->tape, other);

        return ((*this) / c);
    }

    template<typename T>
    AD_Value operator*(const T other) const 
    {
        AD_Value c = AD_Value::constant(this->tape, other);

        return ((*this) * c);
    }

    template<typename T>
    AD_Value operator+=(const T other) 
    {
        AD_Value c = AD_Value::constant(this->tape, other);

        (*this) = (*this) + c;

        return (*this);
    }

    template<typename T>
    AD_Value operator-=(const T other) 
    {
        AD_Value c = AD_Value::constant(this->tape, other);

        (*this) = (*this) - c;

        return (*this);
    }

    template<typename T>
    AD_Value operator/=(const T other) 
    {
        AD_Value c = AD_Value::constant(this->tape, other);

        (*this) = (*this) / c;

        return (*this);
    }

    template<typename T>
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
