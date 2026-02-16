#pragma once

#include <fstream>
#include <ostream>
#include <vector>

#include "evaluate.hpp"

constexpr uint64_t TUNER_NUMBER_OF_EPOCHS = 1000;
constexpr uint64_t TUNER_MINI_BATCH_SIZE = 16384;

constexpr double TUNER_WEIGHTS_INITIALIZATION_VALUE = 1;
constexpr double TUNER_DECAY_FACTOR = 0.975;
constexpr double TUNER_LEARNING_RATE = 0.002;
constexpr double TUNER_NU = 0.999;
constexpr double TUNER_EPSILON = 1e-8;
constexpr double TUNER_HUBER_LOSS_GAMMA = 0.25;
constexpr double TUNER_SIGMOID_K = 0.00029;
constexpr double TUNER_REGULARIZATION_LAMBDA = 1e-4;

struct Mini_Batch {
  std::vector<std::string> fens;
  std::vector<double> scores;
};

struct Dataset {
  std::vector<Mini_Batch> mini_batches;
  std::size_t size;
};

struct Tuner_Eval_Params {
  std::vector<Chess_Board> boards;
  std::vector<Moves_Bitboard_Matrix> moving_side_matrices;
  std::vector<Moves_Bitboard_Matrix> opposing_side_matrices;
};

class Tuner {
 public:
  Tuner(std::ostream& logging, std::ifstream& dataset, std::ofstream& output);
  Evaluation_Weights<double> tune();

 private:
  std::ostream& m_log;
  Dataset m_dataset;
  std::ofstream& m_output;

  Evaluation_Weights<double> init_weights();

  Dataset parse_dataset_file(std::ifstream& dataset_file);

  Tuner_Eval_Params compute_eval_params(const Mini_Batch& mini_batch);

  Evaluation_Weights<double> compute_gradient(
      const Evaluation_Weights<double>& weights, const Mini_Batch& mini_batch);

  double compute_loss(const Evaluation_Weights<double>& weights);

  double huber_loss(double a) const;
  double derivative_huber_loss(double a) const;

  double sigmoid(double s) const;
  double derivative_sigmoid(double s) const;
};
