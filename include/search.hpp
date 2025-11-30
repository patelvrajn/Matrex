#pragma once

#include <array>

#include "chess_board.hpp"
#include "chess_move.hpp"
#include "move_generator.hpp"
#include "move_ordering.hpp"
#include "score.hpp"
#include "timer.hpp"
#include "transposition_table.hpp"

// 5898.5 is the theoritical maximum number of moves (2-ply) in a chess game.
constexpr uint16_t MAX_SEARCH_DEPTH = (5899 * NUM_OF_PLAYERS);

struct Time_Control {
  uint64_t time_remaining;
  uint64_t increment;
};

struct Search_Constraints {
  std::array<Time_Control, NUM_OF_PLAYERS> time_controls;
  uint64_t transposition_table_size;
};

typedef std::pair<Chess_Move, Score> Search_Engine_Result;

class Search_Engine {
 public:
  Search_Engine();

  void new_game();
  Search_Engine_Result search(const Chess_Board& cb,
                              const Search_Constraints& constraints);
  Search_Engine_Result negamax(Chess_Board& position, uint16_t depth,
                               uint16_t ply = 0,
                               Score alpha = Score(ESCORE::NEGATIVE_INFINITY),
                               Score beta = Score(ESCORE::POSITIVE_INFINITY));

 private:
  Chess_Board m_chess_board;
  Transposition_Table m_transposition_table;
  Search_Constraints m_constraints;
  PIECE_COLOR m_my_side;
  Timer m_timer;
  bool m_timer_expired_during_search;
  uint64_t m_num_of_nodes_searched;

  Search_Engine_Result quiescence(Chess_Board& position, uint16_t ply,
                                  Score alpha, Score beta);
  Search_Engine_Result iterative_deepening();

  inline Score get_mate_score(const Move_Ordering& mo, uint16_t ply);
  inline bool should_use_transposition_table_score(
      const bool is_hit, const uint16_t depth,
      const Transposition_Table_Entry entry, const Score alpha,
      const Score beta);
  inline bool should_use_transposition_table_score(
      const bool is_hit, const uint16_t depth,
      const Transposition_Table_Entry entry, const Score eval);
};

inline Score Search_Engine::get_mate_score(const Move_Ordering& mo,
                                           uint16_t ply) {
  Score mate_score;

  // The side to move is in check and has no legal moves means they are in a
  // losing mating net specifically a checkmate at this depth.
  if (mo.is_side_to_move_in_check()) {
    mate_score = Score::from_int(ESCORE::LOSING_MATE_MIN + ply);
  } else {
    mate_score = Score::from_int(ESCORE::DRAW);  // Stalemate.
  }

  return mate_score;
}

inline bool Search_Engine::should_use_transposition_table_score(
    const bool is_hit, const uint16_t depth,
    const Transposition_Table_Entry entry, const Score alpha,
    const Score beta) {
  return is_hit && (depth <= entry.depth) &&
         ((entry.score_bound == Score_Bound_Type::EXACT) ||
          (entry.score_bound == Score_Bound_Type::LOWER_BOUND &&
           entry.score >= beta) ||
          (entry.score_bound == Score_Bound_Type::UPPER_BOUND &&
           entry.score <= alpha));
}

inline bool Search_Engine::should_use_transposition_table_score(
    const bool is_hit, const uint16_t depth,
    const Transposition_Table_Entry entry, const Score eval) {
  return is_hit && (depth <= entry.depth) &&
         ((entry.score_bound == Score_Bound_Type::EXACT) ||
          (entry.score_bound == Score_Bound_Type::LOWER_BOUND &&
           entry.score >= eval) ||
          (entry.score_bound == Score_Bound_Type::UPPER_BOUND &&
           entry.score <= eval));
}
