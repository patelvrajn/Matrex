#include "search.hpp"

#include "evaluate.hpp"
#include "evaluation_terms.hpp"
#include "static_exchange_evaluation.hpp"

Search_Engine::Search_Engine() :
    m_timer_expired_during_search(false), m_num_of_nodes_searched(0)
{
}

void Search_Engine::new_game()
{
#if COLLECT_TT_STATISTICS == 1
    m_transposition_table.get_statistics().print();
    m_transposition_table.clear_statistics();
#endif
    m_transposition_table.clear();
}

Search_Engine_Result
Search_Engine::search(const Chess_Board&        cb,
                      const Search_Constraints& constraints)
{
    m_chess_board                 = cb;
    m_constraints                 = constraints;
    m_my_side                     = cb.get_side_to_move();
    m_num_of_nodes_searched       = 0;
    m_timer_expired_during_search = false;

    m_transposition_table.resize(constraints.transposition_table_size);
    m_principal_variation.clear();

    return iterative_deepening();
}

Search_Engine_Result
Search_Engine::negamax(Chess_Board&              position,
                       uint16_t                  depth,
                       Principal_Variation_List& principal_variation,
                       uint16_t                  ply,
                       Score                     alpha,
                       Score                     beta)
{
    const uint32_t depth_squared = (depth * depth);

    // The parent's PV must be cleared between negamax calls because sibling
    // moves could influence each other.
    principal_variation.clear();

    bool is_three_fold_repetition = false;
    bool is_upcoming_repetition =
        m_cuckoo_rm_table.is_upcoming_repetition(position,
                                                 is_three_fold_repetition);

    if ((alpha < Score::from_int(ESCORE::DRAW)) && is_upcoming_repetition)
    {
        // If there is an upcoming repetition, the lowest score we can have is a
        // draw.
        alpha = Score::from_int(ESCORE::DRAW);

        // On fail-high, return the draw score even though we are in a fail-soft
        // framework.
        if (alpha >= beta) { return {Chess_Move(), alpha}; }
    }

    if (is_three_fold_repetition)
    {
        // If there is a three fold repetition, the score of the position is a
        // draw and we can return immediately without searching the position.
        return {Chess_Move(), Score::from_int(ESCORE::DRAW)};
    }

    // Note: Never cache draw by fifty move rule in the transposition table
    // because the Zobrist Hash implementation does not take into consideration
    // fifty move rule and a position can have a completely different evaluation
    // if the half move clock was not at it's maximum.
    if (position.is_draw_by_fifty_move_rule())
    {
        return {Chess_Move(), Score::from_int(ESCORE::DRAW)};
    }

    // Calling has_insufficient_mating_material() may be faster than writing to
    // the transposition table - also the concern is writing it to the
    // transposition table as an exact score but with a null move would result
    // in a possible illegal move.
    if (position.has_insufficient_mating_material())
    {
        return {Chess_Move(), Score::from_int(ESCORE::DRAW)};
    }

    const Zobrist_Hash        position_z_hash = position.get_zobrist_hash();
    Transposition_Table_Entry transposition_table_entry;
    const bool                did_transposition_table_hit =
        m_transposition_table.read(m_current_search_depth,
                                   position_z_hash,
                                   transposition_table_entry);

    // A principal variation node is any node that requires an alpha-beta window
    // wider than 1 because in principal variation search we only search the
    // best move from last iteration (PV node, the first move, presumably) with
    // a full window to get a baseline score. Assuming the first move is not an
    // exact score and is not the best move then we redo the search with a full
    // window looking for the PV node. If the first move is the PV node then we
    // only expect a PV_WINDOW_SIZE alpha-beta window for all other moves.
    int is_pv_node = ((beta - alpha).to_int() > PV_WINDOW_SIZE.get_value());

    // Transposition table cutoff - use the stored best move and score if it
    // satisfies the conditions.
    if (should_use_transposition_table_score(is_pv_node,
                                             did_transposition_table_hit,
                                             depth,
                                             transposition_table_entry,
                                             alpha,
                                             beta))
    {
        return {transposition_table_entry.best_move,
                transposition_table_entry.score};
    }

    // Assume that the score bound for a position's score to be stored in the
    // transposition table is an upper bound (or <= alpha) until we find out
    // otherwise.
    Score_Bound_Type score_bound = Score_Bound_Type::UPPER_BOUND;

    Move_Ordering mo(position, transposition_table_entry.best_move);
    mo.generate_moves<MOVE_GENERATION_TYPE::ALL>();
    Move_Generation_List& moves = mo.get_sorted_moves();

    // No legal moves available, return the appropriate mate or draw score.
    if (moves.get_max_index() == -1)
    {
        m_num_of_nodes_searched++;
        const Score mate_score = get_mate_score(mo, ply);

        // Cache the position's mate evaluation in the transposition table.
        transposition_table_entry = {
            .best_move = Chess_Move(), // No best move in mate positions
            .score     = mate_score,
            .partial_zobrist =
                Transposition_Table::get_partial_zobrist(position_z_hash),
            .depth = depth,
            .score_bound =
                Score_Bound_Type::EXACT // Mate scores are always exact.
        };
        m_transposition_table.write(m_current_search_depth,
                                    position_z_hash,
                                    transposition_table_entry);

        return {Chess_Move(), mate_score};
    }

    // Base case: if depth is 0, perform quiescence search.
    if (depth == QUIESCENCE_SEARCH_DEPTH)
    {
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
    if (m_timer_expired_during_search) { return {moves[0], beta}; }

    Principal_Variation_List child_principal_variation;
    Chess_Move               best_move  = Chess_Move();
    Score                    best_score = Score(FP_NEGATIVE_INFINITY);

    Static_Exchange_Evaluator<int64_t> see(position);

    bool is_first_move = true;
    for (const Chess_Move& move : moves)
    {
        // Static Exchange Evaluation Pruning (Captures Only)
        if (move.is_capture)
        {
            const auto see_evaluation =
                see.evaluate(move.destination_square, move.moving_piece, 1);

            if (see_evaluation < see.negamax_threshold(depth_squared))
            {
                continue;
            }
        }

        // Ensure each child has its own principal variation and is unaffected
        // by moves from the previous sibling.
        child_principal_variation.clear();

        // Explore the child move's subtree for it's evaluation. Negate the
        // result to compare it's score to the parent's scores (alpha,
        // evaluation, etc).
        const Undo_Chess_Move undo_move = position.make_move(move);
        // m_transposition_table.prefetch(position.get_zobrist_hash());

        Search_Engine_Result child_result = {best_move, best_score};

        if (is_first_move)
        {
            // Full alpha-beta window search for the first move which we assume
            // to be a PV node.
            child_result = negamax(position,
                                   (depth - 1),
                                   child_principal_variation,
                                   (ply + 1),
                                   -beta,
                                   -alpha);
        }
        else
        {
            // Search the presumably non-PV node with the narrowest window
            // around alpha since, we assume no other move will raise alpha.
            child_result = negamax(position,
                                   (depth - 1),
                                   child_principal_variation,
                                   (ply + 1),
                                   (-alpha - Score(PV_WINDOW_SIZE)),
                                   -alpha);

            const Score child_score = -child_result.second;

            // If the child result's score raised alpha and was within the full
            // alpha-beta window - redo the search because we found out that
            // the first move is not the PV node for this position. We only want
            // to redo the search if the current search is a full-window search
            // otherwise, we may do redundant searches for non-PV nodes.
            if (((child_score > alpha) && (child_score < beta)) && is_pv_node)
            {
                child_result = negamax(position,
                                       (depth - 1),
                                       child_principal_variation,
                                       (ply + 1),
                                       -beta,
                                       -alpha);
            }
        }

        const Score child_score = -child_result.second;
        position.undo_move(undo_move);

        bool is_child_score_better_than_alpha = child_score > alpha;

        // Update alpha if the child's score is better than the current alpha.
        // All nodes in negamax are looking to maximize their alpha value. If
        // the score is greater than alpha and assuming it doesn't cause a beta
        // cutoff, then the score is exact because it falls between the
        // invariant; alpha < score < beta.
        if (is_child_score_better_than_alpha)
        {
            score_bound = Score_Bound_Type::EXACT;
            alpha       = child_score;
        }

        // Update the best score found so far at this node even if the child is
        // expected to cause pruning because the information that this node
        // caused a beta cutoff is still needed for the parent node's move.
        if (child_score > best_score) { best_score = child_score; }

        // When alpha of the parent becomes greater than or equal to beta, a
        // beta cutoff (fail-high) or pruning of the node is needed because the
        // opponent will never allow this sequence moves to occur In this case,
        // we don't consider further children because alpha can only become
        // higher not lower. Note, that the equal sign in the pruning condition
        // is needed because:
        //
        //  We know b_c = -a_p (property essential to Negamax) and assume that
        //  the pruning condition (with the equal sign) is met i.e. a_c >= b_c.
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
        //  the pruned child is futile. This logic holds even if we assumed a_c
        //  > b_c thus, the equal sign eliminates further cases of futile work.
        //
        // (Credit to Tobi/toanth in the Engine Programming Discord)
        //
        // When the pruning condition is met, the score is a lower bound on what
        // the score could of been if the other children were not pruned.
        if (alpha >= beta)
        {
            score_bound = Score_Bound_Type::LOWER_BOUND;
            break;
        }

        // If the child's score raised alpha and was within alpha < score <
        // beta, then the child's move is the new best move and a principal
        // variation move for the current ply.
        if (is_child_score_better_than_alpha)
        {
            best_move = move;
            principal_variation.push_and_copy(best_move,
                                              child_principal_variation);
        }

        is_first_move = false;
    }

    // Cache the position's best move and evaluation in the transposition table
    // regardless of time because principal variation search guarantees the next
    // move found is a better move.
    transposition_table_entry = {
        .best_move = best_move,
        .score     = best_score,
        .partial_zobrist =
            Transposition_Table::get_partial_zobrist(position_z_hash),
        .depth       = depth,
        .score_bound = score_bound};
    m_transposition_table.write(m_current_search_depth,
                                position_z_hash,
                                transposition_table_entry);

    // Fail-soft. Always return the calculated score and don't bound it between
    // the alpha-beta invariant.
    return {best_move, best_score};
}

// This function implements quiescence search. The idea of quiescence search
// is to avoid the horizon effect - the concept that a depth-limited search
// will be insufficient to play out the consequences of available tactical
// moves. The difference between quiescence search and basic negamax search
// is the following:
//    1. Stand pat score - we are statically evaluating every node (because
//    we need a way to escape the infinite search without having to go
//    through all the moves - if we only evaluated leaf nodes like Negamax
//    we would need to create the entire tree) while looking for cutoffs.
//    2. Only tactical moves are generated every iteration unless we are in
//    check.
//    3. There is no depth limit.
Search_Engine_Result Search_Engine::quiescence(Chess_Board& position,
                                               uint16_t     ply,
                                               Score        alpha,
                                               Score        beta)
{
    const Zobrist_Hash position_z_hash = position.get_zobrist_hash();

    Transposition_Table_Entry transposition_table_entry;
    const bool                did_transposition_table_hit =
        m_transposition_table.read(m_current_search_depth,
                                   position_z_hash,
                                   transposition_table_entry);

    // Transposition table cutoff - use the stored best move and score if it
    // satisfies the conditions. Note, quiescence search is a zero depth
    // search - it doesn't search all moves to depth.
    if (should_use_transposition_table_score(did_transposition_table_hit,
                                             QUIESCENCE_SEARCH_DEPTH,
                                             transposition_table_entry,
                                             alpha,
                                             beta))
    {
        return {transposition_table_entry.best_move,
                transposition_table_entry.score};
    }

    // Assume that the score bound for a position's score to be stored in
    // the transposition table is an upper bound (or <= alpha) until we find
    // out otherwise.
    Score_Bound_Type score_bound = Score_Bound_Type::UPPER_BOUND;

    m_num_of_nodes_searched++;

    // Generate sorted tactical moves in the current position if not in
    // check, if in check, we need all moves because it is not guaranteed
    // that at least one tactical move is a check evasion move.
    Move_Ordering mo(position, transposition_table_entry.best_move);
    const bool    is_side_to_move_in_check = mo.is_side_to_move_in_check();
    if (is_side_to_move_in_check)
    {
        mo.generate_moves<MOVE_GENERATION_TYPE::ALL>();
    }
    else
    {
        mo.generate_moves<MOVE_GENERATION_TYPE::TACTICAL>();
    }
    Move_Generation_List&  moves              = mo.get_sorted_moves();
    Moves_Bitboard_Matrix& moving_side_matrix = mo.get_moves_matrix();

    // Generate moves matrix for the opposing side for evaluation purposes.
    const PIECE_COLOR opposing_side =
        (PIECE_COLOR) ((~position.get_side_to_move()) & 0x1);
    Move_Generation_List  not_used_moves_list;
    Moves_Bitboard_Matrix opposing_side_matrix;
    Move_Generator        mg(position);
    mg.generate_all_moves<MOVE_GENERATION_TYPE::ALL>(opposing_side,
                                                     not_used_moves_list,
                                                     opposing_side_matrix);

    // No moves and in check - return mate score. Note, we don't handle
    // stalemates in quiescence search.
    if ((moves.get_max_index() == -1) && is_side_to_move_in_check)
    {
        const Score mate_score = get_mate_score(mo, ply);

        // Cache the position's mate evaluation in the transposition table.
        transposition_table_entry = {
            .best_move = Chess_Move(), // No best move in mate positions.
            .score     = mate_score,
            .partial_zobrist =
                Transposition_Table::get_partial_zobrist(position_z_hash),
            .depth = QUIESCENCE_SEARCH_DEPTH,
            .score_bound =
                Score_Bound_Type::EXACT // Mate scores are always exact.
        };
        m_transposition_table.write(m_current_search_depth,
                                    position_z_hash,
                                    transposition_table_entry);

        return {Chess_Move(), mate_score};
    }

    // Stand pat evaluation.
    const Evaluator e(TUNED_EVALUATION_WEIGHTS,
                      position,
                      moving_side_matrix,
                      opposing_side_matrix);

    Score stand_pat = e.evaluate();

    // Update stand pat evaluation based on a transposition table hit which
    // would most likely be based on a deeper search.
    if (should_use_transposition_table_score(did_transposition_table_hit,
                                             QUIESCENCE_SEARCH_DEPTH,
                                             transposition_table_entry,
                                             stand_pat))
    {
        stand_pat = transposition_table_entry.score;
    }

    // No tactical moves - return the static evaluation (stand pat).
    if ((moves.get_max_index() == -1) && (!is_side_to_move_in_check))
    {
        // Cache the position's stand pat evaluation in the transposition table.
        transposition_table_entry = {
            .best_move = Chess_Move(), // No best move.
            .score     = stand_pat,
            .partial_zobrist =
                Transposition_Table::get_partial_zobrist(position_z_hash),
            .depth = QUIESCENCE_SEARCH_DEPTH,
            .score_bound =
                Score_Bound_Type::EXACT // Static evaluations are always exact.
        };
        m_transposition_table.write(m_current_search_depth,
                                    position_z_hash,
                                    transposition_table_entry);

        // Note: Only quiescence search's evaluation is ever used, the move is
        // not propagated up negamax's search tree. Quiescence replaces static
        // evaluation - move selection happens at the parent node.
        return {Chess_Move(), stand_pat};
    }

    Score      best_score = Score(FP_NEGATIVE_INFINITY);
    Chess_Move best_move  = Chess_Move();

    if (!is_side_to_move_in_check) // Heuristic
    {
        best_score = stand_pat;

        // Update alpha if the stand pat score is greater than alpha.
        if (best_score > alpha)
        {
            score_bound = Score_Bound_Type::EXACT;
            alpha       = best_score;
        }

        // Alpha-beta pruning based on stand pat score.
        if (alpha >= beta)
        {
            // Cache the position's mate evaluation in the transposition
            // table.
            transposition_table_entry = {
                .best_move = best_move,
                .score     = best_score,
                .partial_zobrist =
                    Transposition_Table::get_partial_zobrist(position_z_hash),
                .depth = QUIESCENCE_SEARCH_DEPTH,
                .score_bound =
                    Score_Bound_Type::LOWER_BOUND // Scores at beta cutoffs are
                                                  // always lower bounds.
            };
            m_transposition_table.write(m_current_search_depth,
                                        position_z_hash,
                                        transposition_table_entry);

            return {best_move, best_score};
        }
    }

    Static_Exchange_Evaluator<int64_t> see(position);

    for (const Chess_Move& move : moves)
    {
        // Static Exchange Evaluation Pruning
        if (move.is_capture)
        {
            const auto see_evaluation =
                see.evaluate(move.destination_square, move.moving_piece, 1);

            if (see_evaluation < see.quiescence_threshold()) { continue; }
        }

        // Explore the child move's subtree for it's evaluation. Negate
        // the result to compare it's score to the parent's scores
        // (alpha, evaluation, etc).
        const Undo_Chess_Move undo_move = position.make_move(move);
        // m_transposition_table.prefetch(position.get_zobrist_hash());
        const Search_Engine_Result child_result =
            quiescence(position, (ply + 1), -beta, -alpha);
        const Score child_score = -child_result.second;
        position.undo_move(undo_move);

        bool is_child_score_better_than_alpha = child_score > alpha;

        // Update alpha if the child's score is better than the alpha.
        if (is_child_score_better_than_alpha)
        {
            score_bound = Score_Bound_Type::EXACT;
            alpha       = child_score;
        }

        // Update best score based on child's score.
        if (child_score > best_score) { best_score = child_score; }

        // Alpha-beta pruning based on child's score.
        if (alpha >= beta)
        {
            score_bound = Score_Bound_Type::LOWER_BOUND;
            break;
        }

        // A best move is found if the score is exact and it is greater than
        // alpha.
        if (is_child_score_better_than_alpha) { best_move = move; }
    }

    // Cache the position's best move and evaluation in the transposition table.
    transposition_table_entry = {
        .best_move = best_move,
        .score     = best_score,
        .partial_zobrist =
            Transposition_Table::get_partial_zobrist(position_z_hash),
        .depth       = QUIESCENCE_SEARCH_DEPTH,
        .score_bound = score_bound};
    m_transposition_table.write(m_current_search_depth,
                                position_z_hash,
                                transposition_table_entry);

    return {best_move, best_score};
}

Search_Engine_Result Search_Engine::iterative_deepening()
{
    // Declare the best search result obtained.
    Search_Engine_Result best;

    // Iteratively increment the negamax search depth and start the
    // search timer.
    m_timer.start();
    for (uint16_t current_depth = 1; current_depth < MAX_SEARCH_DEPTH;
         current_depth++)
    {
        m_current_search_depth = current_depth;
        Search_Engine_Result result =
            negamax(m_chess_board, current_depth, m_principal_variation);
        uint64_t current_time = m_timer.elapsed();

        UCI_Search_Information uci_search_info(m_current_search_depth,
                                               current_time,
                                               m_num_of_nodes_searched,
                                               m_principal_variation,
                                               result.second);
        std::cout << uci_search_info << std::endl;

        best = result;

        m_principal_variation.clear();

        // Only update best search result if the timer didn't expire
        // during the search. Otherwise, time has expired, break out
        // of iterative deepening loop.
        if (m_timer_expired_during_search) { break; }
    }

    return best;
}

const Transposition_Table_Statistics& Search_Engine::get_tt_statistics() const
{
#if COLLECT_TT_STATISTICS == 1
    return m_transposition_table.get_statistics();
#else
    static const Transposition_Table_Statistics empty_stats {};
    return empty_stats;
#endif
}
