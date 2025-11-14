#pragma once

#include <array>

#include "chess_board.hpp"
#include "chess_move.hpp"
#include "move_generator.hpp"
#include "move_ordering.hpp"
#include "score.hpp"
#include "timer.hpp"

// 5898.5 is the theoretical maximum number of moves (2-ply) in a chess game.
constexpr uint16_t THEORETICAL_MAX_SEARCH_DEPTH = (5899 * NUM_OF_PLAYERS);

// Realistically, the search depth of the engine will never go beyond this
// depth.
constexpr uint16_t SEARCH_DEPTH_SOFT_LIMITATION = 256;

struct Time_Control {
  uint64_t time_remaining;
  uint64_t increment;
};

struct Search_Constraints {
  std::array<Time_Control, NUM_OF_PLAYERS> time_controls;
};

typedef std::pair<Chess_Move, Score> Search_Engine_Result;

enum GAME_TREE_SEARCH_DIRECTION { UP, DOWN };

struct Game_Tree_Node {
  Chess_Move_List children;
  Undo_Chess_Move undo_move;
  uint16_t current_child_index;
  Chess_Move best_move;
  Score best_score;
  Score alpha;
  Score beta;

  Game_Tree_Node()
      : current_child_index(0),
        best_score(Score(ESCORE::NEGATIVE_INFINITY)),
        alpha(Score(ESCORE::NEGATIVE_INFINITY)),
        beta(Score(ESCORE::POSITIVE_INFINITY)) {}

  bool out_of_moves() const {
    return (current_child_index >= children.get_max_index());
  }

  Chess_Move get_next_move() {
    current_child_index = current_child_index + 1;
    return children[current_child_index];
  }
};

class Search_Engine {
 public:
  Search_Engine(const Chess_Board& cb, const Search_Constraints& constraints);

  Search_Engine_Result search();
  Search_Engine_Result negamax(uint16_t depth);

 private:
  Chess_Board m_chess_board;
  Search_Constraints m_constraints;
  PIECE_COLOR m_my_side;
  Timer m_timer;
  bool m_timer_expired_during_search;
  uint64_t m_num_of_nodes_searched;

  Search_Engine_Result quiescence(Score alpha, Score beta,
                                  uint16_t depth_from_negamax);
  Search_Engine_Result iterative_deepening();

  inline void leaf_node_treatment(Game_Tree_Node* nodes, Score s,
                                  uint16_t& current_depth, uint16_t parent,
                                  GAME_TREE_SEARCH_DIRECTION& search_direction);
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

inline void Search_Engine::leaf_node_treatment(
    Game_Tree_Node* nodes, Score s, uint16_t& current_depth, uint16_t parent,
    GAME_TREE_SEARCH_DIRECTION& search_direction) {
  // Negate the given score it in order to compare and equate it against the
  // parent's scores.
  const Score leaf_score = -s;

#define PARENT_NODE nodes[parent]
#define CHILD_NODE nodes[current_depth + 1]
#define CURRENT_NODE nodes[current_depth]

  // Update parent's best score and best move.
  if (leaf_score > PARENT_NODE.best_score) {
    PARENT_NODE.best_score = leaf_score;
    const uint16_t move_index = PARENT_NODE.current_child_index;
    PARENT_NODE.best_move = PARENT_NODE.children[move_index];
  }

  // Since every level of the game tree is always from the maximizer's POV in
  // negamax alpha is always updated (like in minimax where the maximizer
  // updates alpha) to the maximum value. Beta updates implicitly because we
  // swap alpha and beta every level.
  if (PARENT_NODE.alpha < leaf_score) {
    PARENT_NODE.alpha = leaf_score;
  }

  // Go up from the leaf node, undoing its move first, and continue depth-first.
  m_chess_board.undo_move(CURRENT_NODE.undo_move);
  search_direction = UP;
  current_depth = parent;

#undef PARENT_NODE
#undef CHILD_NODE
#undef CURRENT_NODE
}
