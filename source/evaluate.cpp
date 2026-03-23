#include "evaluate.hpp"

#include <cmath>

double Non_Linear_Response::value(double F) const {
  const double H = calculate_function_H(F);
  const double S = calculate_function_S(F);
  const double P = calculate_function_P(F);
  const double B = calculate_function_B(F);

  return (H * S * P * B);
}

double Non_Linear_Response::partial_derivative_h_plus(double F) const {
  const double G = calculate_function_G(F);
  const double S = calculate_function_S(F);
  const double P = calculate_function_P(F);
  const double B = calculate_function_B(F);

  return (G * S * P * B);
}

double Non_Linear_Response::partial_derivative_h_minus(double F) const {
  const double G = calculate_function_G(F);
  const double S = calculate_function_S(F);
  const double P = calculate_function_P(F);
  const double B = calculate_function_B(F);

  return ((1.0L - G) * S * P * B);
}

double Non_Linear_Response::partial_derivative_z(double F) const {
  const double u = calculate_u(F);

  const double H = calculate_function_H(F);
  const double P = calculate_function_P(F);
  const double B = calculate_function_B(F);

  const double Mu = calculate_function_M(u);

  return (H * (u - Mu) * P * B);
}

double Non_Linear_Response::partial_derivative_k(double F) const {
  const double u = calculate_u(F);

  const double Mu = calculate_function_M(u);
  const double G = calculate_function_G(F);
  const double H = calculate_function_H(F);
  const double S = calculate_function_S(F);
  const double P = calculate_function_P(F);
  const double B = calculate_function_B(F);
  const double P_plus = calculate_function_P_plus(F);
  const double P_minus = calculate_function_P_minus(F);
  const double Mgp = calculate_function_M(m_g_plus_parameter);
  const double Mgm = calculate_function_M(m_g_minus_parameter);
  const double B_plus = calculate_function_B_plus(F);
  const double B_minus = calculate_function_B_minus(F);

  const double Gt = NON_LINEAR_RESPONSE_T * G * (1.0L - G);

  const double dH_du = Gt * (m_h_plus_parameter - m_h_minus_parameter);

  const double dMu_du = u / Mu;
  const double dS_du = m_z_parameter + ((1.0L - m_z_parameter) * dMu_du);

  const double dPp_du =
      m_q_plus_parameter * u * std::pow(Mu, (m_q_plus_parameter - 2.0L));
  const double dPm_du =
      m_q_minus_parameter * u * std::pow(Mu, (m_q_minus_parameter - 2.0L));

  const double dP_du =
      (Gt * (P_plus - P_minus)) + (G * dPp_du) + ((1.0L - G) * dPm_du);

  const double vp = Mu / Mgp;
  const double vm = Mu / Mgm;

  const double dvp_du = u / (Mu * Mgp);
  const double dvm_du = u / (Mu * Mgm);

  const double dBp_du = (1.0L - (B_plus * B_plus)) * m_r_plus_parameter *
                        std::pow(vp, m_r_plus_parameter - 1.0L) * dvp_du;

  const double dBm_du = (1.0L - (B_minus * B_minus)) * m_r_minus_parameter *
                        std::pow(vm, m_r_minus_parameter - 1.0L) * dvm_du;

  const double dB_du =
      (Gt * (B_plus - B_minus)) + (G * dBp_du) + ((1.0L - G) * dBm_du);

  const double dY_du = (dH_du * S * P * B) + (H * dS_du * P * B) +
                       (H * S * dP_du * B) + (H * S * P * dB_du);

  return -dY_du;
}

double Non_Linear_Response::partial_derivative_q_plus(double F) const {
  const double u = calculate_u(F);

  const double G = calculate_function_G(F);
  const double H = calculate_function_H(F);
  const double S = calculate_function_S(F);
  const double B = calculate_function_B(F);

  const double Mu = calculate_function_M(u);
  const double P_plus = calculate_function_P_plus(F);

  return (H * S * (G * P_plus * std::log(Mu)) * B);
}

double Non_Linear_Response::partial_derivative_q_minus(double F) const {
  const double u = calculate_u(F);
  const double G = calculate_function_G(F);
  const double H = calculate_function_H(F);
  const double S = calculate_function_S(F);
  const double B = calculate_function_B(F);

  const double Mu = calculate_function_M(u);
  const double P_minus = calculate_function_P_minus(F);

  return (H * S * ((1.0 - G) * P_minus * std::log(Mu)) * B);
}

double Non_Linear_Response::partial_derivative_r_plus(double F) const {
  const double u = calculate_u(F);

  const double G = calculate_function_G(F);
  const double H = calculate_function_H(F);
  const double S = calculate_function_S(F);
  const double P = calculate_function_P(F);

  const double B_plus = calculate_function_B_plus(F);

  const double Mu = calculate_function_M(u);
  const double Mg = calculate_function_M(m_g_plus_parameter);

  const double v = Mu / Mg;
  const double w = std::pow(v, m_r_plus_parameter);

  const double term = (1.0L - (B_plus * B_plus)) * w * std::log(v);

  return (H * S * P * G * term);
}

double Non_Linear_Response::partial_derivative_r_minus(double F) const {
  const double u = calculate_u(F);

  const double G = calculate_function_G(F);
  const double H = calculate_function_H(F);
  const double S = calculate_function_S(F);
  const double P = calculate_function_P(F);

  const double B_minus = calculate_function_B_minus(F);

  const double Mu = calculate_function_M(u);
  const double Mg = calculate_function_M(m_g_minus_parameter);

  const double v = Mu / Mg;
  const double w = std::pow(v, m_r_minus_parameter);

  const double term = (1.0L - (B_minus * B_minus)) * w * std::log(v);

  return (H * S * P * ((1.0L - G) * term));
}

double Non_Linear_Response::partial_derivative_g_plus(double F) const {
  const double u = calculate_u(F);

  const double G = calculate_function_G(F);
  const double H = calculate_function_H(F);
  const double S = calculate_function_S(F);
  const double P = calculate_function_P(F);

  const double B_plus = calculate_function_B_plus(F);

  const double Mu = calculate_function_M(u);
  const double Mg = calculate_function_M(m_g_plus_parameter);

  const double v = Mu / Mg;

  const double term_one = -(m_g_plus_parameter * Mu) / (Mg * Mg * Mg);

  const double term_two = (1.0L - (B_plus * B_plus)) * m_r_plus_parameter *
                          std::pow(v, (m_r_plus_parameter - 1.0L));

  return (H * S * P * G * term_one * term_two);
}

double Non_Linear_Response::partial_derivative_g_minus(double F) const {
  const double u = calculate_u(F);

  const double G = calculate_function_G(F);
  const double H = calculate_function_H(F);
  const double S = calculate_function_S(F);
  const double P = calculate_function_P(F);

  const double B_minus = calculate_function_B_minus(F);

  const double Mu = calculate_function_M(u);
  const double Mg = calculate_function_M(m_g_minus_parameter);

  const double v = Mu / Mg;

  const double term_one = -(m_g_minus_parameter * Mu) / (Mg * Mg * Mg);

  const double term_two = (1.0L - (B_minus * B_minus)) * m_r_minus_parameter *
                          std::pow(v, (m_r_minus_parameter - 1.0L));

  return (H * S * P * (1.0L - G) * term_one * term_two);
}

double Non_Linear_Response::partial_derivative_u(double F) const {
  return -partial_derivative_k(F);
}

double Non_Linear_Response::calculate_u(double F) const {
  return (F - m_k_parameter);
}

double Non_Linear_Response::calculate_function_M(double x) const {
  const double result = std::sqrt((x * x) + NON_LINEAR_RESPONSE_EPSILON);
  return result;
}

double Non_Linear_Response::calculate_function_G(double F) const {
  const double u = calculate_u(F);
  const double sigmoid =
      1.0L / (1.0L + std::exp(-1 * u * NON_LINEAR_RESPONSE_T));
  return sigmoid;
}

double Non_Linear_Response::calculate_function_H(double F) const {
  const double first_term = calculate_function_G(F) * m_h_plus_parameter;
  const double second_term =
      (1 - calculate_function_G(F)) * m_h_minus_parameter;
  return (first_term + second_term);
}

double Non_Linear_Response::calculate_function_S(double F) const {
  const double u = calculate_u(F);
  const double first_term = m_z_parameter * u;
  const double second_term = (1 - m_z_parameter) * calculate_function_M(u);
  return (first_term + second_term);
}

double Non_Linear_Response::calculate_function_P_plus(double F) const {
  const double u = calculate_u(F);
  const double term = std::pow(calculate_function_M(u), m_q_plus_parameter);
  return term;
}

double Non_Linear_Response::calculate_function_P_minus(double F) const {
  const double u = calculate_u(F);
  const double term = std::pow(calculate_function_M(u), m_q_minus_parameter);
  return term;
}

double Non_Linear_Response::calculate_function_P(double F) const {
  const double first_term =
      calculate_function_G(F) * calculate_function_P_plus(F);
  const double second_term =
      (1 - calculate_function_G(F)) * calculate_function_P_minus(F);
  return (first_term + second_term);
}

double Non_Linear_Response::calculate_function_B_plus(double F) const {
  const double u = calculate_u(F);
  const double v =
      calculate_function_M(u) / calculate_function_M(m_g_plus_parameter);
  const double w = std::pow(v, m_r_plus_parameter);
  return std::tanh(w);
}

double Non_Linear_Response::calculate_function_B_minus(double F) const {
  const double u = calculate_u(F);
  const double v =
      calculate_function_M(u) / calculate_function_M(m_g_minus_parameter);
  const double w = std::pow(v, m_r_minus_parameter);
  return std::tanh(w);
}

double Non_Linear_Response::calculate_function_B(double F) const {
  const double first_term =
      calculate_function_G(F) * calculate_function_B_plus(F);
  const double second_term =
      (1 - calculate_function_G(F)) * calculate_function_B_minus(F);
  return (first_term + second_term);
}
