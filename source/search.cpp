#include "search.hpp"

#include "evaluate.hpp"

Search_Engine::Search_Engine(const Chess_Board& cb,
                             const Search_Constraints& constraints)
    : m_chess_board(cb),
      m_constraints(constraints),
      m_my_side(cb.get_side_to_move()),
      m_timer_expired_during_search(false),
      m_num_of_nodes_searched(0) {}

Search_Engine_Result Search_Engine::search() { return iterative_deepening(); }

Search_Engine_Result Search_Engine::negamax(uint16_t target_depth) {
  // If the target depth is just the root node - only return an evaluation of
  // the chess board.
  if (target_depth == 0) {
    const Evaluator e(m_chess_board);
    return {Chess_Move(), e.evaluate()};
  }

  GAME_TREE_SEARCH_DIRECTION search_direction = DOWN;

  // Create a fixed sized array of size (target_depth + 2). The +2 because the
  // first node, node[0] will act as the parent of the root of the game tree and
  // under the condition ((current_depth - DEPTH_FLOOR) < target_depth) which is
  // equivalent to current_depth <= target_depth (given they are integers), we
  // access the child of target_depth i.e. (target_depth + 1) so we need an
  // array sized from 0...(target_depth + 1).
  Game_Tree_Node* nodes = new Game_Tree_Node[(target_depth + 2)];

#define PARENT_NODE nodes[parent]
#define CHILD_NODE nodes[current_depth + 1]
#define CURRENT_NODE nodes[current_depth]

  // Because we created the parent of the root as nodes[0], start at depth 1.
  constexpr uint16_t DEPTH_FLOOR = 1;
  uint16_t current_depth = DEPTH_FLOOR;

  while (true) {
    m_timer_expired_during_search = m_timer.is_search_time_expired(
        m_constraints.time_controls[m_my_side].time_remaining,
        m_constraints.time_controls[m_my_side].increment);

    uint16_t parent = current_depth - 1;

    // We only reach the down state if we have reached an unexplored node in the
    // game tree.
    if (search_direction == DOWN) {
      // Time is up, do not do anything - just go back up the tree which will
      // propagate the best score so far back up the tree.
      if (m_timer_expired_during_search) {
        if (current_depth <= DEPTH_FLOOR) {
          break;
        }
        m_chess_board.undo_move(CURRENT_NODE.undo_move);
        current_depth = parent;
        search_direction = UP;
        continue;
      }

      // Count the number of nodes searched.
      m_num_of_nodes_searched++;

      // Generate all sorted moves in the current position.
      Move_Ordering mo(m_chess_board);
      mo.generate_moves<MOVE_GENERATION_TYPE::ALL>();
      Chess_Move_List moves = mo.get_sorted_moves();

      // We are at a node that is a parent.
      if ((current_depth - DEPTH_FLOOR) < target_depth) {
        // We have reached a node with no legal moves for the current side to
        // play. It is either checkmate or stalemate. Treat this node as a leaf
        // node.
        if (moves.get_max_index() == -1) {
          const Score s = get_mate_score<DEPTH_FLOOR>(mo, current_depth);
          if (current_depth <= DEPTH_FLOOR) {
            Search_Engine_Result return_value = {nodes[DEPTH_FLOOR].best_move,
                                                 s};
            delete[] nodes;
            return return_value;
          }
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
        const uint16_t move_index = CURRENT_NODE.current_child_index;
        CHILD_NODE.undo_move =
            m_chess_board.make_move(CURRENT_NODE.children[move_index]);
        current_depth = current_depth + 1;

        // Reached a leaf node.
      } else {
        Score leaf_score;

        // We have reached a node with no legal moves for the current side to
        // play. It is either checkmate or stalemate.
        if (moves.get_max_index() == -1) {
          leaf_score = get_mate_score<DEPTH_FLOOR>(mo, current_depth);
          // Not a checkmate or a stalemate, perform quiescence search.
        } else {
          leaf_score = quiescence(PARENT_NODE.alpha, PARENT_NODE.beta,
                                  (current_depth - DEPTH_FLOOR))
                           .second;
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
        if (current_depth <= DEPTH_FLOOR) {
          break;
        }
        m_chess_board.undo_move(CURRENT_NODE.undo_move);
        current_depth = parent;
        continue;
      }

      // This parent does not have anymore children to process or we are out of
      // time and can't process anymore children.
      if (CURRENT_NODE.out_of_moves() || m_timer_expired_during_search) {
        // If the depth as reached the root, search is done.
        if (current_depth <= DEPTH_FLOOR) {
          break;
        }

        // Get the best score for this node and negate it in order to compare
        // and equate it against the parent's scores.
        const Score best_score = -CURRENT_NODE.best_score;

        // Update the best score of the parent based on this child's best score.
        if (best_score > PARENT_NODE.best_score) {
          PARENT_NODE.best_score = best_score;
          const uint16_t move_index = PARENT_NODE.current_child_index;
          PARENT_NODE.best_move = PARENT_NODE.children[move_index];
        }

        // Update alpha of the parent based on the best score - all nodes in
        // Negamax are looking to maximize their alpha.
        if (PARENT_NODE.alpha < best_score) {
          PARENT_NODE.alpha = best_score;
        }

        // Continue up the tree looking for branches that haven't been evaluated
        // in depth-first order.
        m_chess_board.undo_move(CURRENT_NODE.undo_move);
        current_depth = parent;
        continue;
      }

      // Else, look at other children of this parent and move down to that
      // child.
      const Chess_Move next_move = CURRENT_NODE.get_next_move();
      CHILD_NODE.undo_move = m_chess_board.make_move(next_move);
      search_direction = DOWN;
      current_depth = current_depth + 1;
    }
  }

  Search_Engine_Result return_value = {nodes[DEPTH_FLOOR].best_move,
                                       nodes[DEPTH_FLOOR].best_score};

  delete[] nodes;

  return return_value;

#undef PARENT_NODE
#undef CHILD_NODE
#undef CURRENT_NODE
}

// This function implements quiescence search. The idea of quiescence search is
// to avoid the horizon effect - the concept that a depth-limited search will
// be insufficient to play out the consequences of available tactical moves. The
// difference between quiescence search and basic negamax search is the
// following:
//    1. Stand pat score - we are statically evaluating every node (because we
//    need a way to escape the infinite search without having to go through all
//    the moves - if we only evaluated leaf nodes like Negamax we would need to
//    create the entire tree) while looking for cutoffs.
//    2. Only tactical moves are generated every iteration.
//    3. There is no depth limit - the target depth for quiescence search in
//    this function is so we can allocate memory for the nodes array however, it
//    is high enough that realistically the search won't go beyond that depth.
Search_Engine_Result Search_Engine::quiescence(Score alpha, Score beta,
                                               uint16_t depth_from_negamax) {
  constexpr uint16_t TARGET_DEPTH = SEARCH_DEPTH_SOFT_LIMITATION;

  GAME_TREE_SEARCH_DIRECTION search_direction = DOWN;

  Game_Tree_Node nodes[(TARGET_DEPTH + 2)];

  // The parent of our root node inherits the alpha and beta parameters - so
  // that the root's alpha and beta are correct.
  nodes[0].alpha = alpha;
  nodes[0].beta = beta;

#define PARENT_NODE nodes[parent]
#define CHILD_NODE nodes[current_depth + 1]
#define CURRENT_NODE nodes[current_depth]

  // Because we created the parent of the root as nodes[0], start at depth 1.
  constexpr uint16_t DEPTH_FLOOR = 1;
  uint16_t current_depth = DEPTH_FLOOR;

  while (true) {
    uint16_t parent = current_depth - 1;

    // We only reach the down state if we have reached an unexplored node in the
    // game tree.
    if (search_direction == DOWN) {
      // Count the number of nodes searched.
      m_num_of_nodes_searched++;

      // Generate sorted tactical moves in the current position if not in check,
      // if in check, we need all moves because it is not guaranteed that at
      // least one tactical move is a check evasion move.
      Move_Ordering mo(m_chess_board);
      const bool is_side_to_move_in_check = mo.is_side_to_move_in_check();
      if (is_side_to_move_in_check) {
        mo.generate_moves<MOVE_GENERATION_TYPE::ALL>();
      } else {
        mo.generate_moves<MOVE_GENERATION_TYPE::TACTICAL>();
      }
      Chess_Move_List moves = mo.get_sorted_moves();

      // We have reached a node with no legal moves for the current side to
      // play and they are in check thus, it is checkmate. We cannot detect
      // stalemates efficiently in quiescence. Treat this node as a leaf node.
      // In addition, when we retrieve the mate score, the depth that negamax
      // searched to must be added on.
      if ((moves.get_max_index() == -1) && is_side_to_move_in_check) {
        const Score s = get_mate_score<DEPTH_FLOOR>(
            mo, (depth_from_negamax + current_depth));
        if (current_depth <= DEPTH_FLOOR) {
          return {nodes[DEPTH_FLOOR].best_move, s};
        }
        leaf_node_treatment(nodes, s, current_depth, parent, search_direction);
        continue;
      }

      // Evaluate all new positions.
      const Evaluator e(m_chess_board);
      Score evaluation = e.evaluate();

      // No tactical moves - just evaluate the position and treat this as a
      // leaf node.
      if ((moves.get_max_index() == -1) && (!is_side_to_move_in_check)) {
        if (current_depth <= DEPTH_FLOOR) {
          return {nodes[DEPTH_FLOOR].best_move, evaluation};
        }
        leaf_node_treatment(nodes, evaluation, current_depth, parent,
                            search_direction);
        continue;
      }

      // Initialize this node's attributes.
      CURRENT_NODE.children = moves;
      CURRENT_NODE.best_move = Chess_Move();
      CURRENT_NODE.best_score = Score(ESCORE::NEGATIVE_INFINITY);
      CURRENT_NODE.current_child_index = 0;

      // The parent's player is not this depth's player - swap and negate the
      // inherited alpha and beta.
      CURRENT_NODE.alpha = -PARENT_NODE.beta;
      CURRENT_NODE.beta = -PARENT_NODE.alpha;

      // If the evaluation is greater than or equal to this node's beta, prune
      // this node by going up the tree. Do this only when not in check
      // (heuristic known to gain elo).
      if (!is_side_to_move_in_check) {
        const bool is_prune_time = evaluation >= CURRENT_NODE.beta;
        if (is_prune_time) {
          if (current_depth <= DEPTH_FLOOR) {
            return {nodes[DEPTH_FLOOR].best_move, evaluation};
          }
          m_chess_board.undo_move(CURRENT_NODE.undo_move);
          current_depth = parent;
          search_direction = UP;
          continue;
        }

        // Update this node's alpha based on the evaluation.
        if (evaluation > CURRENT_NODE.alpha) {
          CURRENT_NODE.alpha = evaluation;
        }
      }

      // Make the first child's move, storing the undo move structure in the
      // child node for any time we go back up a level in the game tree from
      // the child, and keep going down until we reach a leaf (depth-first).
      const uint16_t move_index = CURRENT_NODE.current_child_index;
      CHILD_NODE.undo_move =
          m_chess_board.make_move(CURRENT_NODE.children[move_index]);
      current_depth = current_depth + 1;

    } else if (search_direction == UP) {
      // When alpha is greater than or equal to beta, a beta cutoff (fail-high)
      // or pruning of the node is needed. In this case, we do nothing but go up
      // the tree and continue searching nodes in depth-first order.
      const bool is_prune_time = CURRENT_NODE.alpha >= CURRENT_NODE.beta;
      if (is_prune_time) {
        if (current_depth <= DEPTH_FLOOR) {
          break;
        }
        m_chess_board.undo_move(CURRENT_NODE.undo_move);
        current_depth = parent;
        continue;
      }

      // This parent does not have anymore children to process or we are out of
      // time and can't process anymore children.
      if (CURRENT_NODE.out_of_moves()) {
        // If the depth as reached the root, search is done.
        if (current_depth <= DEPTH_FLOOR) {
          break;
        }

        // Get the best score for this node and negate it in order to compare
        // and equate it against the parent's scores.
        const Score best_score = -CURRENT_NODE.best_score;

        // Update the best score of the parent based on this child's best score.
        if (best_score > PARENT_NODE.best_score) {
          PARENT_NODE.best_score = best_score;
          const uint16_t move_index = PARENT_NODE.current_child_index;
          PARENT_NODE.best_move = PARENT_NODE.children[move_index];
        }

        // Update alpha of the parent based on the best score - all nodes in
        // Negamax are looking to maximize their alpha.
        if (PARENT_NODE.alpha < best_score) {
          PARENT_NODE.alpha = best_score;
        }

        // Continue up the tree looking for branches that haven't been evaluated
        // in depth-first order.
        m_chess_board.undo_move(CURRENT_NODE.undo_move);
        current_depth = parent;
        continue;
      }

      // Else, look at other children of this parent and move down to that
      // child.
      const Chess_Move next_move = CURRENT_NODE.get_next_move();
      CHILD_NODE.undo_move = m_chess_board.make_move(next_move);
      search_direction = DOWN;
      current_depth = current_depth + 1;
    }
  }

  return {nodes[DEPTH_FLOOR].best_move, nodes[DEPTH_FLOOR].best_score};

#undef PARENT_NODE
#undef CHILD_NODE
#undef CURRENT_NODE
}

Search_Engine_Result Search_Engine::iterative_deepening() {
  // Declare the best search result obtained.
  Search_Engine_Result best;

  // Iteratively increment the negamax search depth and start the search timer.
  m_timer.start();
  for (uint16_t current_depth = 1; current_depth < THEORETICAL_MAX_SEARCH_DEPTH;
       current_depth++) {
    Search_Engine_Result result = negamax(current_depth);
    // Only update best search result if the timer didn't expire during the
    // search. Otherwise, time has expired, break out of iterative deepening
    // loop.
    if (!m_timer_expired_during_search) {
      best = result;
    } else {
      break;
    }
  }

  return best;
}
