#pragma once

#include <array>

#include "chess_board.hpp"
#include "chess_move.hpp"
#include "cuckoo_reversible_move_table.hpp"
#include "move_generator.hpp"
#include "move_ordering.hpp"
#include "score.hpp"
#include "timer.hpp"
#include "transposition_table.hpp"
#include "correction_history_table.hpp"
#include "history.hpp"

// 5898.5 is the theoritical maximum number of moves (2-ply) in a chess game.
constexpr uint16_t MAX_SEARCH_DEPTH = (5899 * NUM_OF_PLAYERS);

constexpr uint16_t MAX_SEARCH_DEPTH_SOFT_LIMIT = 256;

constexpr uint16_t QUIESCENCE_SEARCH_DEPTH = 0;

constexpr Matrex_FP_Int PV_WINDOW_SIZE = Matrex_FP_Int::from_integer(1);

constexpr std::size_t CORRECTION_HISTORY_TABLE_SIZE = 16384;

struct Time_Control
{
    uint64_t time_remaining; // Time in milliseconds.
    uint64_t increment;      // Time in milliseconds.
};

struct Search_Constraints
{
    std::array<Time_Control, NUM_OF_PLAYERS> time_controls;
    uint64_t                                 transposition_table_size;
};

struct UCI_Search_Information
{
    uint16_t&                 depth;
    uint64_t&                 time;
    uint64_t&                 node_count;
    Principal_Variation_List& principal_variation;
    Score&                    score;

    UCI_Search_Information(uint16_t&                 search_depth,
                           uint64_t&                 search_time,
                           uint64_t&                 search_node_count,
                           Principal_Variation_List& search_principal_variation,
                           Score&                    search_score) :
        depth(search_depth),
        time(search_time),
        node_count(search_node_count),
        principal_variation(search_principal_variation),
        score(search_score)
    {
    }

    friend std::ostream& operator<<(std::ostream&                 os,
                                    const UCI_Search_Information& search_info)
    {
        const Fixed_Point_Int_Storage_Type score_cp =
            Matrex_FP_Int::from_value(search_info.score.to_int()).get_integer();
        const uint64_t time_ms = search_info.time / 1e6;
        const uint64_t nps = search_info.node_count / (search_info.time / 1e9);

        os << "info";
        os << " depth " << search_info.depth;
        os << " nodes " << search_info.node_count;
        os << " time " << time_ms;
        os << " nps " << nps;
        os << " score cp " << score_cp;
        os << " pv" << search_info.principal_variation;

        return os;
    }
};

typedef std::pair<Chess_Move, Score> Search_Engine_Result;

using Search_Quiet_Cont_Hist_Stack =
    Quiet_Continuation_History_Stack<MAX_SEARCH_DEPTH_SOFT_LIMIT>;
using Search_Capture_Cont_Hist_Stack =
    Capture_Continuation_History_Stack<MAX_SEARCH_DEPTH_SOFT_LIMIT>;

class Search_Engine
{
  public:

    Search_Engine();

    void new_game();

    Search_Engine_Result search(const Chess_Board&        cb,
                                const Search_Constraints& constraints);

    const Transposition_Table_Statistics& get_tt_statistics() const;

  private:

    Chess_Board              m_chess_board;
    Transposition_Table      m_transposition_table;
    Search_Constraints       m_constraints;
    PIECE_COLOR              m_my_side;
    Timer                    m_timer;
    bool                     m_timer_expired_during_search;
    uint64_t                 m_num_of_nodes_searched;
    uint16_t                 m_current_search_depth;
    Principal_Variation_List m_principal_variation;
    const Cuckoo_RM_Table    m_cuckoo_rm_table;
    Correction_History_Tables<CORRECTION_HISTORY_TABLE_SIZE>
        m_correction_history;

    Quiet_Continuation_History_Table   m_q_cont_hist_table;
    Search_Quiet_Cont_Hist_Stack       m_q_cont_hist_stack;
    Capture_Continuation_History_Table m_c_cont_hist_table;
    Search_Capture_Cont_Hist_Stack     m_c_cont_hist_stack;

    Search_Engine_Result
    negamax(Chess_Board&                    position,
            uint16_t                        depth,
            Principal_Variation_List&       principal_variation,
            Search_Quiet_Cont_Hist_Stack&   q_cont_hist_stack,
            Search_Capture_Cont_Hist_Stack& c_cont_hist_stack,
            uint16_t                        ply   = 0,
            Score                           alpha = Score(FP_NEGATIVE_INFINITY),
            Score                           beta = Score(FP_POSITIVE_INFINITY));
    Search_Engine_Result
    quiescence(Chess_Board& position, uint16_t ply, Score alpha, Score beta);
    Search_Engine_Result iterative_deepening();

    template <std::size_t CONT_HIST_STACK_SIZE>
    inline Score get_mate_score(const Move_Ordering<CONT_HIST_STACK_SIZE>& mo,
                                uint16_t                                   ply);

    inline bool
    should_use_transposition_table_score(const bool     is_pv,
                                         const bool     is_hit,
                                         const uint16_t depth,
                                         const Transposition_Table_Entry& entry,
                                         const Score                      alpha,
                                         const Score                      beta);

    inline bool
    should_use_transposition_table_score(const bool     is_hit,
                                         const uint16_t depth,
                                         const Transposition_Table_Entry& entry,
                                         const Score                      alpha,
                                         const Score                      beta);
    inline bool
    should_use_transposition_table_score(const bool     is_hit,
                                         const uint16_t depth,
                                         const Transposition_Table_Entry& entry,
                                         const Score                      eval);

    inline bool should_update_correction_history(Chess_Move best_move,
                                                 Score      best_score,
                                                 Score      static_evaluation,
                                                 Score_Bound_Type score_bound,
                                                 bool is_side_to_move_in_check);

    inline bool should_update_quiet_continuation_history(
        const Chess_Move&      beta_cutoff_move,
        const Score_Bound_Type score_bound);

    inline bool should_update_capture_continuation_history(
        const Chess_Move&      beta_cutoff_move,
        const Score_Bound_Type score_bound);

    void
    update_continuation_history(Search_Quiet_Cont_Hist_Stack& q_cont_hist_stack,
                                const Chess_Move&             move,
                                uint16_t                      ply,
                                uint32_t                      depth_squared);

    void update_continuation_history(
        Search_Capture_Cont_Hist_Stack& c_cont_hist_stack,
        const Chess_Move&               move,
        uint16_t                        ply,
        uint32_t                        depth_squared);

    inline bool should_do_see_pruning(const Chess_Move& move,
                                      const Score       best_score);
};

template <std::size_t CONT_HIST_STACK_SIZE>
inline Score
Search_Engine::get_mate_score(const Move_Ordering<CONT_HIST_STACK_SIZE>& mo,
                              uint16_t                                   ply)
{
    Score mate_score;

    // The side to move is in check and has no legal moves means they are in a
    // losing mating net specifically a checkmate at this depth.
    if (mo.is_side_to_move_in_check())
    {
        mate_score = Score::from_int(
            FP_LOSING_MATE_MIN
            + Matrex_FP_Int::from_integer(
                  static_cast<Fixed_Point_Int_Storage_Type>(ply))
                  .get_value());
    }
    else
    {
        mate_score = Score::from_int(ESCORE::DRAW); // Stalemate.
    }

    return mate_score;
}

inline bool Search_Engine::should_use_transposition_table_score(
    const bool                       is_pv,
    const bool                       is_hit,
    const uint16_t                   depth,
    const Transposition_Table_Entry& entry,
    const Score                      alpha,
    const Score                      beta)
{
    return (!is_pv) && is_hit && (depth <= entry.depth)
        && ((entry.score_bound == Score_Bound_Type::EXACT)
            || (entry.score_bound == Score_Bound_Type::LOWER_BOUND
                && entry.score >= beta)
            || (entry.score_bound == Score_Bound_Type::UPPER_BOUND
                && entry.score <= alpha));
}

inline bool Search_Engine::should_use_transposition_table_score(
    const bool                       is_hit,
    const uint16_t                   depth,
    const Transposition_Table_Entry& entry,
    const Score                      alpha,
    const Score                      beta)
{
    return is_hit && (depth <= entry.depth)
        && ((entry.score_bound == Score_Bound_Type::EXACT)
            || (entry.score_bound == Score_Bound_Type::LOWER_BOUND
                && entry.score >= beta)
            || (entry.score_bound == Score_Bound_Type::UPPER_BOUND
                && entry.score <= alpha));
}

inline bool Search_Engine::should_use_transposition_table_score(
    const bool                       is_hit,
    const uint16_t                   depth,
    const Transposition_Table_Entry& entry,
    const Score                      eval)
{
    return is_hit && (depth <= entry.depth)
        && ((entry.score_bound == Score_Bound_Type::EXACT)
            || (entry.score_bound == Score_Bound_Type::LOWER_BOUND
                && entry.score >= eval)
            || (entry.score_bound == Score_Bound_Type::UPPER_BOUND
                && entry.score <= eval));
}

inline bool
Search_Engine::should_update_correction_history(Chess_Move best_move,
                                                Score      best_score,
                                                Score      static_evaluation,
                                                Score_Bound_Type score_bound,
                                                bool is_side_to_move_in_check)
{
    return (best_move.is_quiet_move()
            && ((score_bound == Score_Bound_Type::EXACT)
                || ((score_bound == Score_Bound_Type::UPPER_BOUND)
                    && (best_score < static_evaluation))
                || ((score_bound == Score_Bound_Type::LOWER_BOUND)
                    && (best_score > static_evaluation)))
            && (!is_side_to_move_in_check));
}

inline bool Search_Engine::should_update_quiet_continuation_history(
    const Chess_Move&      beta_cutoff_move,
    const Score_Bound_Type score_bound)
{
    return (beta_cutoff_move.is_quiet_move()
            && (score_bound == Score_Bound_Type::LOWER_BOUND));
}

inline bool Search_Engine::should_update_capture_continuation_history(
    const Chess_Move&      beta_cutoff_move,
    const Score_Bound_Type score_bound)
{
    return (beta_cutoff_move.is_capture
            && (score_bound == Score_Bound_Type::LOWER_BOUND));
}

inline bool Search_Engine::should_do_see_pruning(const Chess_Move& move,
                                                 const Score       best_score)
{
    // IMPORTANT: All move loop pruning should have the condition of
    // !best_score.is_enemy_mate().
    return (move.is_capture && (!best_score.is_enemy_mate()));
}
