#pragma once

#include <fstream>
#include <ostream>
#include <vector>

#include "evaluate.hpp"

constexpr uint64_t TUNER_ITERATIONS = 10000;

constexpr double TUNER_DECAY_FACTOR = 0.975;
constexpr double TUNER_LEARNING_RATE = 0.002;
constexpr double TUNER_NU = 0.999;
constexpr double TUNER_EPSILON = 1e-8;

struct Dataset {
  std::vector<std::string> fens;
  std::vector<double> scores;
};

class Tuner {
 public:
  Tuner(std::ostream& logging, std::ifstream& dataset, std::ofstream& output);
  Evaluation_Weights<double> tune();

 private:
  std::ostream& m_log;
  Dataset m_dataset;
  std::ofstream& m_output;

  Dataset parse_dataset_file(std::ifstream& dataset_file);
  Evaluation_Weights<double> compute_gradient(
      Evaluation_Weights<double>& weights);
};
