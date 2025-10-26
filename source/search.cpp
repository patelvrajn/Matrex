#include "search.hpp"

#include "evaluate.hpp"

Search_Engine::Search_Engine(const Chess_Board& cb) { m_chess_board = cb; }

Search_Engine_Result Search_Engine::negamax(double target_depth) {
  // If the target depth is just the root node - only return an evaluation of
  // the chess board.
  if (target_depth == 0) {
    Evaluator e(m_chess_board);
    return {Chess_Move(), e.evaluate()};
  }

  GAME_TREE_SEARCH_DIRECTION search_direction = DOWN;

  std::deque<Game_Tree_Node>* nodes = new std::deque<Game_Tree_Node>();

#define NODES_DEQUE (*nodes)
#define PARENT_NODE NODES_DEQUE[parent]
#define CHILD_NODE NODES_DEQUE[current_depth + 1]
#define CURRENT_NODE NODES_DEQUE[current_depth]

  // Push an initial node to be the parent of node at depth = 0. Note, now when
  // indexing nodes the index is (depth + 1).
  nodes->emplace_back();

  // Because we pushed the parent of depth 0 into nodes, start at depth 1.
  constexpr double DEPTH_FLOOR = 1;
  double current_depth = DEPTH_FLOOR;

  while (true) {
    double parent = current_depth - 1;

    // If nodes does not have node at the current depth, push a node.
    if (current_depth == nodes->size()) {
      nodes->emplace_back();
    }

    // If nodes does not have a child node at the current depth, push a node.
    if ((current_depth + 1) == nodes->size()) {
      nodes->emplace_back();
    }

    // We only reach the down state if we have reached an unexplored node in the
    // game tree.
    if (search_direction == DOWN) {
      // Generate all moves in the current position.
      Move_Generator mg(m_chess_board);
      Chess_Move_List moves;
      mg.generate_all_moves(moves);

      // We are at a node that is a parent.
      if ((current_depth - DEPTH_FLOOR) < target_depth) {
        // We have reached a node with no legal moves for the current side to
        // play. It is either checkmate or stalemate. Treat this node as a leaf
        // node.
        if (moves.get_max_index() == 0) {
          Score s = get_mate_score<DEPTH_FLOOR>(mg, current_depth);
          leaf_node_treatment(nodes, s, current_depth, parent,
                              search_direction);
          continue;
        }

        // Initialize this parent's attributes.
        CURRENT_NODE.children = moves;
        CURRENT_NODE.best_move = Chess_Move();
        CURRENT_NODE.best_score = Score(ESCORE::NEGATIVE_INFINITY);
        CURRENT_NODE.current_child_index = 0;

        // The parent's player is not this depth's player - swap and negate the
        // inherited alpha and beta.
        CURRENT_NODE.alpha = -PARENT_NODE.beta;
        CURRENT_NODE.beta = -PARENT_NODE.alpha;

        // Make the first child's move, storing the undo move structure in the
        // child node for any time we go back up a level in the game tree from
        // the child, and keep going down until we reach a leaf (depth-first).
        uint16_t move_index = CURRENT_NODE.current_child_index;
        CHILD_NODE.undo_move =
            m_chess_board.make_move(CURRENT_NODE.children[move_index]);
        current_depth = current_depth + 1;

        // Reached a leaf node.
      } else {
        Score leaf_score;

        // We have reached a node with no legal moves for the current side to
        // play. It is either checkmate or stalemate.
        if (moves.get_max_index() == 0) {
          leaf_score = get_mate_score<DEPTH_FLOOR>(mg, current_depth);
          // Not a checkmate or a stalemate, evaluate the leaf node normally.
        } else {
          Evaluator e(m_chess_board);
          leaf_score = e.evaluate();
        }

        leaf_node_treatment(nodes, leaf_score, current_depth, parent,
                            search_direction);
      }
    } else if (search_direction == UP) {
      // When alpha is greater than or equal to beta, a beta cutoff (fail-high)
      // or pruning of the node is needed. In this case, we do nothing but go up
      // the tree and continue searching nodes in depth-first order. Note, that
      // the equal sign in the pruning condition is needed because:
      //
      //  We know b_c = -a_p (property essential to Negamax) and assume that the
      //  pruning condition (with the equal sign) is met i.e. a_c >= b_c.
      //
      //  Then the following is true:
      //    a_c >= -a_p
      //    -a_c <= a_p
      //  Which means a_p will not change in a_p = max(a_p, -negamax(...))
      //  because the returned value from the child (v) is at least beta so:
      //    v >= b_c = -a_p
      //    parent_score (p) = -v <= -b_c
      //    p <= -(-a_p)
      //    p <= a_p
      //  Since, the parent is looking to improve it's alpha, fully evaluating
      //  the pruned child is futile. This logic holds even if we assumed
      //  a_c > b_c thus, the equal sign eliminates further cases of futile
      //  work.
      //
      // (Credit to Tobi/toanth in the Engine Programming Discord)
      const bool is_prune_time = CURRENT_NODE.alpha >= CURRENT_NODE.beta;
      if (is_prune_time) {
        if (current_depth <=
            DEPTH_FLOOR) {  // Can occur if alpha reaches infinity and beta
                            // doesn't find a better minimum.
          break;
        }
        m_chess_board.undo_move(CURRENT_NODE.undo_move);
        current_depth = parent;
        continue;
      }

      // This parent does not have anymore children to process.
      if (CURRENT_NODE.out_of_moves()) {
        // Get the best score for this node and negate it in order to compare
        // and equate it against the parent's scores.
        Score best_score = -CURRENT_NODE.best_score;

        // Update the best score of the parent based on this child's best score.
        if (best_score > PARENT_NODE.best_score) {
          PARENT_NODE.best_score = best_score;
          uint16_t move_index = PARENT_NODE.current_child_index;
          PARENT_NODE.best_move = PARENT_NODE.children[move_index];
        }

        // Update alpha of the parent based on the best score - all nodes in
        // Negamax are looking to maximize their alpha.
        if (PARENT_NODE.alpha < best_score) {
          PARENT_NODE.alpha = best_score;
        }

        // If the depth as reached the root, search is done otherwise, continue
        // up the tree looking for branches that haven't been evaluated in
        // depth-first order.
        if (current_depth <= DEPTH_FLOOR) {
          break;
        }
        m_chess_board.undo_move(CURRENT_NODE.undo_move);
        current_depth = parent;
        continue;
      }

      // Else, look at other children of this parent and move down to that
      // child.
      Chess_Move next_move = CURRENT_NODE.get_next_move();
      CHILD_NODE.undo_move = m_chess_board.make_move(next_move);
      search_direction = DOWN;
      current_depth = current_depth + 1;
    }
  }

  return {NODES_DEQUE[DEPTH_FLOOR].best_move,
          NODES_DEQUE[DEPTH_FLOOR].best_score};

#undef NODES_DEQUE
#undef PARENT_NODE
#undef CHILD_NODE
#undef CURRENT_NODE
}
