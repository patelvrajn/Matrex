#include "tuner.hpp"

#include <iomanip>
#include <numbers>
#include <random>

#include "threads.hpp"

Tuner::Tuner(std::ostream&  logging,
             std::ifstream& dataset_file,
             std::ofstream& output) :
    m_log(logging), m_output(output), m_thread_pool(TUNER_NUM_OF_THREADS)
{
    parse_dataset_file(dataset_file, m_training_dataset, m_validation_dataset);

    constexpr uint8_t DOUBLE_STD_OUT_PRECISION = 8;

    m_log << std::setprecision(DOUBLE_STD_OUT_PRECISION);
    m_output << std::setprecision(DOUBLE_STD_OUT_PRECISION);
}

double Tuner::perturb(double mean)
{
    std::random_device               rd;
    std::mt19937_64                  rng(rd());
    constexpr double                 PERTURBATION_STD_DEV_FACTOR = 0.10;
    std::normal_distribution<double> distribution(
        mean,
        std::abs(PERTURBATION_STD_DEV_FACTOR * mean));
    return distribution(rng);
}

NLR_Parameters<double> Tuner::random_nlr(double h_mean)
{
    return {.h_plus  = perturb(h_mean),
            .h_minus = perturb(h_mean),
            .z       = perturb(1.0L),
            .k       = perturb(1.0L),
            .q_plus  = perturb(0.25L),
            .q_minus = perturb(0.25L),
            .r_plus  = perturb(-1.0L),
            .r_minus = perturb(-1.0L),
            .g_plus  = perturb(1.0L),
            .g_minus = perturb(1.0L)};
}

Evaluation_Weights<double> Tuner::init_weights()
{
    std::random_device                     rd;
    std::mt19937_64                        rng(rd());
    std::uniform_real_distribution<double> distribution(
        Matrex_FP_Int::safe_minimum(),
        Matrex_FP_Int::safe_maximum());

    Evaluation_Weights<double> weights;

    weights.material_NLR_parameters = {random_nlr(0.25L),
                                       random_nlr(0.5L),
                                       random_nlr(0.65L),
                                       random_nlr(0.85L),
                                       random_nlr(0.95L)};
    weights.material                = {perturb(0.1L),
                                       perturb(0.3L),
                                       perturb(0.35L),
                                       perturb(0.65L),
                                       perturb(0.9L)};

    weights.piece_mobility_NLR_parameters = {random_nlr(0.05L),
                                             random_nlr(0.15L),
                                             random_nlr(0.2L),
                                             random_nlr(0.3L),
                                             random_nlr(0.6L),
                                             random_nlr(0.1L)};
    weights.diagonal_mobility             = perturb(0.43L);
    weights.orthogonal_mobility           = perturb(0.34L);
    weights.knight_movement_mobility      = perturb(0.57L);
    weights.multi_movement_mobility       = perturb(1.0L);
    weights.backwards_movement_mobility   = perturb(0.36L);

    for (uint8_t color = PIECE_COLOR::WHITE; color <= PIECE_COLOR::BLACK;
         color++)
    {
        for (uint8_t piece = PIECES::PAWN; piece <= PIECES::KING; piece++)
        {
            weights.piece_square_NLR_parameters[color][piece] = random_nlr(1);
        }
    }

    weights.interactive_piece_square_NLR_parameters = {random_nlr(1),
                                                       random_nlr(1)};

    for (uint8_t color = PIECE_COLOR::WHITE; color <= PIECE_COLOR::BLACK;
         color++)
    {
        for (uint8_t piece = PIECES::PAWN; piece <= PIECES::KING; piece++)
        {
            for (uint8_t square_idx = 0;
                 square_idx < NUM_OF_SQUARES_ON_CHESS_BOARD;
                 square_idx++)
            {
                weights.piece_square_tables[color][piece][square_idx] =
                    distribution(rng);
            }
        }
    }

    return weights;
}

Evaluation_Weights<double> Tuner::tune()
{
    Tuner_Step_State global_state;
    global_state.weights = init_weights();

    m_log << "[INFO] Initial weights: " << global_state.weights << std::endl;

    Evaluation_Weights<double> best_weights = global_state.weights;

    const std::size_t num_of_mini_batches =
        m_training_dataset.mini_batches.size();
    const double max_training_data_loss =
        compute_max_data_loss(m_training_dataset);
    const double max_validation_data_loss =
        compute_max_data_loss(m_validation_dataset);

    m_log << "[INFO] There is " << num_of_mini_batches
          << " batches in 1 training epoch." << std::endl;
    m_log << "[INFO] Maximum data loss for the training dataset is: "
          << max_training_data_loss << std::endl;
    m_log << "[INFO] Maximum data loss for the validation dataset is: "
          << max_validation_data_loss << std::endl;

    uint64_t t = 1; // NADAM's timestep
    double   previous_epoch_training_loss =
        compute_loss(m_training_dataset, best_weights);
    double previous_epoch_validation_loss =
        compute_loss(m_validation_dataset, best_weights);
    uint8_t epoch_patience_count = 0;

    m_log << "[INFO] Initial training dataset loss is "
          << previous_epoch_training_loss << std::endl;
    m_log << "[INFO] Initial validation dataset loss is "
          << previous_epoch_validation_loss << std::endl;

    for (uint64_t epoch = 1; epoch <= TUNER_MAX_EPOCHS; epoch++)
    {
        double weight_update_magnitude_average = 0;

        // Shuffling our mini batches diversifies the loss landscapes L_Xi (Xi
        // being the mini batch i of a training set X - shuffling increases the
        // number of unique sequences of error surfaces seen) that we traverse
        // during each epoch, which helps escape local minima and improves
        // generalization (there is no order of mini-batches being engrained in
        // the learning). Note, that even though the mini-batches introduce a
        // sequence of unique loss surfaces we are still able to converge
        // because there will be enough similarity between the loss surfaces of
        // different mini-batches for the optimization to make progress towards
        // a minimum - noting that it will escape some local minima that are not
        // minima across all mini-batches (some depressions in the surface/local
        // minima can be caused by noise in the data of a particular
        // mini-batch).
        std::shuffle(m_training_dataset.mini_batches.begin(),
                     m_training_dataset.mini_batches.end(),
                     std::mt19937_64(std::random_device {}()));

        double learning_rate = learning_rate_scheduler(epoch);

        m_log << "[INFO] Learning rate for epoch " << epoch << " is "
              << learning_rate << std::endl;

        for (const Mini_Batch& mini_batch : m_training_dataset.mini_batches)
        {
            // Create a split of the mini batch for every worker thread.
            Worker_Batches worker_data = create_worker_batches(mini_batch);

            // Create an array for each thread to store it's state.
            Tuner_Step_States_Array thread_states;

            // Create the shared data object for the threads.
            Threads_Shared_Data shared_data(learning_rate,
                                            global_state,
                                            thread_states,
                                            weight_update_magnitude_average,
                                            t);

            // Push as many jobs as there are threads assigning each tuner step
            // job a different split of the mini-batch.
            for (std::size_t i = 0; i < TUNER_NUM_OF_THREADS; i++)
            {
                std::unique_ptr<Thread_Job> job =
                    std::make_unique<Tuner_Step>(shared_data,
                                                 (*this),
                                                 worker_data.batches[i]);

                m_thread_pool.push_job(std::move(job));
            }

            // Wait for all threads in the pool to complete their jobs.
            m_thread_pool.wait_for_jobs_to_complete();

            // Calculate the average state (average moments and average weights)
            // from the thread states array. Note that all threads have
            // completed execution so their is no need for synchronization
            // primitives to access their final states.
            Tuner_Step_State average_state;
            for (const auto& state : thread_states)
            {
                average_state.first_moment =
                    average_state.first_moment + state.first_moment;
                average_state.second_moment =
                    average_state.second_moment + state.second_moment;
                average_state.weights = average_state.weights + state.weights;
            }
            average_state.first_moment =
                average_state.first_moment
                / static_cast<double>(TUNER_NUM_OF_THREADS);
            average_state.second_moment =
                average_state.second_moment
                / static_cast<double>(TUNER_NUM_OF_THREADS);
            average_state.weights = average_state.weights
                                  / static_cast<double>(TUNER_NUM_OF_THREADS);

            // Assign the new global state as the average.
            global_state = average_state;

            m_log << "Weights at timestep " << t << " are; "
                  << global_state.weights << std::endl;

            // Increment timestep.
            t++;
        }

        weight_update_magnitude_average =
            weight_update_magnitude_average / num_of_mini_batches;

        const double validation_loss =
            compute_loss(m_validation_dataset, global_state.weights);
        const double validation_loss_percent =
            100.0L * (validation_loss / max_validation_data_loss);
        const double validation_loss_improvement =
            previous_epoch_validation_loss - validation_loss;

        const double training_loss =
            compute_loss(m_training_dataset, global_state.weights);
        const double training_loss_percent =
            100.0L * (training_loss / max_training_data_loss);
        const double training_loss_improvement =
            previous_epoch_training_loss - training_loss;

        m_log << "[INFO] Epoch " << epoch
              << ": Validation Loss = " << validation_loss
              << "; Validation Loss Improvement = "
              << validation_loss_improvement
              << "; Validation Loss Percent = " << validation_loss_percent
              << "%" << "; Training Loss = " << training_loss
              << "; Training Loss Improvement = " << training_loss_improvement
              << "; Training Loss Percent = " << training_loss_percent << "%"
              << "; Weight Update Average = " << weight_update_magnitude_average
              << std::endl;

        if ((validation_loss_improvement < TUNER_LOSS_IMPROVEMENT_CUTOFF)
            && (weight_update_magnitude_average < TUNER_WEIGHT_UPDATE_CUTOFF))
        { // Converged!
            epoch_patience_count++;
            if (epoch_patience_count == TUNER_PATIENCE) { break; }
        }
        else
        { // Not converged.
            if (validation_loss_improvement > 0)
            {
                best_weights = global_state.weights;
            }
            epoch_patience_count = 0;
        }

        m_log << "[INFO] Epoch patience count = "
              << static_cast<uint64_t>(epoch_patience_count) << std::endl;

        previous_epoch_training_loss   = training_loss;
        previous_epoch_validation_loss = validation_loss;
    }

    print_header_file(best_weights);

    return best_weights;
}

// In the theme of projected gradient descent, we set the gradient to a scaled
// value if the weights minus the gradient were to cross the valid range for the
// fixed point math we are using for static evaluation - this is to prevent the
// algorithm from jumping back and forth between the valid range and outside the
// valid range because of momentum.
Evaluation_Weights<double>
Tuner::projected_gradient(const Evaluation_Weights<double>& weights,
                          const Evaluation_Weights<double>& gradient) const
{
    Evaluation_Weights<double> result;

    for (std::size_t i = 0; i < weights.get_size(); i++)
    {
        // If the gradient is negative, according to the traditional gradient
        // descent weight update, we want to increase the weights - so as a
        // conservative measure we reduce the gradient to a numerical value a
        // scale less than what would be needed to be greater than the maximum -
        // this allows us to keep some form of a direction/magnitude rather than
        // aggressively diminishing it to zero.
        constexpr double PROJECTED_GRADIENT_SCALE = 1000.0;
        const double     weights_with_step        = weights[i] - gradient[i];
        if (weights_with_step > Matrex_FP_Int::safe_maximum())
        {
            double distance_from_boundary =
                std::abs(weights[i] - Matrex_FP_Int::safe_maximum());
            result[i] = std::copysign(distance_from_boundary, gradient[i])
                      / PROJECTED_GRADIENT_SCALE;
        }
        // Simalaur logic as the maximum clamp.
        else if (weights_with_step < Matrex_FP_Int::safe_minimum())
        {
            double distance_from_boundary =
                std::abs(weights[i] - Matrex_FP_Int::safe_minimum());
            result[i] = std::copysign(distance_from_boundary, gradient[i])
                      / PROJECTED_GRADIENT_SCALE;
        }
        else
        {
            result[i] = gradient[i];
        }
    }

    return result;
}

// Here we are using a concept from projected gradient descent where we project
// the weights back into a valid range for the fixed point math we are using for
// static evaluation.
Evaluation_Weights<double> Tuner::projected_weight_change(
    const Evaluation_Weights<double>& weights,
    const Evaluation_Weights<double>& weight_update) const
{
    Evaluation_Weights<double> result;

    for (std::size_t i = 0; i < weights.get_size(); i++)
    {
        result[i] = std::clamp((weights[i] - weight_update[i]),
                               Matrex_FP_Int::safe_minimum(),
                               Matrex_FP_Int::safe_maximum());
    }

    return result;
}

// OneCycleLR (Super-convergence)
double Tuner::learning_rate_scheduler(uint64_t epoch) const
{
    if (epoch <= TUNER_LINEAR_LR_MAX_EPOCHS)
    { // Linear rise phase
        const double linear_lr =
            TUNER_MIN_LINEAR_LR
            + (((TUNER_MAX_LINEAR_LR - TUNER_MIN_LINEAR_LR)
                / static_cast<double>(TUNER_LINEAR_LR_MAX_EPOCHS - 1))
               * static_cast<double>(epoch - 1));
        return linear_lr;
    }
    else
    { // Cosine decay phase
        const double cosine_lr =
            TUNER_MIN_COSINE_LR
            + (0.5L * (TUNER_MAX_COSINE_LR - TUNER_MIN_COSINE_LR)
               * (1.0L
                  + std::cos(std::numbers::pi_v<double>
                             * (static_cast<double>(
                                    (epoch - TUNER_LINEAR_LR_MAX_EPOCHS) - 1)
                                / (TUNER_COSINE_LR_MAX_EPOCHS - 1)))));
        return cosine_lr;
    }
}

void Tuner::parse_dataset_file(std::ifstream& dataset_file,
                               Dataset&       training_dataset,
                               Dataset&       validation_dataset)
{
    Mini_Batch aggregate_batch;

    m_log << "[INFO] Starting to parse dataset file." << std::endl;

    std::string line;
    while (std::getline(dataset_file, line))
    {
        std::size_t last_space_pos = line.find_last_of(' ');
        if (last_space_pos != std::string::npos)
        {
            std::string fen = line.substr(0, last_space_pos);

            constexpr uint8_t MAX_SCORE_STRING_LENGTH = 3;
            double            score                   = std::stod(
                line.substr((last_space_pos + 2),
                            MAX_SCORE_STRING_LENGTH)); // +2 to skip space and
                                                       // opening bracket

            // m_log << "[INFO] Parsed (FEN, score) pair: (\"" << fen << "\", "
            // << score
            //       << ")" << std::endl;

            aggregate_batch.fens.push_back(fen);
            aggregate_batch.scores.push_back(score);
        }
    }

    m_log << "[INFO] Finished parsing dataset file of "
          << aggregate_batch.fens.size() << " entries." << std::endl;

    if (aggregate_batch.fens.empty())
    {
        throw std::runtime_error("Dataset file is empty.");
    }

    std::size_t training_dataset_size = static_cast<std::size_t>(
        (1.0L - TUNER_VALIDATION_SPLIT)
        * static_cast<double>(aggregate_batch.fens.size()));

    auto fens_split_it =
        aggregate_batch.fens.begin() + (training_dataset_size - 1);
    auto scores_split_it =
        aggregate_batch.scores.begin() + (training_dataset_size - 1);

    Mini_Batch training_aggregate_batch {
        std::vector<std::string>(aggregate_batch.fens.begin(), fens_split_it),
        std::vector<double>(aggregate_batch.scores.begin(), scores_split_it)};
    Mini_Batch validation_aggregate_batch {
        std::vector<std::string>(fens_split_it, aggregate_batch.fens.end()),
        std::vector<double>(scores_split_it, aggregate_batch.scores.end())};

    training_dataset   = create_mini_batches(training_aggregate_batch);
    validation_dataset = create_mini_batches(validation_aggregate_batch);
}

Dataset Tuner::create_mini_batches(const Mini_Batch& aggregate_batch)
{
    Dataset returned_dataset;

    // Now that we have the size of the entire dataset, split the aggregate
    // batch into mini-batches of size at most TUNER_MINI_BATCH_SIZE.
    for (std::size_t i  = 0; i < aggregate_batch.fens.size();
         i             += TUNER_MINI_BATCH_SIZE)
    {
        Mini_Batch mini_batch;

        for (std::size_t j = i; (j < (i + TUNER_MINI_BATCH_SIZE)
                                 && j < aggregate_batch.fens.size());
             j++)
        {
            mini_batch.fens.push_back(aggregate_batch.fens[j]);
            mini_batch.scores.push_back(aggregate_batch.scores[j]);
        }

        returned_dataset.mini_batches.push_back(mini_batch);
    }

    returned_dataset.size = aggregate_batch.fens.size();

    return returned_dataset;
}

Worker_Batches Tuner::create_worker_batches(const Mini_Batch& mini_batch)
{
    Worker_Batches returned_batches;

    const std::size_t mini_batch_size = mini_batch.fens.size();
    const std::size_t worker_batch_size =
        mini_batch_size / TUNER_NUM_OF_THREADS;

    for (std::size_t i = 0; i < TUNER_NUM_OF_THREADS; i++)
    {
        Mini_Batch batch;

        const std::size_t batch_start = worker_batch_size * i;
        const std::size_t batch_end   = (i == (TUNER_NUM_OF_THREADS - 1))
                                          ? mini_batch_size
                                          : worker_batch_size * (i + 1);

        batch.fens =
            std::vector<std::string>(mini_batch.fens.begin() + batch_start,
                                     mini_batch.fens.begin() + batch_end);
        batch.scores =
            std::vector<double>(mini_batch.scores.begin() + batch_start,
                                mini_batch.scores.begin() + batch_end);

        returned_batches.batches[i] = batch;
    }

    return returned_batches;
}

Tuner_Eval_Params Tuner::compute_eval_params(const Mini_Batch& mini_batch) const
{
    Tuner_Eval_Params return_value;
    return_value.boards.reserve(mini_batch.fens.size());
    return_value.moving_side_matrices.reserve(mini_batch.fens.size());
    return_value.opposing_side_matrices.reserve(mini_batch.fens.size());

    for (std::size_t i = 0; i < mini_batch.fens.size(); i++)
    {
        Chess_Board cb;
        cb.set_from_fen(mini_batch.fens[i]);

        const PIECE_COLOR     moving_side = cb.get_side_to_move();
        Move_Generation_List  moving_side_moves_list;
        Moves_Bitboard_Matrix moving_side_matrix;
        Move_Generator        mg_moving_side(cb);
        mg_moving_side.generate_all_moves<MOVE_GENERATION_TYPE::ALL>(
            moving_side,
            moving_side_moves_list,
            moving_side_matrix);

        const PIECE_COLOR opposing_side =
            (PIECE_COLOR) ((~cb.get_side_to_move()) & 0x1);
        Move_Generation_List  opposing_side_moves_list;
        Moves_Bitboard_Matrix opposing_side_matrix;
        Move_Generator        mg_opposing_side(cb);
        mg_opposing_side.generate_all_moves<MOVE_GENERATION_TYPE::ALL>(
            opposing_side,
            opposing_side_moves_list,
            opposing_side_matrix);

        return_value.boards.push_back(cb);
        return_value.moving_side_matrices.push_back(moving_side_matrix);
        return_value.opposing_side_matrices.push_back(opposing_side_matrix);
    }

    return return_value;
}

Evaluation_Weights<double>
Tuner::compute_gradient(const Evaluation_Weights<double>& weights,
                        const Mini_Batch&                 mini_batch) const
{
    Evaluation_Weights<double> gradient;

    std::size_t N = mini_batch.fens.size();

    Tuner_Eval_Params eval_params = compute_eval_params(mini_batch);

    for (std::size_t i = 0; i < N; i++)
    {
        Evaluator e(weights,
                    eval_params.boards[i],
                    eval_params.moving_side_matrices[i],
                    eval_params.opposing_side_matrices[i]);

        const double sign =
            (eval_params.boards[i].get_side_to_move() == PIECE_COLOR::WHITE)
                ? 1.0L
                : -1.0L;
        const double evaluation = e.evaluate_template_typed();
        const double evaluation_white =
            sign * evaluation; // Convert side-to-move's evaluation to white's
                               // perspective.
        const double target_evaluation = mini_batch.scores[i];
        const double error = target_evaluation - sigmoid(evaluation_white);
        const double huber_loss_derivative = derivative_huber_loss(error);
        const double sigmoid_derivative = derivative_sigmoid(evaluation_white);
        const Evaluation_Weights<double> evaluation_deriative =
            (e.derivative_evaluate() * sign);

        gradient = gradient
                 + (evaluation_deriative
                    * (huber_loss_derivative * sigmoid_derivative));
    }

    gradient = gradient / static_cast<double>(N);

    return gradient;
}

double Tuner::compute_loss(const Dataset&                    d,
                           const Evaluation_Weights<double>& weights)
{
    double            loss = 0.0L;
    const std::size_t N    = d.size;

    for (const Mini_Batch& mini_batch : d.mini_batches)
    {
        Tuner_Eval_Params eval_params = compute_eval_params(mini_batch);

        for (std::size_t i = 0; i < mini_batch.fens.size(); i++)
        {
            Evaluator e(weights,
                        eval_params.boards[i],
                        eval_params.moving_side_matrices[i],
                        eval_params.opposing_side_matrices[i]);

            const double sign =
                (eval_params.boards[i].get_side_to_move() == PIECE_COLOR::WHITE)
                    ? 1.0L
                    : -1.0L;
            const double evaluation = e.evaluate_template_typed();
            const double evaluation_white =
                sign * evaluation; // Convert side-to-move's evaluation to
                                   // white's perspective.
            const double target_evaluation = mini_batch.scores[i];
            const double error  = target_evaluation - sigmoid(evaluation_white);
            loss               += huber_loss(error);
        }
    }

    loss = loss / static_cast<double>(N);

    return loss;
}

double Tuner::compute_max_data_loss(const Dataset& d)
{
    std::size_t num_of_decisive_games = 0;
    std::size_t num_of_draws          = 0;

    for (const Mini_Batch& mini_batch : d.mini_batches)
    {
        for (std::size_t i = 0; i < mini_batch.fens.size(); i++)
        {
            if (mini_batch.scores[i] != 0.5L) { num_of_decisive_games++; }
            else
            {
                num_of_draws++;
            }
        }
    }

    const double loss_on_decisive = huber_loss(1.0L);
    const double loss_on_draw     = huber_loss(0.5L);

    const double max_data_loss = ((num_of_decisive_games * loss_on_decisive)
                                  + (num_of_draws * loss_on_draw))
                               / static_cast<double>(d.size);

    return max_data_loss;
}

void Tuner::print_element_as_cpp(std::ofstream& ofs, double scalar)
{
    ofs << "Matrex_FP_Int(" << Matrex_FP_Int::from_double(scalar) << ")";
}

void Tuner::print_element_as_cpp(std::ofstream&                ofs,
                                 const NLR_Parameters<double>& nlr)
{
    ofs << "NLR_Parameters<Matrex_FP_Int>{" << ".h_plus = Matrex_FP_Int("
        << Matrex_FP_Int::from_double(nlr.h_plus) << ")"
        << ", .h_minus = Matrex_FP_Int("
        << Matrex_FP_Int::from_double(nlr.h_minus) << ")"
        << ", .z = Matrex_FP_Int(" << Matrex_FP_Int::from_double(nlr.z) << ")"
        << ", .k = Matrex_FP_Int(" << Matrex_FP_Int::from_double(nlr.k) << ")"
        << ", .q_plus = Matrex_FP_Int("
        << Matrex_FP_Int::from_double(nlr.q_plus) << ")"
        << ", .q_minus = Matrex_FP_Int("
        << Matrex_FP_Int::from_double(nlr.q_minus) << ")"
        << ", .r_plus = Matrex_FP_Int("
        << Matrex_FP_Int::from_double(nlr.r_plus) << ")"
        << ", .r_minus = Matrex_FP_Int("
        << Matrex_FP_Int::from_double(nlr.r_minus) << ")"
        << ", .g_plus = Matrex_FP_Int("
        << Matrex_FP_Int::from_double(nlr.g_plus) << ")"
        << ", .g_minus = Matrex_FP_Int("
        << Matrex_FP_Int::from_double(nlr.g_minus) << ")" << "}";
}

template <typename T, std::size_t N>
void Tuner::print_multi_array_as_cpp(std::ofstream&           ofs,
                                     const multi_array<T, N>& arr)
{
    ofs << "{";
    for (std::size_t i = 0; i < N; i++)
    {
        print_element_as_cpp(ofs, arr[i]);
        if (i != (N - 1)) { ofs << ", "; }
    }
    ofs << "}";
}

template <typename T, std::size_t N, std::size_t... Rest>
void Tuner::print_multi_array_as_cpp(std::ofstream&                    ofs,
                                     const multi_array<T, N, Rest...>& arr)
{
    ofs << "{";
    for (std::size_t i = 0; i < N; i++)
    {
        print_multi_array_as_cpp(ofs, arr[i]);
        if (i != (N - 1)) { ofs << ", "; }
    }
    ofs << "}";
}

void Tuner::print_header_file(const Evaluation_Weights<double>& weights)
{
    // Includes
    m_output << "#pragma once" << std::endl << std::endl;
    m_output << "#include \"evaluate.hpp\"" << std::endl;
    m_output << "#include \"globals.hpp\"" << std::endl << std::endl;

    // Material
    m_output << "constexpr multi_array<NLR_Parameters<Matrex_FP_Int>, "
                "(NUM_OF_UNIQUE_PIECES_PER_PLAYER - 1)> "
                "TUNED_MATERIAL_NLR_WEIGHTS = ";
    print_multi_array_as_cpp(m_output, weights.material_NLR_parameters);
    m_output << ";" << std::endl;
    m_output << "constexpr multi_array<Matrex_FP_Int, "
                "(NUM_OF_UNIQUE_PIECES_PER_PLAYER "
                "- 1)> TUNED_MATERIAL_WEIGHTS = ";
    print_multi_array_as_cpp(m_output, weights.material);
    m_output << ";" << std::endl;

    // Mobility
    m_output << "constexpr multi_array<NLR_Parameters<Matrex_FP_Int>, "
                "NUM_OF_UNIQUE_PIECES_PER_PLAYER> "
                "TUNED_PIECE_MOBILITY_NLR_WEIGHTS = ";
    print_multi_array_as_cpp(m_output, weights.piece_mobility_NLR_parameters);
    m_output << ";" << std::endl;
    m_output << "constexpr Matrex_FP_Int TUNED_DIAGONAL_MOBILITY_WEIGHT = "
                "Matrex_FP_Int("
             << Matrex_FP_Int::from_double(weights.diagonal_mobility) << ");"
             << std::endl;
    m_output << "constexpr Matrex_FP_Int TUNED_ORTHOGONAL_MOBILITY_WEIGHT = "
                "Matrex_FP_Int("
             << Matrex_FP_Int::from_double(weights.orthogonal_mobility) << ");"
             << std::endl;
    m_output
        << "constexpr Matrex_FP_Int TUNED_KNIGHT_MOVEMENT_MOBILITY_WEIGHT = "
           "Matrex_FP_Int("
        << Matrex_FP_Int::from_double(weights.knight_movement_mobility) << ");"
        << std::endl;
    m_output
        << "constexpr Matrex_FP_Int TUNED_MULTI_MOVEMENT_MOBILITY_WEIGHT = "
           "Matrex_FP_Int("
        << Matrex_FP_Int::from_double(weights.multi_movement_mobility) << ");"
        << std::endl;
    m_output << "constexpr Matrex_FP_Int "
                "TUNED_BACKWARDS_MOVEMENT_MOBILITY_WEIGHT = Matrex_FP_Int("
             << Matrex_FP_Int::from_double(weights.backwards_movement_mobility)
             << ");" << std::endl;

    // Piece Square Tables
    m_output << "constexpr multi_array<NLR_Parameters<Matrex_FP_Int>, "
                "NUM_OF_PLAYERS, NUM_OF_UNIQUE_PIECES_PER_PLAYER> "
                "TUNED_PIECE_SQUARE_NLR_WEIGHTS = ";
    print_multi_array_as_cpp(m_output, weights.piece_square_NLR_parameters);
    m_output << ";" << std::endl;

    m_output << "constexpr multi_array<Matrex_FP_Int, NUM_OF_PLAYERS, "
                "NUM_OF_UNIQUE_PIECES_PER_PLAYER, "
                "NUM_OF_SQUARES_ON_CHESS_BOARD> TUNED_PIECE_SQUARE_TABLE = ";
    print_multi_array_as_cpp(m_output, weights.piece_square_tables);
    m_output << ";" << std::endl;

    m_output << "constexpr multi_array<NLR_Parameters<Matrex_FP_Int>, "
                "NUM_OF_PLAYERS> TUNED_INTERACTIVE_PIECE_SQUARE_NLR_WEIGHTS = ";
    print_multi_array_as_cpp(m_output,
                             weights.interactive_piece_square_NLR_parameters);
    m_output << ";" << std::endl;

    // Weights
    m_output << "const Evaluation_Weights<Matrex_FP_Int> "
                "TUNED_EVALUATION_WEIGHTS(TUNED_MATERIAL_NLR_WEIGHTS,"
                "TUNED_MATERIAL_WEIGHTS,"
                "TUNED_PIECE_MOBILITY_NLR_WEIGHTS,"
                "TUNED_DIAGONAL_MOBILITY_WEIGHT,"
                "TUNED_ORTHOGONAL_MOBILITY_WEIGHT,"
                "TUNED_KNIGHT_MOVEMENT_MOBILITY_WEIGHT,"
                "TUNED_MULTI_MOVEMENT_MOBILITY_WEIGHT,"
                "TUNED_BACKWARDS_MOVEMENT_MOBILITY_WEIGHT,"
                "TUNED_PIECE_SQUARE_NLR_WEIGHTS,"
                "TUNED_PIECE_SQUARE_TABLE,"
                "TUNED_INTERACTIVE_PIECE_SQUARE_NLR_WEIGHTS);";
    m_output.flush();
}

double Tuner::huber_loss(double a) const
{
    if (std::abs(a) <= TUNER_HUBER_LOSS_GAMMA) { return (0.5L * (a * a)); }
    else
    {
        double m = std::sqrt((a * a) + TUNER_EPSILON);
        return (TUNER_HUBER_LOSS_GAMMA * (m - (0.5L * TUNER_HUBER_LOSS_GAMMA)));
    }
}

double Tuner::derivative_huber_loss(double a) const
{
    if (std::abs(a) <= TUNER_HUBER_LOSS_GAMMA) { return (-a); }
    else
    {
        double denominator = std::sqrt((a * a) + TUNER_EPSILON);
        return (-1 * TUNER_HUBER_LOSS_GAMMA * (a / denominator));
    }
}

double Tuner::sigmoid(double s) const
{
    const double sigmoid = 1.0L / (1.0L + std::exp(-1 * s * TUNER_SIGMOID_K));
    return sigmoid;
}

double Tuner::derivative_sigmoid(double s) const
{
    return sigmoid(s) * (1 - sigmoid(s)) * TUNER_SIGMOID_K;
}
