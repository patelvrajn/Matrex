#pragma once

#include <fstream>
#include <ostream>
#include <vector>

#include "evaluate.hpp"
#include "globals.hpp"
#include "threads.hpp"

constexpr uint64_t TUNER_NUM_OF_THREADS      = 4;
constexpr double   TUNER_BLOCK_LEARNING_RATE = 1;
constexpr double   TUNER_BLOCK_MOMENTUM =
    1.0 - (1.0 / static_cast<double>(TUNER_NUM_OF_THREADS));

constexpr uint64_t TUNER_MAX_EPOCHS       = 30;
constexpr uint64_t TUNER_MINI_BATCH_SIZE  = 16384;
constexpr double   TUNER_VALIDATION_SPLIT = 0.2;

constexpr uint64_t TUNER_LINEAR_LR_MAX_EPOCHS = 8;
constexpr double   TUNER_MIN_LINEAR_LR        = 1e-3;
constexpr double   TUNER_MAX_LINEAR_LR        = 1e-1;

constexpr uint64_t TUNER_COSINE_LR_MAX_EPOCHS =
    TUNER_MAX_EPOCHS - TUNER_LINEAR_LR_MAX_EPOCHS;
constexpr double TUNER_MIN_COSINE_LR = 1e-7;
constexpr double TUNER_MAX_COSINE_LR = TUNER_MAX_LINEAR_LR;

constexpr double TUNER_DECAY_FACTOR     = 0.975;
constexpr double TUNER_NU               = 0.999;
constexpr double TUNER_EPSILON          = 1e-8;
constexpr double TUNER_HUBER_LOSS_GAMMA = 0.80;
// Sigmoid k-value controls the sensitivity of win-probability with respect to
// evaluation. It sets how many evaluation points correspond to a meaningful
// change in win-probability. The sigmoid function itself is not a clamp for the
// evaluation but does affect the magnitude of evaluations. Note, that the
// gradient of the loss surface is directly scaled by the k-value as well.
constexpr double  TUNER_SIGMOID_K               = 0.002;
constexpr double  TUNER_REGULARIZATION_LAMBDA   = 1e-3;
constexpr double  TUNER_LOSS_IMPROVEMENT_CUTOFF = 1e-6;
constexpr double  TUNER_WEIGHT_UPDATE_CUTOFF    = 1e-4;
constexpr uint8_t TUNER_PATIENCE                = 7;

struct Mini_Batch
{
    std::vector<std::string> fens;
    std::vector<double>      scores;
};

struct Worker_Batches
{
    multi_array<Mini_Batch, TUNER_NUM_OF_THREADS> batches;
};

struct Dataset
{
    std::vector<Mini_Batch> mini_batches;
    std::size_t             size;
};

struct Tuner_Eval_Params
{
    std::vector<Chess_Board>           boards;
    std::vector<Moves_Bitboard_Matrix> moving_side_matrices;
    std::vector<Moves_Bitboard_Matrix> opposing_side_matrices;
};

class Tuner
{
  public:

    Tuner(std::ostream&  logging,
          std::ifstream& dataset_file,
          std::ofstream& output);
    Evaluation_Weights<double> tune();

    Evaluation_Weights<double>
    compute_gradient(const Evaluation_Weights<double>& weights,
                     const Mini_Batch&                 mini_batch) const;

    Evaluation_Weights<double>
    projected_gradient(const Evaluation_Weights<double>& weights,
                       const Evaluation_Weights<double>& gradient) const;

    Evaluation_Weights<double> projected_weight_change(
        const Evaluation_Weights<double>& weights,
        const Evaluation_Weights<double>& weight_update) const;

  private:

    std::ostream&  m_log;
    Dataset        m_training_dataset;
    Dataset        m_validation_dataset;
    std::ofstream& m_output;

    Thread_Pool m_thread_pool;

    double                 perturb(double mean);
    NLR_Parameters<double> random_nlr(double h_mean);

    Evaluation_Weights<double> init_weights();

    double learning_rate_scheduler(uint64_t epoch) const;

    void parse_dataset_file(std::ifstream& dataset_file,
                            Dataset&       training_dataset,
                            Dataset&       validation_dataset);

    Dataset create_mini_batches(const Mini_Batch& aggregate_batch);

    Worker_Batches create_worker_batches(const Mini_Batch& mini_batch);

    Tuner_Eval_Params compute_eval_params(const Mini_Batch& mini_batch) const;

    auto create_gradient_pair_weights(
        const Evaluation_Weights<double>& weights) const
    {
        auto output = std::make_unique<Evaluation_Weights<Value_Gradient_Pair<double>>>();

        for (std::size_t i = 0; i < weights.get_size(); ++i)
        {
            Evaluation_Weights<double> one_hot_basis;
            one_hot_basis[i] = 1.0;

            (*output)[i] =
                Value_Gradient_Pair<double>::variable(weights[i], one_hot_basis);
        }

        return output;
    }

    double compute_loss(const Dataset&                    d,
                        const Evaluation_Weights<double>& weights);

    double compute_max_data_loss(const Dataset& d);

    void print_element_as_cpp(std::ofstream& ofs, double scalar);
    void print_element_as_cpp(std::ofstream&                ofs,
                              const NLR_Parameters<double>& nlr);

    template <typename T, std::size_t N>
    void print_multi_array_as_cpp(std::ofstream&           ofs,
                                  const multi_array<T, N>& arr);

    template <typename T, std::size_t N, std::size_t... Rest>
    void print_multi_array_as_cpp(std::ofstream&                    ofs,
                                  const multi_array<T, N, Rest...>& arr);

    void print_header_file(const Evaluation_Weights<double>& weights);

    double huber_loss(double a) const;
    double derivative_huber_loss(double a) const;

    double sigmoid(double s) const;
    double derivative_sigmoid(double s) const;
};

struct Tuner_Step_State
{
    Evaluation_Weights<double> first_moment;
    Evaluation_Weights<double> second_moment;
    Evaluation_Weights<double> weights;
};

using Tuner_Step_States_Array =
    multi_array<Tuner_Step_State, TUNER_NUM_OF_THREADS>;

class Tuner_Step : public Thread_Job
{
  private:

    constexpr static std::size_t m_index_to_global_learning_rate            = 0;
    constexpr static std::size_t m_index_to_global_state                    = 1;
    constexpr static std::size_t m_index_to_local_states                    = 2;
    constexpr static std::size_t m_index_to_weight_update_magnitude_average = 3;
    constexpr static std::size_t m_index_to_timestep                        = 4;

    std::size_t m_index_to_tuner;
    std::size_t m_index_to_data;

  public:

    Tuner_Step(Threads_Shared_Data& shared_data,
               const Tuner&         tuner_instance,
               const Mini_Batch&    worker_batch) :
        Thread_Job(shared_data)
    {
        m_index_to_tuner = write_reference_to_private_data(tuner_instance);
        m_index_to_data  = write_reference_to_private_data(worker_batch);
    }

    virtual bool has_job() const override { return true; }

    virtual std::any operator()(std::stop_token) override
    {
        // Grab necessary shared data.
        const auto global_learning_rate =
            read_shared_data<double>(m_index_to_global_learning_rate).get();
        const auto global_state =
            read_shared_data<Tuner_Step_State>(m_index_to_global_state).get();
        const auto global_timestep =
            read_shared_data<uint64_t>(m_index_to_timestep).get();

        // Grab necessary private data.
        const auto& tuner_instance =
            std::any_cast<std::reference_wrapper<const Tuner>>(
                read_private_data(m_index_to_tuner))
                .get();
        const auto& batch =
            std::any_cast<std::reference_wrapper<const Mini_Batch>>(
                read_private_data(m_index_to_data))
                .get();

        // Calculate the local gradient using the global state's weights.
        Evaluation_Weights<double> gradient =
            tuner_instance.compute_gradient(global_state.weights, batch);
        gradient =
            tuner_instance.projected_gradient(global_state.weights, gradient);

        // Local first moment calculation based on global first moment.
        const Evaluation_Weights<double> first_moment =
            (global_state.first_moment * TUNER_DECAY_FACTOR)
            + (gradient * (1.0L - TUNER_DECAY_FACTOR));

        // Commit the calculated local first moment to this thread's local
        // state in shared memory.
        call_shared_data<Tuner_Step_States_Array>(
            m_index_to_local_states,
            [&](Tuner_Step_States_Array& states)
            { states[get_assigned_thread_id()].first_moment = first_moment; });

        // Second moment calculation
        const Evaluation_Weights<double> second_moment =
            (global_state.second_moment * TUNER_NU)
            + ((gradient * gradient) * (1.0L - TUNER_NU));

        // Commit the calculated local second moment to this thread's local
        // state in shared memory.
        call_shared_data<Tuner_Step_States_Array>(
            m_index_to_local_states,
            [&](Tuner_Step_States_Array& states)
            {
                states[get_assigned_thread_id()].second_moment = second_moment;
            });

        // Bias-corrected first moment calculation - notice that the extra
        // gradient term in the bias-correct first moment is where NADAM comes
        // into play. This addition is algebraically equivalent to considering
        // the previous momentum into the calculation of this iteration's
        // gradient.
        const Evaluation_Weights<double> first_moment_corrected =
            ((first_moment * TUNER_DECAY_FACTOR)
             / (1.0L - std::pow(TUNER_DECAY_FACTOR, (global_timestep + 1))))
            + ((gradient * (1.0L - TUNER_DECAY_FACTOR))
               / (1.0L - std::pow(TUNER_DECAY_FACTOR, global_timestep)));

        // Bias-corrected second moment calculation
        const Evaluation_Weights<double> second_moment_corrected =
            (second_moment * TUNER_NU)
            / (1.0L - std::pow(TUNER_NU, global_timestep));

        // Weight update with respect to the gradient and moments.
        const Evaluation_Weights<double> weight_update =
            ((global_learning_rate
              / (second_moment_corrected + TUNER_EPSILON).sqrt())
             * first_moment_corrected);

        // Calculate the magnitude of the weight update vector and then sum it
        // into the global weight update magnitude - this is used in one of the
        // methods to terminate the tuning.
        const double weight_update_magnitude = weight_update.magnitude();
        call_shared_data<double>(m_index_to_weight_update_magnitude_average,
                                 [&](double& average)
                                 { average += weight_update_magnitude; });

        // Decoupled weight decay calculation (concept from AdamW)
        const Evaluation_Weights<double> decoupled_weight_decay =
            global_state.weights * TUNER_REGULARIZATION_LAMBDA;
        const Evaluation_Weights<double> weight_decay_update =
            decoupled_weight_decay * global_learning_rate;

        // Total weight update
        const Evaluation_Weights<double> total_weight_update =
            weight_update + weight_decay_update;

        // Calculate the final weights and commit them to this thread's local
        // state in shared memory.
        const Evaluation_Weights<double> weights =
            tuner_instance.projected_weight_change(global_state.weights,
                                                   total_weight_update);
        call_shared_data<Tuner_Step_States_Array>(
            m_index_to_local_states,
            [&](Tuner_Step_States_Array& states)
            { states[get_assigned_thread_id()].weights = weights; });

        return weights;
    }
};
