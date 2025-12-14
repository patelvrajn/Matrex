#include "search.hpp"

#include "evaluate.hpp"

Search_Engine::Search_Engine()
    : m_timer_expired_during_search(false), m_num_of_nodes_searched(0) {}

void Search_Engine::new_game() { m_transposition_table.clear(); }

Search_Engine_Result Search_Engine::search(
    const Chess_Board& cb, const Search_Constraints& constraints) {
  m_chess_board = cb;
  m_transposition_table.resize(constraints.transposition_table_size);
  m_constraints = constraints;
  m_my_side = cb.get_side_to_move();
  m_num_of_nodes_searched = 0;
  m_timer_expired_during_search = false;

  return iterative_deepening();
}

Search_Engine_Result Search_Engine::negamax(Chess_Board& position,
                                            uint16_t depth, uint16_t ply,
                                            Score alpha, Score beta) {
  const Zobrist_Hash position_z_hash = position.get_zobrist_hash();
  Transposition_Table_Entry transposition_table_entry;
  const bool did_transposition_table_hit =
      m_transposition_table.read(position_z_hash, transposition_table_entry);

  // Transposition table cutoff - use the stored best move and score if it
  // satisfies the conditions.
  if (should_use_transposition_table_score(did_transposition_table_hit, depth,
                                           transposition_table_entry, alpha,
                                           beta)) {
    return {transposition_table_entry.best_move,
            transposition_table_entry.score};
  }

  // Assume that the score bound for a position's score to be stored in the
  // transposition table is an upper bound (or <= alpha) until we find out
  // otherwise.
  Score_Bound_Type score_bound = Score_Bound_Type::UPPER_BOUND;

  Move_Ordering mo(position, transposition_table_entry.best_move);
  mo.generate_moves<MOVE_GENERATION_TYPE::ALL>();
  Chess_Move_List& moves = mo.get_sorted_moves();

  // No legal moves available, return the appropriate mate or draw score.
  if (moves.get_max_index() == -1) {
    m_num_of_nodes_searched++;
    const Score mate_score = get_mate_score(mo, ply);

    // Cache the position's mate evaluation in the transposition table.
    transposition_table_entry = {
        .partial_zobrist =
            Transposition_Table::get_partial_zobrist(position_z_hash),
        .age = 0,
        .best_move = Chess_Move(),  // No best move in mate positions.
        .score = mate_score,
        .depth = depth,
        .score_bound =
            Score_Bound_Type::EXACT,  // Mate scores are always exact.
    };
    m_transposition_table.write(position_z_hash, transposition_table_entry);

    return {Chess_Move(), mate_score};
  }

  // Base case: if depth is 0, perform quiescence search.
  if (depth == 0) {
    return quiescence(position, ply, alpha, beta);
  }

  m_num_of_nodes_searched++;

  // Check if time has expired during the search.
  m_timer_expired_during_search = m_timer.is_search_time_expired(
      m_constraints.time_controls[m_my_side].time_remaining,
      m_constraints.time_controls[m_my_side].increment);

  // When time expires return beta because it will just be alpha of the parent
  // as the child score and this way it doesn't affect the best move of the
  // parent.
  if (m_timer_expired_during_search) {
    return {moves[0], beta};
  }

  Chess_Move best_move = Chess_Move();
  Score best_score = Score(ESCORE::NEGATIVE_INFINITY);

  for (const Chess_Move& move : moves) {
    // Explore the child move's subtree for it's evaluation. Negate the result
    // to compare it's score to the parent's scores (alpha, evaluation, etc).
    const Undo_Chess_Move undo_move = position.make_move(move);
    m_transposition_table.prefetch(position.get_zobrist_hash());
    const Search_Engine_Result child_result =
        negamax(position, (depth - 1), (ply + 1), -beta, -alpha);
    const Score child_score = -child_result.second;
    position.undo_move(undo_move);

    // Update alpha if the child's score is better than the current alpha. All
    // nodes in negamax are looking to maximize their alpha value. If the score
    // is greater than alpha and assuming it doesn't cause a beta cutoff, then
    // the score is exact because it falls between the invariant;
    // alpha < score < beta.
    if (child_score > alpha) {
      score_bound = Score_Bound_Type::EXACT;
      alpha = child_score;
    }

    // Update the best score and best move found so far at this node even if
    // the child is expected to cause pruning because the information that this
    // node caused a beta cutoff is still needed for the parent node's move.
    if (child_score > best_score) {
      best_score = child_score;
      best_move = move;
    }

    // When alpha of the parent becomes greater than or equal to beta, a beta
    // cutoff (fail-high) or pruning of the node is needed because the opponent
    // will never allow this sequence moves to occur In this case, we don't
    // consider further children because alpha can only become higher not lower.
    // Note, that the equal sign in the pruning condition is needed because:
    //
    //  We know b_c = -a_p (property essential to Negamax) and assume that the
    //  pruning condition (with the equal sign) is met i.e. a_c >= b_c.
    //
    //  Then the following is true:
    //    a_c >= -a_p
    //    -a_c <= a_p
    //  Which means a_p will not change in a_p = max(a_p, -negamax(...)) because
    //  the returned value from the child (v) is at least beta so:
    //    v >= b_c = -a_p
    //    parent_score (p) = -v <= -b_c
    //    p <= -(-a_p)
    //    p <= a_p
    //  Since, the parent is looking to improve it's alpha, fully evaluating the
    //  pruned child is futile. This logic holds even if we assumed a_c > b_c
    //  thus, the equal sign eliminates further cases of futile work.
    //
    // (Credit to Tobi/toanth in the Engine Programming Discord)
    //
    // When the pruning condition is met, the score is a lower bound on what the
    // score could of been if the other children were not pruned.
    if (alpha >= beta) {
      score_bound = Score_Bound_Type::LOWER_BOUND;
      break;
    }
  }

  // Cache the position's best move and evaluation in the transposition table.
  transposition_table_entry = {
      .partial_zobrist =
          Transposition_Table::get_partial_zobrist(position_z_hash),
      .age = 0,
      .best_move = best_move,
      .score = best_score,
      .depth = depth,
      .score_bound = score_bound,
  };
  m_transposition_table.write(position_z_hash, transposition_table_entry);

  return {best_move, best_score};
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
//    2. Only tactical moves are generated every iteration unless we are in
//    check.
//    3. There is no depth limit.
Search_Engine_Result Search_Engine::quiescence(Chess_Board& position,
                                               uint16_t ply, Score alpha,
                                               Score beta) {
  const Zobrist_Hash position_z_hash = position.get_zobrist_hash();
  Transposition_Table_Entry transposition_table_entry;
  const bool did_transposition_table_hit =
      m_transposition_table.read(position_z_hash, transposition_table_entry);

  // Transposition table cutoff - use the stored best move and score if it
  // satisfies the conditions. Note, quiescence search is a zero depth search -
  // it doesn't search all moves to depth.
  if (should_use_transposition_table_score(did_transposition_table_hit, 0,
                                           transposition_table_entry, alpha,
                                           beta)) {
    return {transposition_table_entry.best_move,
            transposition_table_entry.score};
  }

  // Assume that the score bound for a position's score to be stored in the
  // transposition table is an upper bound (or <= alpha) until we find out
  // otherwise.
  Score_Bound_Type score_bound = Score_Bound_Type::UPPER_BOUND;

  m_num_of_nodes_searched++;

  // Generate sorted tactical moves in the current position if not in check, if
  // in check, we need all moves because it is not guaranteed that at least one
  // tactical move is a check evasion move.
  Move_Ordering mo(position, transposition_table_entry.best_move);
  const bool is_side_to_move_in_check = mo.is_side_to_move_in_check();
  if (is_side_to_move_in_check) {
    mo.generate_moves<MOVE_GENERATION_TYPE::ALL>();
  } else {
    mo.generate_moves<MOVE_GENERATION_TYPE::TACTICAL>();
  }
  Chess_Move_List& moves = mo.get_sorted_moves();

  // No moves and in check - return mate score. Note, we don't handle stalemates
  // in quiescence search.
  if ((moves.get_max_index() == -1) && is_side_to_move_in_check) {
    const Score mate_score = get_mate_score(mo, ply);

    // Cache the position's mate evaluation in the transposition table.
    transposition_table_entry = {
        .partial_zobrist =
            Transposition_Table::get_partial_zobrist(position_z_hash),
        .age = 0,
        .best_move = Chess_Move(),  // No best move in mate positions.
        .score = mate_score,
        .depth = 0,
        .score_bound =
            Score_Bound_Type::EXACT,  // Mate scores are always exact.
    };
    m_transposition_table.write(position_z_hash, transposition_table_entry);

    return {Chess_Move(), mate_score};
  }

  const Evaluator e(position);
  Score stand_pat = e.evaluate();

  // Update stand pat evaluation based on a transposition table hit which would
  // most likely be based on a deeper search.
  if (should_use_transposition_table_score(did_transposition_table_hit, 0,
                                           transposition_table_entry,
                                           stand_pat)) {
    stand_pat = transposition_table_entry.score;
  }

  // No tactical moves - return the static evaluation (stand pat).
  if ((moves.get_max_index() == -1) && (!is_side_to_move_in_check)) {
    // Cache the position's mate evaluation in the transposition table.
    transposition_table_entry = {
        .partial_zobrist =
            Transposition_Table::get_partial_zobrist(position_z_hash),
        .age = 0,
        .best_move = Chess_Move(),  // No best move.
        .score = stand_pat,
        .depth = 0,
        .score_bound =
            Score_Bound_Type::EXACT,  // Static evaluations are always exact.
    };
    m_transposition_table.write(position_z_hash, transposition_table_entry);

    return {Chess_Move(), stand_pat};
  }

  Score best_score = Score(ESCORE::NEGATIVE_INFINITY);
  Chess_Move best_move = Chess_Move();

  if (!is_side_to_move_in_check) {  // Heuristic

    best_score = stand_pat;

    // Update alpha if the stand pat score is greater than alpha.
    if (best_score > alpha) {
      score_bound = Score_Bound_Type::EXACT;
      alpha = best_score;
    }

    // Alpha-beta pruning based on stand pat score.
    if (alpha >= beta) {
      // Cache the position's mate evaluation in the transposition table.
      transposition_table_entry = {
          .partial_zobrist =
              Transposition_Table::get_partial_zobrist(position_z_hash),
          .age = 0,
          .best_move = best_move,
          .score = best_score,
          .depth = 0,
          .score_bound =
              Score_Bound_Type::LOWER_BOUND,  // Scores at beta cutoffs are
                                              // always lower bounds.
      };
      m_transposition_table.write(position_z_hash, transposition_table_entry);

      return {best_move, best_score};
    }
  }

  for (const Chess_Move& move : moves) {
    // Explore the child move's subtree for it's evaluation. Negate the result
    // to compare it's score to the parent's scores (alpha, evaluation, etc).
    const Undo_Chess_Move undo_move = position.make_move(move);
    m_transposition_table.prefetch(position.get_zobrist_hash());
    const Search_Engine_Result child_result =
        quiescence(position, (ply + 1), -beta, -alpha);
    const Score child_score = -child_result.second;
    position.undo_move(undo_move);

    // Update best score and best move based on child's score.
    if (child_score > best_score) {
      best_score = child_score;
      best_move = move;
    }

    // Update alpha if the child's score is better than the alpha.
    if (child_score > alpha) {
      score_bound = Score_Bound_Type::EXACT;
      alpha = best_score;
    }

    // Alpha-beta pruning based on child's score.
    if (alpha >= beta) {
      score_bound = Score_Bound_Type::LOWER_BOUND;
      break;
    }
  }

  // Cache the position's best move and evaluation in the transposition table.
  transposition_table_entry = {
      .partial_zobrist =
          Transposition_Table::get_partial_zobrist(position_z_hash),
      .age = 0,
      .best_move = best_move,
      .score = best_score,
      .depth = 0,
      .score_bound = score_bound,
  };
  m_transposition_table.write(position_z_hash, transposition_table_entry);

  return {best_move, best_score};
}

Search_Engine_Result Search_Engine::iterative_deepening() {
  // Declare the best search result obtained.
  Search_Engine_Result best;

  // Iteratively increment the negamax search depth and start the search timer.
  m_timer.start();
  for (uint16_t current_depth = 1; current_depth < MAX_SEARCH_DEPTH;
       current_depth++) {
    Search_Engine_Result result = negamax(m_chess_board, current_depth);
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
