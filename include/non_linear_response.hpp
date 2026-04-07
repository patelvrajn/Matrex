#pragma once

#include "fixed_point.hpp"

template <typename T>
struct NLR_Parameters {  // NLR = Non-Linear Response
  T h_plus;
  T h_minus;
  T z;
  T k;
  T q_plus;
  T q_minus;
  T r_plus;
  T r_minus;
  T g_plus;
  T g_minus;
};

#define NLR_ARRAY_FIELDS(arr, idx)                                           \
  (arr)[(idx)].h_plus, (arr)[(idx)].h_minus, (arr)[(idx)].z, (arr)[(idx)].k, \
      (arr)[(idx)].q_plus, (arr)[(idx)].q_minus, (arr)[(idx)].r_plus,        \
      (arr)[(idx)].r_minus, (arr)[(idx)].g_plus, (arr)[(idx)].g_minus

#define NLR_FIELDS(x)                                                         \
  (x).h_plus, (x).h_minus, (x).z, (x).k, (x).q_plus, (x).q_minus, (x).r_plus, \
      (x).r_minus, (x).g_plus, (x).g_minus

constexpr double NON_LINEAR_RESPONSE_EPSILON = 2e-5;
constexpr double NON_LINEAR_RESPONSE_T = 5e3;

template <typename T>
class Non_Linear_Response {
 public:
  Non_Linear_Response(NLR_Parameters<T> params);

  T value(T F) const;

  T partial_derivative_h_plus(T F) const;
  T partial_derivative_h_minus(T F) const;
  T partial_derivative_z(T F) const;
  T partial_derivative_k(T F) const;
  T partial_derivative_q_plus(T F) const;
  T partial_derivative_q_minus(T F) const;
  T partial_derivative_r_plus(T F) const;
  T partial_derivative_r_minus(T F) const;
  T partial_derivative_g_plus(T F) const;
  T partial_derivative_g_minus(T F) const;
  T partial_derivative_u(T F) const;

 private:
  T m_h_plus_parameter;
  T m_h_minus_parameter;
  T m_z_parameter;
  T m_k_parameter;
  T m_q_plus_parameter;
  T m_q_minus_parameter;
  T m_r_plus_parameter;
  T m_r_minus_parameter;
  T m_g_plus_parameter;
  T m_g_minus_parameter;

  T calculate_u(T F) const;

  T calculate_function_M(T x) const;
  T calculate_function_G(T F) const;
  T calculate_function_H(T F) const;
  T calculate_function_S(T F) const;
  T calculate_function_P_plus(T F) const;
  T calculate_function_P_minus(T F) const;
  T calculate_function_P(T F) const;
  T calculate_function_B_plus(T F) const;
  T calculate_function_B_minus(T F) const;
  T calculate_function_B(T F) const;
};

template <typename T>
Non_Linear_Response<T>::Non_Linear_Response(NLR_Parameters<T> params)
    : m_h_plus_parameter(params.h_plus),
      m_h_minus_parameter(params.h_minus),
      m_z_parameter(params.z),
      m_k_parameter(params.k),
      m_q_plus_parameter(params.q_plus),
      m_q_minus_parameter(params.q_minus),
      m_r_plus_parameter(params.r_plus),
      m_r_minus_parameter(params.r_minus),
      m_g_plus_parameter(params.g_plus),
      m_g_minus_parameter(params.g_minus) {}

template <typename T>
T Non_Linear_Response<T>::value(T F) const {
  const T H = calculate_function_H(F);
  const T S = calculate_function_S(F);
  const T P = calculate_function_P(F);
  const T B = calculate_function_B(F);

  return (H * S * P * B);
}

template <typename T>
T Non_Linear_Response<T>::partial_derivative_h_plus(T F) const {
  const T G = calculate_function_G(F);
  const T S = calculate_function_S(F);
  const T P = calculate_function_P(F);
  const T B = calculate_function_B(F);

  return (G * S * P * B);
}

template <typename T>
T Non_Linear_Response<T>::partial_derivative_h_minus(T F) const {
  const T G = calculate_function_G(F);
  const T S = calculate_function_S(F);
  const T P = calculate_function_P(F);
  const T B = calculate_function_B(F);

  return ((1.0L - G) * S * P * B);
}

template <typename T>
T Non_Linear_Response<T>::partial_derivative_z(T F) const {
  const T u = calculate_u(F);

  const T H = calculate_function_H(F);
  const T P = calculate_function_P(F);
  const T B = calculate_function_B(F);

  const T Mu = calculate_function_M(u);

  return (H * (u - Mu) * P * B);
}

template <typename T>
T Non_Linear_Response<T>::partial_derivative_k(T F) const {
  const T u = calculate_u(F);

  const T Mu = calculate_function_M(u);
  const T G = calculate_function_G(F);
  const T H = calculate_function_H(F);
  const T S = calculate_function_S(F);
  const T P = calculate_function_P(F);
  const T B = calculate_function_B(F);
  const T P_plus = calculate_function_P_plus(F);
  const T P_minus = calculate_function_P_minus(F);
  const T Mgp = calculate_function_M(m_g_plus_parameter);
  const T Mgm = calculate_function_M(m_g_minus_parameter);
  const T B_plus = calculate_function_B_plus(F);
  const T B_minus = calculate_function_B_minus(F);

  const T Gt = NON_LINEAR_RESPONSE_T * G * (1.0L - G);

  const T dH_du = Gt * (m_h_plus_parameter - m_h_minus_parameter);

  const T dMu_du = u / Mu;
  const T dS_du = m_z_parameter + ((1.0L - m_z_parameter) * dMu_du);

  const T dPp_du =
      m_q_plus_parameter * u * std::pow(Mu, (m_q_plus_parameter - 2.0L));
  const T dPm_du =
      m_q_minus_parameter * u * std::pow(Mu, (m_q_minus_parameter - 2.0L));

  const T dP_du =
      (Gt * (P_plus - P_minus)) + (G * dPp_du) + ((1.0L - G) * dPm_du);

  const T vp = Mu / Mgp;
  const T vm = Mu / Mgm;

  const T dvp_du = u / (Mu * Mgp);
  const T dvm_du = u / (Mu * Mgm);

  const T dBp_du = (1.0L - (B_plus * B_plus)) * m_r_plus_parameter *
                   std::pow(vp, m_r_plus_parameter - 1.0L) * dvp_du;

  const T dBm_du = (1.0L - (B_minus * B_minus)) * m_r_minus_parameter *
                   std::pow(vm, m_r_minus_parameter - 1.0L) * dvm_du;

  const T dB_du =
      (Gt * (B_plus - B_minus)) + (G * dBp_du) + ((1.0L - G) * dBm_du);

  const T dY_du = (dH_du * S * P * B) + (H * dS_du * P * B) +
                  (H * S * dP_du * B) + (H * S * P * dB_du);

  return -dY_du;
}

template <typename T>
T Non_Linear_Response<T>::partial_derivative_q_plus(T F) const {
  const T u = calculate_u(F);

  const T G = calculate_function_G(F);
  const T H = calculate_function_H(F);
  const T S = calculate_function_S(F);
  const T B = calculate_function_B(F);

  const T Mu = calculate_function_M(u);
  const T P_plus = calculate_function_P_plus(F);

  return (H * S * (G * P_plus * std::log(Mu)) * B);
}

template <typename T>
T Non_Linear_Response<T>::partial_derivative_q_minus(T F) const {
  const T u = calculate_u(F);
  const T G = calculate_function_G(F);
  const T H = calculate_function_H(F);
  const T S = calculate_function_S(F);
  const T B = calculate_function_B(F);

  const T Mu = calculate_function_M(u);
  const T P_minus = calculate_function_P_minus(F);

  return (H * S * ((1.0L - G) * P_minus * std::log(Mu)) * B);
}

template <typename T>
T Non_Linear_Response<T>::partial_derivative_r_plus(T F) const {
  const T u = calculate_u(F);

  const T G = calculate_function_G(F);
  const T H = calculate_function_H(F);
  const T S = calculate_function_S(F);
  const T P = calculate_function_P(F);

  const T B_plus = calculate_function_B_plus(F);

  const T Mu = calculate_function_M(u);
  const T Mg = calculate_function_M(m_g_plus_parameter);

  const T v = Mu / Mg;
  const T w = std::pow(v, m_r_plus_parameter);

  const T term = (1.0L - (B_plus * B_plus)) * w * std::log(v);

  return (H * S * P * G * term);
}

template <typename T>
T Non_Linear_Response<T>::partial_derivative_r_minus(T F) const {
  const T u = calculate_u(F);

  const T G = calculate_function_G(F);
  const T H = calculate_function_H(F);
  const T S = calculate_function_S(F);
  const T P = calculate_function_P(F);

  const T B_minus = calculate_function_B_minus(F);

  const T Mu = calculate_function_M(u);
  const T Mg = calculate_function_M(m_g_minus_parameter);

  const T v = Mu / Mg;
  const T w = std::pow(v, m_r_minus_parameter);

  const T term = (1.0L - (B_minus * B_minus)) * w * std::log(v);

  return (H * S * P * ((1.0L - G) * term));
}

template <typename T>
T Non_Linear_Response<T>::partial_derivative_g_plus(T F) const {
  const T u = calculate_u(F);

  const T G = calculate_function_G(F);
  const T H = calculate_function_H(F);
  const T S = calculate_function_S(F);
  const T P = calculate_function_P(F);

  const T B_plus = calculate_function_B_plus(F);

  const T Mu = calculate_function_M(u);
  const T Mg = calculate_function_M(m_g_plus_parameter);

  const T v = Mu / Mg;

  const T term_one = -(m_g_plus_parameter * Mu) / (Mg * Mg * Mg);

  const T term_two = (1.0L - (B_plus * B_plus)) * m_r_plus_parameter *
                     std::pow(v, (m_r_plus_parameter - 1.0L));

  return (H * S * P * G * term_one * term_two);
}

template <typename T>
T Non_Linear_Response<T>::partial_derivative_g_minus(T F) const {
  const T u = calculate_u(F);

  const T G = calculate_function_G(F);
  const T H = calculate_function_H(F);
  const T S = calculate_function_S(F);
  const T P = calculate_function_P(F);

  const T B_minus = calculate_function_B_minus(F);

  const T Mu = calculate_function_M(u);
  const T Mg = calculate_function_M(m_g_minus_parameter);

  const T v = Mu / Mg;

  const T term_one = -(m_g_minus_parameter * Mu) / (Mg * Mg * Mg);

  const T term_two = (1.0L - (B_minus * B_minus)) * m_r_minus_parameter *
                     std::pow(v, (m_r_minus_parameter - 1.0L));

  return (H * S * P * (1.0L - G) * term_one * term_two);
}

template <typename T>
T Non_Linear_Response<T>::partial_derivative_u(T F) const {
  return -partial_derivative_k(F);
}

template <typename T>
T Non_Linear_Response<T>::calculate_u(T F) const {
  return (F - m_k_parameter);
}

template <typename T>
T Non_Linear_Response<T>::calculate_function_M(T x) const {
  const T result = Matrex::sqrt((x * x) + NON_LINEAR_RESPONSE_EPSILON);
  return result;
}

template <typename T>
T Non_Linear_Response<T>::calculate_function_G(T F) const {
  const T u = calculate_u(F);

  // A conversative clamp such that the shifts used in calculating exp2()
  // doesn't produce undefined behavior. This does not result in changing the
  // partials since the exponent clamp is large enough that it's in the
  // saturating region of sigmoid - where the derivatives are close to zero.
  constexpr int32_t G_EXPONENT_CLAMP = 15;
  T exponent = (-1 * u * NON_LINEAR_RESPONSE_T);
  if (exponent >= G_EXPONENT_CLAMP) {
    if constexpr (std::is_same_v<T, double>) {
      return 0.0;
    } else {
      return T::from_integer(0);
    }
  } else if (exponent <= -G_EXPONENT_CLAMP) {
    if constexpr (std::is_same_v<T, double>) {
      return 1.0;
    } else {
      return T::from_integer(1);
    }
  }

  const T sigmoid = 1.0 / (Matrex::exp(exponent) + 1.0);
  return sigmoid;
}

template <typename T>
T Non_Linear_Response<T>::calculate_function_H(T F) const {
  const T first_term = calculate_function_G(F) * m_h_plus_parameter;
  const T second_term = (1 - calculate_function_G(F)) * m_h_minus_parameter;
  return (first_term + second_term);
}

template <typename T>
T Non_Linear_Response<T>::calculate_function_S(T F) const {
  const T u = calculate_u(F);
  const T first_term = m_z_parameter * u;
  const T second_term = (1 - m_z_parameter) * calculate_function_M(u);
  return (first_term + second_term);
}

template <typename T>
T Non_Linear_Response<T>::calculate_function_P_plus(T F) const {
  const T u = calculate_u(F);
  const T term = Matrex::pow(calculate_function_M(u), m_q_plus_parameter);
  return term;
}

template <typename T>
T Non_Linear_Response<T>::calculate_function_P_minus(T F) const {
  const T u = calculate_u(F);
  const T term = Matrex::pow(calculate_function_M(u), m_q_minus_parameter);
  return term;
}

template <typename T>
T Non_Linear_Response<T>::calculate_function_P(T F) const {
  const T first_term = calculate_function_G(F) * calculate_function_P_plus(F);
  const T second_term =
      (1 - calculate_function_G(F)) * calculate_function_P_minus(F);
  return (first_term + second_term);
}

template <typename T>
T Non_Linear_Response<T>::calculate_function_B_plus(T F) const {
  const T u = calculate_u(F);
  const T v =
      calculate_function_M(u) / calculate_function_M(m_g_plus_parameter);
  const T w = Matrex::pow(v, m_r_plus_parameter);
  return Matrex::tanh(w);
}

template <typename T>
T Non_Linear_Response<T>::calculate_function_B_minus(T F) const {
  const T u = calculate_u(F);
  const T v =
      calculate_function_M(u) / calculate_function_M(m_g_minus_parameter);
  const T w = Matrex::pow(v, m_r_minus_parameter);
  return Matrex::tanh(w);
}

template <typename T>
T Non_Linear_Response<T>::calculate_function_B(T F) const {
  const T first_term = calculate_function_G(F) * calculate_function_B_plus(F);
  const T second_term =
      (1 - calculate_function_G(F)) * calculate_function_B_minus(F);
  return (first_term + second_term);
}
