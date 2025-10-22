#pragma once

#include "chess_board.hpp"
#include "chess_move.hpp"
#include "score.hpp"

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
        best_score(Score()),
        alpha(Score(ESCORE::NEGATIVE_INFINITY)),
        beta(Score(ESCORE::POSITIVE_INFINITY)) {}

  bool out_of_moves() const {
    return (current_child_index >= (children.get_max_index() - 1));
  }

  Chess_Move get_next_move() {
    current_child_index = current_child_index + 1;
    return children[current_child_index];
  }
};

class Search_Engine {
 public:
  Search_Engine(const Chess_Board& cb);

  Search_Engine_Result negamax(double depth);

 private:
  Chess_Board m_chess_board;
};
