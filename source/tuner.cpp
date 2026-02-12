#include "tuner.hpp"

Tuner::Tuner(std::ostream& logging, std::ifstream& dataset,
             std::ofstream& output)
    : m_log(logging), m_output(output) {
  m_dataset = parse_dataset_file(dataset);
}

Evaluation_Weights<double> Tuner::tune() {
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

Dataset Tuner::parse_dataset_file(std::ifstream& dataset_file) {
  Dataset returned_dataset;

  m_log << "[INFO] Starting to parse dataset file." << std::endl;

  std::string line;
  while (std::getline(dataset_file, line)) {
    std::size_t last_space_pos = line.find_last_of(' ');
    if (last_space_pos != std::string::npos) {
      std::string fen = line.substr(0, last_space_pos);

      constexpr uint8_t MAX_SCORE_STRING_LENGTH = 3;
      double score = std::stod(line.substr(
          (last_space_pos + 2),
          MAX_SCORE_STRING_LENGTH));  // +2 to skip space and opening bracket

      m_log << "[INFO] Parsed (FEN, score) pair: (\"" << fen << "\", " << score
            << ")" << std::endl;

      returned_dataset.fens.push_back(fen);
      returned_dataset.scores.push_back(score);
    }
  }

  m_log << "[INFO] Finished parsing dataset file." << std::endl;

  return returned_dataset;
}

Evaluation_Weights<double> Tuner::compute_gradient(
    Evaluation_Weights<double>& weights) {}
