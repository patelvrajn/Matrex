#pragma once

#include <array>

#include "chess_board.hpp"
#include "chess_move.hpp"
#include "move_generator.hpp"
#include "move_ordering.hpp"
#include "score.hpp"
#include "timer.hpp"

// 5898.5 is the theoritical maximum number of moves (2-ply) in a chess game.
constexpr uint16_t MAX_SEARCH_DEPTH = (5899 * NUM_OF_PLAYERS);

struct Time_Control {
  uint64_t time_remaining;
  uint64_t increment;
};

struct Search_Constraints {
  std::array<Time_Control, NUM_OF_PLAYERS> time_controls;
};

typedef std::pair<Chess_Move, Score> Search_Engine_Result;

class Search_Engine {
 public:
  Search_Engine(const Chess_Board& cb, const Search_Constraints& constraints);

  Search_Engine_Result search();
  Search_Engine_Result negamax(Chess_Board& position, uint16_t depth,
                               Score alpha = Score(ESCORE::NEGATIVE_INFINITY),
                               Score beta = Score(ESCORE::POSITIVE_INFINITY));

 private:
  Chess_Board m_chess_board;
  Search_Constraints m_constraints;
  PIECE_COLOR m_my_side;
  Timer m_timer;
  bool m_timer_expired_during_search;
  uint64_t m_num_of_nodes_searched;

  Search_Engine_Result iterative_deepening();

  template <uint16_t DEPTH_FLOOR>
  inline Score get_mate_score(const Move_Ordering& mo, uint16_t current_depth);
};

template <uint16_t DEPTH_FLOOR>
inline Score Search_Engine::get_mate_score(const Move_Ordering& mo,
                                           uint16_t current_depth) {
  Score mate_score;

  // The side to move is in check and has no legal moves means they are in a
  // losing mating net specifically a checkmate at this depth.
  if (mo.is_side_to_move_in_check()) {
    mate_score = Score::from_int(ESCORE::LOSING_MATE_MIN +
                                 (current_depth - DEPTH_FLOOR));
  } else {
    mate_score = Score::from_int(ESCORE::DRAW);  // Stalemate.
  }

  return mate_score;
}
