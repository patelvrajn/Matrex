#pragma once

#include <fstream>
#include <ostream>
#include <vector>

#include "evaluate.hpp"

constexpr uint64_t TUNER_NUMBER_OF_EPOCHS = 1000;
constexpr uint64_t TUNER_MINI_BATCH_SIZE = 16384;
constexpr double TUNER_VALIDATION_SPLIT = 0.2;

constexpr double TUNER_DECAY_FACTOR = 0.975;
constexpr double TUNER_LEARNING_RATE = 0.002;
constexpr double TUNER_NU = 0.999;
constexpr double TUNER_EPSILON = 1e-8;
constexpr double TUNER_HUBER_LOSS_GAMMA = 0.60;
constexpr double TUNER_SIGMOID_K = 0.00029;
constexpr double TUNER_REGULARIZATION_LAMBDA = 1e-9;
constexpr double TUNER_LOSS_IMPROVEMENT_CUTOFF = 1e-6;
constexpr double TUNER_WEIGHT_UPDATE_CUTOFF = 1e-4;
constexpr uint8_t TUNER_PATIENCE = 7;

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
  Tuner(std::ostream& logging, std::ifstream& dataset_file,
        std::ofstream& output);
  Evaluation_Weights<double> tune();

 private:
  std::ostream& m_log;
  Dataset m_training_dataset;
  Dataset m_validation_dataset;
  std::ofstream& m_output;

  double perturb(double mean);
  NLR_Parameters<double> random_nlr();

  Evaluation_Weights<double> init_weights();

  void parse_dataset_file(std::ifstream& dataset_file,
                          Dataset& training_dataset,
                          Dataset& validation_dataset);

  Dataset create_mini_batches(const Mini_Batch& aggregate_batch);

  Tuner_Eval_Params compute_eval_params(const Mini_Batch& mini_batch);

  Evaluation_Weights<double> compute_gradient(
      const Evaluation_Weights<double>& weights, const Mini_Batch& mini_batch);

  double compute_loss(const Dataset& d,
                      const Evaluation_Weights<double>& weights);

  double compute_max_data_loss(const Dataset& d);

  void print_element_as_cpp(std::ofstream& ofs, double scalar);
  void print_element_as_cpp(std::ofstream& ofs,
                            const NLR_Parameters<double>& nlr);

  template <typename T, std::size_t N>
  void print_multi_array_as_cpp(std::ofstream& ofs,
                                const multi_array<T, N>& arr);

  template <typename T, std::size_t N, std::size_t... Rest>
  void print_multi_array_as_cpp(std::ofstream& ofs,
                                const multi_array<T, N, Rest...>& arr);

  void print_header_file(const Evaluation_Weights<double>& weights);

  double huber_loss(double a) const;
  double derivative_huber_loss(double a) const;

  double sigmoid(double s) const;
  double derivative_sigmoid(double s) const;
};
