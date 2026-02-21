#include "tuner.hpp"

#include <iomanip>
#include <random>

Tuner::Tuner(std::ostream& logging, std::ifstream& dataset,
             std::ofstream& output)
    : m_log(logging), m_output(output) {
  m_dataset = parse_dataset_file(dataset);

  constexpr uint8_t DOUBLE_STD_OUT_PRECISION = 8;

  m_log << std::setprecision(DOUBLE_STD_OUT_PRECISION);
  m_output << std::setprecision(DOUBLE_STD_OUT_PRECISION);
}

double Tuner::perturb(double mean) {
  std::random_device rd;
  std::mt19937_64 rng(rd());
  constexpr double PERTURBATION_STD_DEV_FACTOR = 0.20;
  std::normal_distribution<double> distribution(
      mean, (PERTURBATION_STD_DEV_FACTOR * mean));
  return distribution(rng);
}

NLR_Parameters<double> Tuner::random_nlr() {
  return {.h_plus = perturb(30.0L),
          .h_minus = perturb(30.0L),
          .z = perturb(1.0L),
          .k = perturb(1.0L),
          .q_plus = perturb(0.25L),
          .q_minus = perturb(0.25L),
          .r_plus = perturb(-1.0L),
          .r_minus = perturb(-1.0L),
          .g_plus = perturb(1.0L),
          .g_minus = perturb(1.0L)};
}

Evaluation_Weights<double> Tuner::init_weights() {
  Evaluation_Weights<double> weights;

  weights.material_NLR_parameters = {random_nlr(), random_nlr(), random_nlr(),
                                     random_nlr(), random_nlr()};
  weights.material = {perturb(100.0L), perturb(300.0L), perturb(350.0L),
                      perturb(500.0L), perturb(900.0L)};

  weights.piece_mobility_NLR_parameters = {random_nlr(), random_nlr(),
                                           random_nlr(), random_nlr(),
                                           random_nlr(), random_nlr()};
  weights.diagonal_mobility = perturb(30.0L);
  weights.orthogonal_mobility = perturb(24.0L);
  weights.knight_movement_mobility = perturb(40.0L);
  weights.multi_movement_mobility = perturb(150.0L);
  weights.backwards_movement_mobility = perturb(25.0L);

  return weights;
}

Evaluation_Weights<double> Tuner::tune() {
  Evaluation_Weights<double> weights = init_weights();

  m_log << "[INFO] Initial weights: " << weights << std::endl;

  Evaluation_Weights<double> best_weights = weights;
  Evaluation_Weights<double> first_moment;
  Evaluation_Weights<double> second_moment;

  const std::size_t num_of_mini_batches = m_dataset.mini_batches.size();
  const double max_data_loss = compute_max_data_loss();

  m_log << "[INFO] There is " << num_of_mini_batches << " batches in 1 epoch."
        << std::endl;
  m_log << "[INFO] Maximum data loss for this dataset is: " << max_data_loss
        << std::endl;

  uint64_t t = 1;  // NADAM's timestep
  double previous_epoch_loss = compute_loss(best_weights);
  uint8_t epoch_patience_count = 0;

  m_log << "[INFO] Initial loss is " << previous_epoch_loss << std::endl;

  for (uint64_t epoch = 1; epoch <= TUNER_NUMBER_OF_EPOCHS; epoch++) {
    double weight_update_magnitude_average = 0;

    for (const Mini_Batch& mini_batch : m_dataset.mini_batches) {
      // Calculate gradient
      Evaluation_Weights<double> gradient =
          compute_gradient(weights, mini_batch);

      // First moment calculation
      first_moment = (first_moment * TUNER_DECAY_FACTOR) +
                     (gradient * (1.0L - TUNER_DECAY_FACTOR));

      // Second moment calculation
      second_moment = (second_moment * TUNER_NU) +
                      ((gradient * gradient) * (1.0L - TUNER_NU));

      // Bias-corrected first moment calculation
      const Evaluation_Weights<double> first_moment_corrected =
          ((first_moment * TUNER_DECAY_FACTOR) /
           (1.0L - std::pow(TUNER_DECAY_FACTOR, (t + 1)))) +
          ((gradient * (1.0L - TUNER_DECAY_FACTOR)) /
           (1.0L - std::pow(TUNER_DECAY_FACTOR, t)));

      // Bias-corrected second moment calculation
      const Evaluation_Weights<double> second_moment_corrected =
          (second_moment * TUNER_NU) / (1.0L - std::pow(TUNER_NU, t));

      const Evaluation_Weights<double> weight_update =
          ((TUNER_LEARNING_RATE /
            (second_moment_corrected + TUNER_EPSILON).sqrt()) *
           first_moment_corrected);
      const double weight_update_magnitude = weight_update.magnitude();
      weight_update_magnitude_average += weight_update_magnitude;

      m_log << "[INFO] Weight update with magnitude " << weight_update_magnitude
            << " at timestep " << t << "; " << weight_update << std::endl;

      // Parameter update
      weights = weights - weight_update;

      m_log << "[INFO] Weights at timestep " << t << "; " << weights
            << std::endl;

      t++;
    }

    weight_update_magnitude_average =
        weight_update_magnitude_average / num_of_mini_batches;

    // Compute this epoch's loss and loss improvment.
    const double loss = compute_loss(weights);
    const double loss_improvement = previous_epoch_loss - loss;

    m_log << "[INFO] Epoch " << epoch << ": Loss = " << loss
          << "; Loss Improvement = " << loss_improvement
          << "; Weight Update Average = " << weight_update_magnitude_average
          << std::endl;

    if ((loss_improvement < TUNER_LOSS_IMPROVEMENT_CUTOFF) &&
        (weight_update_magnitude_average <
         TUNER_WEIGHT_UPDATE_CUTOFF)) {  // Converged!
      epoch_patience_count++;
      if (epoch_patience_count == TUNER_PATIENCE) {
        break;
      }
    } else {  // Not converged.
      if (loss_improvement > 0) {
        best_weights = weights;
      }
      epoch_patience_count = 0;
    }

    m_log << "[INFO] Epoch patience count = "
          << static_cast<uint64_t>(epoch_patience_count) << std::endl;

    previous_epoch_loss = loss;
  }

  print_header_file(best_weights);

  return best_weights;
}

Dataset Tuner::parse_dataset_file(std::ifstream& dataset_file) {
  Mini_Batch aggregate_batch;

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

      // m_log << "[INFO] Parsed (FEN, score) pair: (\"" << fen << "\", " <<
      // score
      //       << ")" << std::endl;

      aggregate_batch.fens.push_back(fen);
      aggregate_batch.scores.push_back(score);
    }
  }

  m_log << "[INFO] Finished parsing dataset file of "
        << aggregate_batch.fens.size() << " entries." << std::endl;

  Dataset returned_dataset;

  // Now that we have the size of the entire dataset, split the aggregate batch
  // into mini-batches of size at most TUNER_MINI_BATCH_SIZE.
  for (std::size_t i = 0; i < aggregate_batch.fens.size();
       i += TUNER_MINI_BATCH_SIZE) {
    Mini_Batch mini_batch;

    for (std::size_t j = i;
         (j < (i + TUNER_MINI_BATCH_SIZE) && j < aggregate_batch.fens.size());
         j++) {
      mini_batch.fens.push_back(aggregate_batch.fens[j]);
      mini_batch.scores.push_back(aggregate_batch.scores[j]);
    }

    returned_dataset.mini_batches.push_back(mini_batch);
  }

  returned_dataset.size = aggregate_batch.fens.size();

  return returned_dataset;
}

Tuner_Eval_Params Tuner::compute_eval_params(const Mini_Batch& mini_batch) {
  Tuner_Eval_Params return_value;
  return_value.boards.reserve(mini_batch.fens.size());
  return_value.moving_side_matrices.reserve(mini_batch.fens.size());
  return_value.opposing_side_matrices.reserve(mini_batch.fens.size());

  for (std::size_t i = 0; i < mini_batch.fens.size(); i++) {
    Chess_Board cb;
    cb.set_from_fen(mini_batch.fens[i]);

    const PIECE_COLOR moving_side = cb.get_side_to_move();
    Chess_Move_List moving_side_moves_list;
    Moves_Bitboard_Matrix moving_side_matrix;
    Move_Generator mg_moving_side(cb);
    mg_moving_side.generate_all_moves<MOVE_GENERATION_TYPE::ALL>(
        moving_side, moving_side_moves_list, moving_side_matrix);

    const PIECE_COLOR opposing_side =
        (PIECE_COLOR)((~cb.get_side_to_move()) & 0x1);
    Chess_Move_List opposing_side_moves_list;
    Moves_Bitboard_Matrix opposing_side_matrix;
    Move_Generator mg_opposing_side(cb);
    mg_opposing_side.generate_all_moves<MOVE_GENERATION_TYPE::ALL>(
        opposing_side, opposing_side_moves_list, opposing_side_matrix);

    return_value.boards.push_back(cb);
    return_value.moving_side_matrices.push_back(moving_side_matrix);
    return_value.opposing_side_matrices.push_back(opposing_side_matrix);
  }

  return return_value;
}

Evaluation_Weights<double> Tuner::compute_gradient(
    const Evaluation_Weights<double>& weights, const Mini_Batch& mini_batch) {
  Evaluation_Weights<double> gradient;

  std::size_t N = mini_batch.fens.size();

  Tuner_Eval_Params eval_params = compute_eval_params(mini_batch);

  for (std::size_t i = 0; i < N; i++) {
    Evaluator e(weights, eval_params.boards[i],
                eval_params.moving_side_matrices[i],
                eval_params.opposing_side_matrices[i]);

    const double sign =
        (eval_params.boards[i].get_side_to_move() == PIECE_COLOR::WHITE)
            ? 1.0L
            : -1.0L;
    const double evaluation = e.evaluate_template_typed();
    const double evaluation_white =
        sign * evaluation;  // Convert side-to-move's evaluation to white's
                            // perspective.
    const double target_evaluation = mini_batch.scores[i];
    const double error = target_evaluation - sigmoid(evaluation_white);
    const double huber_loss_derivative = derivative_huber_loss(error);
    const double sigmoid_derivative = derivative_sigmoid(evaluation_white);
    const Evaluation_Weights<double> evaluation_deriative =
        (e.derivative_evaluate() * sign);

    gradient = gradient + (evaluation_deriative *
                           (huber_loss_derivative * sigmoid_derivative));
  }

  gradient = gradient / static_cast<double>(N);

  m_log << "[INFO] Data-based gradient is; " << gradient << std::endl;

  const Evaluation_Weights<double> regularization =
      (weights * (2.0L * TUNER_REGULARIZATION_LAMBDA));

  m_log << "[INFO] Regularization-based gradient is; " << regularization
        << std::endl;

  gradient = gradient + regularization;

  return gradient;
}

double Tuner::compute_loss(const Evaluation_Weights<double>& weights) {
  double loss = 0.0L;
  const std::size_t N = m_dataset.size;

  for (const Mini_Batch& mini_batch : m_dataset.mini_batches) {
    Tuner_Eval_Params eval_params = compute_eval_params(mini_batch);

    for (std::size_t i = 0; i < mini_batch.fens.size(); i++) {
      Evaluator e(weights, eval_params.boards[i],
                  eval_params.moving_side_matrices[i],
                  eval_params.opposing_side_matrices[i]);

      const double sign =
          (eval_params.boards[i].get_side_to_move() == PIECE_COLOR::WHITE)
              ? 1.0L
              : -1.0L;
      const double evaluation = e.evaluate_template_typed();
      const double evaluation_white =
          sign * evaluation;  // Convert side-to-move's evaluation to white's
                              // perspective.
      const double target_evaluation = mini_batch.scores[i];
      const double error = target_evaluation - sigmoid(evaluation_white);
      loss += huber_loss(error);
    }
  }

  loss = loss / static_cast<double>(N);

  m_log << "[INFO] Data-based loss is " << loss << std::endl;

  double regularization = 0.0L;

  for (std::size_t j = 0; j < weights.get_size(); j++) {
    regularization += (weights[j] * weights[j]);
  }

  regularization *= TUNER_REGULARIZATION_LAMBDA;

  m_log << "[INFO] Regularization-based loss is " << regularization
        << std::endl;

  loss += regularization;

  return loss;
}

double Tuner::compute_max_data_loss() {
  std::size_t num_of_decisive_games = 0;
  std::size_t num_of_draws = 0;

  for (const Mini_Batch& mini_batch : m_dataset.mini_batches) {
    for (std::size_t i = 0; i < mini_batch.fens.size(); i++) {
      if (mini_batch.scores[i] != 0.5L) {
        num_of_decisive_games++;
      } else {
        num_of_draws++;
      }
    }
  }

  const double loss_on_decisive = huber_loss(1.0L);
  const double loss_on_draw = huber_loss(0.5L);

  const double max_data_loss = ((num_of_decisive_games * loss_on_decisive) +
                                (num_of_draws * loss_on_draw)) /
                               m_dataset.size;

  return max_data_loss;
}

void Tuner::print_element_as_cpp(std::ofstream& ofs, double scalar) {
  ofs << scalar;
}

void Tuner::print_element_as_cpp(std::ofstream& ofs,
                                 const NLR_Parameters<double>& nlr) {
  ofs << "NLR_Parameters<double>{" << ".h_plus = " << nlr.h_plus
      << ", .h_minus = " << nlr.h_minus << ", .z = " << nlr.z
      << ", .k = " << nlr.k << ", .q_plus = " << nlr.q_plus
      << ", .q_minus = " << nlr.q_minus << ", .r_plus = " << nlr.r_plus
      << ", .r_minus = " << nlr.r_minus << ", .g_plus = " << nlr.g_plus
      << ", .g_minus = " << nlr.g_minus << "}";
}

template <typename T, std::size_t N>
void Tuner::print_multi_array_as_cpp(std::ofstream& ofs,
                                     const multi_array<T, N>& arr) {
  ofs << "{";
  for (std::size_t i = 0; i < N; i++) {
    print_element_as_cpp(ofs, arr[i]);
    if (i != (N - 1)) {
      ofs << ", ";
    }
  }
  ofs << "}";
}

template <typename T, std::size_t N, std::size_t... Rest>
void Tuner::print_multi_array_as_cpp(std::ofstream& ofs,
                                     const multi_array<T, N, Rest...>& arr) {
  ofs << "{";
  for (std::size_t i = 0; i < N; i++) {
    print_multi_array_as_cpp(ofs, arr[i]);
    if (i != (N - 1)) {
      ofs << ", ";
    }
  }
  ofs << "}";
}

void Tuner::print_header_file(const Evaluation_Weights<double>& weights) {
  // Includes
  m_output << "#pragma once" << std::endl << std::endl;
  m_output << "#include \"evaluate.hpp\"" << std::endl;
  m_output << "#include \"globals.hpp\"" << std::endl << std::endl;

  // Material
  m_output
      << "constexpr multi_array<NLR_Parameters<double>, "
         "(NUM_OF_UNIQUE_PIECES_PER_PLAYER - 1)> TUNED_MATERIAL_NLR_WEIGHTS = ";
  print_multi_array_as_cpp(m_output, weights.material_NLR_parameters);
  m_output << ";" << std::endl;
  m_output << "constexpr multi_array<double, (NUM_OF_UNIQUE_PIECES_PER_PLAYER "
              "- 1)> TUNED_MATERIAL_WEIGHTS = ";
  print_multi_array_as_cpp(m_output, weights.material);
  m_output << ";" << std::endl;

  // Mobility
  m_output
      << "constexpr multi_array<NLR_Parameters<double>, "
         "NUM_OF_UNIQUE_PIECES_PER_PLAYER> TUNED_PIECE_MOBILITY_NLR_WEIGHTS = ";
  print_multi_array_as_cpp(m_output, weights.piece_mobility_NLR_parameters);
  m_output << ";" << std::endl;
  m_output << "constexpr double TUNED_DIAGONAL_MOBILITY_WEIGHT = "
           << weights.diagonal_mobility << ";" << std::endl;
  m_output << "constexpr double TUNED_ORTHOGONAL_MOBILITY_WEIGHT = "
           << weights.orthogonal_mobility << ";" << std::endl;
  m_output << "constexpr double TUNED_KNIGHT_MOVEMENT_MOBILITY_WEIGHT = "
           << weights.knight_movement_mobility << ";" << std::endl;
  m_output << "constexpr double TUNED_MULTI_MOVEMENT_MOBILITY_WEIGHT = "
           << weights.multi_movement_mobility << ";" << std::endl;
  m_output << "constexpr double TUNED_BACKWARDS_MOVEMENT_MOBILITY_WEIGHT = "
           << weights.backwards_movement_mobility << ";" << std::endl;

  // Weights
  m_output
      << "const Evaluation_Weights<double> "
         "TUNED_EVALUATION_WEIGHTS(TUNED_MATERIAL_NLR_WEIGHTS, "
         "TUNED_MATERIAL_WEIGHTS, TUNED_PIECE_MOBILITY_NLR_WEIGHTS, "
         "TUNED_DIAGONAL_MOBILITY_WEIGHT, TUNED_ORTHOGONAL_MOBILITY_WEIGHT, "
         "TUNED_KNIGHT_MOVEMENT_MOBILITY_WEIGHT, "
         "TUNED_MULTI_MOVEMENT_MOBILITY_WEIGHT, "
         "TUNED_BACKWARDS_MOVEMENT_MOBILITY_WEIGHT);";
  m_output.flush();
}

double Tuner::huber_loss(double a) const {
  if (std::abs(a) <= TUNER_HUBER_LOSS_GAMMA) {
    return (0.5L * (a * a));
  } else {
    double m = std::sqrt((a * a) + TUNER_EPSILON);
    return (TUNER_HUBER_LOSS_GAMMA * (m - (0.5L * TUNER_HUBER_LOSS_GAMMA)));
  }
}

double Tuner::derivative_huber_loss(double a) const {
  if (std::abs(a) <= TUNER_HUBER_LOSS_GAMMA) {
    return (-a);
  } else {
    double denominator = std::sqrt((a * a) + TUNER_EPSILON);
    return (-1 * TUNER_HUBER_LOSS_GAMMA * (a / denominator));
  }
}

double Tuner::sigmoid(double s) const {
  const double sigmoid = 1.0L / (1.0L + std::exp(-1 * s * TUNER_SIGMOID_K));
  return sigmoid;
}

double Tuner::derivative_sigmoid(double s) const {
  return sigmoid(s) * (1 - sigmoid(s)) * TUNER_SIGMOID_K;
}
