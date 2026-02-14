#include "tuner.hpp"

Tuner::Tuner(std::ostream& logging, std::ifstream& dataset,
             std::ofstream& output)
    : m_log(logging), m_output(output) {
  m_dataset = parse_dataset_file(dataset);
}

Evaluation_Weights<double> Tuner::tune() {
  Evaluation_Weights<double> weights;
  weights = weights + TUNER_WEIGHTS_INITIALIZATION_VALUE;
  Evaluation_Weights<double> first_moment;
  Evaluation_Weights<double> second_moment;

  for (uint64_t t = 1; t <= TUNER_ITERATIONS; t++) {
    // Calculate gradient
    Evaluation_Weights<double> gradient = compute_gradient(weights);

    m_log << "[INFO] Gradient at iteration " << t << ":" << std::endl
          << gradient;

    // First moment calculation
    first_moment = (first_moment * TUNER_DECAY_FACTOR) +
                   (gradient * (1.0L - TUNER_DECAY_FACTOR));

    // Second moment calculation
    second_moment = (second_moment * TUNER_NU) +
                    ((gradient * gradient) * (1.0L - TUNER_NU));

    // Bias-corrected first moment calculation
    Evaluation_Weights<double> first_moment_corrected =
        ((first_moment * TUNER_DECAY_FACTOR) /
         (1.0L - std::pow(TUNER_DECAY_FACTOR, (t + 1)))) +
        ((gradient * (1.0L - TUNER_DECAY_FACTOR)) /
         (1.0L - std::pow(TUNER_DECAY_FACTOR, t)));

    // Bias-corrected second moment calculation
    Evaluation_Weights<double> second_moment_corrected =
        (second_moment * TUNER_NU) / (1.0L - std::pow(TUNER_NU, t));

    // Parameter update
    weights = weights - ((TUNER_LEARNING_RATE /
                          (second_moment_corrected + TUNER_EPSILON).sqrt()) *
                         first_moment_corrected);

    // Compute this iteration's loss.
    double loss = compute_loss(weights);

    m_log << "[INFO] Weights at iteration " << t << ":" << std::endl << weights;

    m_log << "[INFO] Iteration " << t << ": Loss = " << loss << std::endl;
  }

  return weights;
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

  m_log << "[INFO] Finished parsing dataset file of "
        << returned_dataset.fens.size() << " entries." << std::endl;

  return returned_dataset;
}

Evaluation_Weights<double> Tuner::compute_gradient(
    const Evaluation_Weights<double>& weights) {
  Evaluation_Weights<double> gradient;

  for (std::size_t j = 0; j < gradient.get_size(); j++) {
    double L_data_wj = 0.0L;
    std::size_t N = m_dataset.fens.size();

    for (std::size_t i = 0; i < N; i++) {
      Chess_Board cb;
      cb.set_from_fen(m_dataset.fens[i]);

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

      Evaluator e(weights, cb, moving_side_matrix, opposing_side_matrix);

      const double evaluation = e.evaluate_template_typed();
      const double evaluation_white =
          (cb.get_side_to_move() == PIECE_COLOR::WHITE)
              ? evaluation
              : -evaluation;  // Convert side-to-move's evaluation to white's
                              // perspective.
      const double target_evaluation = m_dataset.scores[i];
      const double error = target_evaluation - sigmoid(evaluation_white);
      const double huber_loss_derivative = derivative_huber_loss(error);
      const double sigmoid_derivative = derivative_sigmoid(evaluation_white);
      const double evaluation_deriative = e.derivative_evaluate()[j];

      L_data_wj +=
          (huber_loss_derivative * sigmoid_derivative * evaluation_deriative);
    }

    L_data_wj = L_data_wj / static_cast<double>(N);

    const double L_reg_wj = 2.0L * TUNER_REGULARIZATION_LAMBDA * weights[j];

    gradient[j] = L_data_wj + L_reg_wj;
  }

  return gradient;
}

double Tuner::compute_loss(const Evaluation_Weights<double>& weights) {
  double loss = 0.0L;
  const std::size_t N = m_dataset.fens.size();

  for (std::size_t i = 0; i < N; i++) {
    Chess_Board cb;
    cb.set_from_fen(m_dataset.fens[i]);

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

    Evaluator e(weights, cb, moving_side_matrix, opposing_side_matrix);

    const double evaluation = e.evaluate_template_typed();
    const double evaluation_white =
        (cb.get_side_to_move() == PIECE_COLOR::WHITE)
            ? evaluation
            : -evaluation;  // Convert side-to-move's evaluation to white's
                            // perspective.
    const double target_evaluation = m_dataset.scores[i];
    const double error = target_evaluation - sigmoid(evaluation_white);
    loss += huber_loss(error);
  }

  loss = loss / static_cast<double>(N);

  double regularization = 0.0L;

  for (std::size_t j = 0; j < weights.get_size(); j++) {
    regularization += (weights[j] * weights[j]);
  }

  loss += (TUNER_REGULARIZATION_LAMBDA * regularization);

  return loss;
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
