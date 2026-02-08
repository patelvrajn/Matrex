#include "tuner.hpp"

Tuner::Tuner(std::ostream& logging, std::ifstream dataset,
             std::ofstream& output)
    : m_log(logging), m_output(output) {
  m_dataset = parse_dataset_file(dataset);
}

Evaluation_Weights Tuner::tune() {
  // Evaluation_Weights first_moment = 0 * ;
  // Evaluation_Weights second_moment = 0 * ;

  for (uint64_t i = 0; i < TUNER_ITERATIONS; i++) {
    // Calculate gradient
    // Evaluation_Weights gradient = compute_gradient();

    // Gradient calculation

    // First moment calculation

    // Second moment calculation

    // Bias-corrected first moment calculation

    // Bias-corrected second moment calculation

    // Parameter update
  }
}

Dataset Tuner::parse_dataset_file(std::ifstream& dataset) {}

Evaluation_Weights Tuner::compute_gradient(Evaluation_Weights& weights) {}
