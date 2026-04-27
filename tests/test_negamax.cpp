#include "evaluate.hpp"
#include "evaluation_terms.hpp"
#include "gtest/gtest.h"
#include "search.hpp"

TEST(negamax, mating)
{
    constexpr std::string_view FENS[] = {
        "8/8/8/8/8/4K3/R7/Q3k3 b - - 0 1",
        "4r2k/1p3rbp/2p1N1p1/p3n3/P2NB1nq/1P6/4R1P1/B1Q2RK1 b - - 4 32",
        "4r3/1pp2rbk/6pn/4n3/P3BN1q/1PB2bPP/8/2Q1RRK1 b - - 0 31",
        "4r1k1/4r1p1/8/p2R1P1K/5P1P/1QP3q1/1P6/3R4 b - - 0 1"};

    Chess_Board cb;

    // Arbitrary search constraints.
    Search_Constraints constraints;
    constraints.time_controls[PIECE_COLOR::WHITE].time_remaining = 150000;
    constraints.time_controls[PIECE_COLOR::WHITE].increment      = 1500;
    constraints.time_controls[PIECE_COLOR::BLACK].time_remaining = 150000;
    constraints.time_controls[PIECE_COLOR::BLACK].increment      = 1500;

    // Note: The maximum fen index is 3 otherwise, search is too slow to
    // complete upto mate distance depth search.
    for (uint8_t fen_idx = 0; fen_idx < 3; fen_idx++)
    {
        cb.set_from_fen(std::string(FENS[fen_idx]));
        Search_Engine  search(cb, constraints);
        const uint16_t distance_to_mate = (NUM_OF_PLAYERS * fen_idx);
        const Search_Engine_Result search_result =
            search.negamax(cb, distance_to_mate);
        if (fen_idx == 0)
        { // Side to move is in checkmate.
            EXPECT_EQ(search_result.second.to_int(), FP_LOSING_MATE_MIN);
            continue;
        }
        EXPECT_EQ(search_result.second.to_int(),
                  (FP_WINNING_MATE_MAX
                   - Matrex_FP_Int::from_integer(
                         static_cast<Fixed_Point_Int_Storage_Type>(
                             distance_to_mate - 1))
                         .get_value()));
        EXPECT_TRUE(search_result.second.is_mating_score());
        EXPECT_EQ(search_result.second.mate_in(), (distance_to_mate - 1));
    }
}

TEST(negamax, consistent_scoring)
{
    constexpr uint16_t SEARCH_DEPTH = 5;

    // Arbitrary search constraints.
    Search_Constraints constraints;
    constraints.time_controls[PIECE_COLOR::WHITE].time_remaining = 150000;
    constraints.time_controls[PIECE_COLOR::WHITE].increment      = 1500;
    constraints.time_controls[PIECE_COLOR::BLACK].time_remaining = 150000;
    constraints.time_controls[PIECE_COLOR::BLACK].increment      = 1500;

    Chess_Board cb;
    cb.set_from_fen("4k3/8/8/8/8/8/4PP2/4K3 w - - 0 1");

    Search_Engine              first_search(cb, constraints);
    const Search_Engine_Result first_search_result =
        first_search.negamax(cb, SEARCH_DEPTH);

    cb.make_move(first_search_result.first);

    Search_Engine              second_search(cb, constraints);
    const Search_Engine_Result second_search_result =
        second_search.negamax(cb, SEARCH_DEPTH - 1);

    // The score from the first search made at depth D must be the negated score
    // of a second search (made after making the best move) made at depth (D-1).
    ASSERT_EQ(first_search_result.second.to_int(),
              -second_search_result.second.to_int());
}

TEST(negamax, DISABLED_debug)
{
    constexpr std::string_view FEN =
        "r1bqk2r/2p1b3/2pn1ppp/p7/5B2/2N1QN2/PPP1KPPP/R6R w kq - 2 14";

    Chess_Board cb;
    cb.set_from_fen(std::string(FEN));

    // Arbitrary search constraints.
    Search_Constraints constraints;
    constraints.time_controls[PIECE_COLOR::WHITE].time_remaining = 15000;
    constraints.time_controls[PIECE_COLOR::WHITE].increment      = 1500;
    constraints.time_controls[PIECE_COLOR::BLACK].time_remaining = 15000;
    constraints.time_controls[PIECE_COLOR::BLACK].increment      = 1500;

    Search_Engine              search(cb, constraints);
    const Search_Engine_Result search_result = search.search();
    std::cout << "Best move: "
              << search_result.first.to_coordinate_notation(false)
              << " Score: " << search_result.second.to_int()
              << " Is mating score?: "
              << (search_result.second.is_friendly_mate()
                  || search_result.second.is_enemy_mate())
              << std::endl;

    const PIECE_COLOR     moving_side = cb.get_side_to_move();
    Chess_Move_List       moving_side_moves_list;
    Moves_Bitboard_Matrix moving_side_matrix;
    Move_Generator        mg_moving_side(cb);
    mg_moving_side.generate_all_moves<MOVE_GENERATION_TYPE::ALL>(
        moving_side,
        moving_side_moves_list,
        moving_side_matrix);

    const PIECE_COLOR opposing_side =
        (PIECE_COLOR) ((~cb.get_side_to_move()) & 0x1);
    Chess_Move_List       opposing_side_moves_list;
    Moves_Bitboard_Matrix opposing_side_matrix;
    Move_Generator        mg_opposing_side(cb);
    mg_opposing_side.generate_all_moves<MOVE_GENERATION_TYPE::ALL>(
        opposing_side,
        opposing_side_moves_list,
        opposing_side_matrix);

    Evaluator   e(TUNED_EVALUATION_WEIGHTS,
                cb,
                moving_side_matrix,
                opposing_side_matrix);
    const Score evaluation = e.evaluate();
    std::cout << "Evaluation: " << evaluation.to_int() << std::endl;
}
