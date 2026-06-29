#include "cuckoo_reversible_move_table.hpp"
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

    Chess_Board   cb;
    Search_Engine search_engine;

    // Arbitrary search constraints.
    Search_Constraints constraints;
    constraints.time_controls[PIECE_COLOR::WHITE].time_remaining = 150000;
    constraints.time_controls[PIECE_COLOR::WHITE].increment      = 1500;
    constraints.time_controls[PIECE_COLOR::BLACK].time_remaining = 150000;
    constraints.time_controls[PIECE_COLOR::BLACK].increment      = 1500;
    constraints.transposition_table_size                         = 64;

    // Note: The maximum fen index is 3 otherwise, search is too slow to
    // complete upto mate distance depth search.
    for (uint8_t fen_idx = 0; fen_idx < 3; fen_idx++)
    {
        cb.set_from_fen(std::string(FENS[fen_idx]));
        const uint16_t distance_to_mate = (NUM_OF_PLAYERS * fen_idx);
        const Search_Engine_Result search_result =
            search_engine.search(cb, constraints);
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

TEST(negamax, draw_detection)
{
    Search_Engine search_engine;

    // Arbitrary search constraints.
    Search_Constraints constraints;
    constraints.time_controls[PIECE_COLOR::WHITE].time_remaining = 150000;
    constraints.time_controls[PIECE_COLOR::WHITE].increment      = 1500;
    constraints.time_controls[PIECE_COLOR::BLACK].time_remaining = 150000;
    constraints.time_controls[PIECE_COLOR::BLACK].increment      = 1500;
    constraints.transposition_table_size                         = 64;

    Chess_Board cb;
    cb.set_from_fen(std::string(START_POSITION_FEN));
    cb.make_moves_from_string(
        "e2e4 e7e5 f1c4 f8c5 c4f1 c5f8 f1c4 f8c5 c4f1 c5f8",
        false);

    Cuckoo_RM_Table rm_table;

    bool is_three_fold_repetition = false;
    bool is_upcoming_repetition =
        rm_table.is_upcoming_repetition(cb, is_three_fold_repetition);

    Search_Engine_Result search_result = search_engine.search(cb, constraints);

    EXPECT_TRUE(search_result.second >= Score(0));

    EXPECT_FALSE(is_three_fold_repetition);
    EXPECT_TRUE(is_upcoming_repetition);

    cb.make_moves_from_string("f1c4", false);

    is_upcoming_repetition =
        rm_table.is_upcoming_repetition(cb, is_three_fold_repetition);

    search_result = search_engine.search(cb, constraints);

    EXPECT_TRUE(search_result.second.is_draw());

    EXPECT_TRUE(is_three_fold_repetition);
    EXPECT_FALSE(is_upcoming_repetition);

    // King v King is insufficient.
    cb.set_from_fen("8/8/3k4/8/8/4K3/8/8 w - - 0 1");
    EXPECT_TRUE(cb.has_insufficient_mating_material());

    search_result = search_engine.search(cb, constraints);

    EXPECT_TRUE(search_result.second.is_draw());

    // King and Bishop v King is insufficient.
    cb.set_from_fen("8/8/8/3k4/5B2/4K3/8/8 w - - 0 1");
    EXPECT_TRUE(cb.has_insufficient_mating_material());

    search_result = search_engine.search(cb, constraints);

    EXPECT_TRUE(search_result.second.is_draw());

    // King vs King and Knight is insufficient.
    cb.set_from_fen("8/8/8/3k1n2/8/5K2/8/8 w - - 0 1");
    EXPECT_TRUE(cb.has_insufficient_mating_material());

    search_result = search_engine.search(cb, constraints);

    EXPECT_TRUE(search_result.second.is_draw());

    // Kings with only light square bishops is insufficient.
    cb.set_from_fen("8/8/4b3/3k4/8/5K2/4B3/8 w - - 0 1");
    EXPECT_TRUE(cb.has_insufficient_mating_material());

    search_result = search_engine.search(cb, constraints);

    EXPECT_TRUE(search_result.second.is_draw());

    // Kings with only dark square bishops is insufficient.
    cb.set_from_fen("8/8/8/3kb3/8/5K2/5B2/8 w - - 0 1");
    EXPECT_TRUE(cb.has_insufficient_mating_material());

    search_result = search_engine.search(cb, constraints);

    EXPECT_TRUE(search_result.second.is_draw());

    cb.set_from_fen("8/8/8/3k3p/8/7P/8/6K1 w - - 99 1");
    cb.make_moves_from_string("g1h1", false);
    EXPECT_TRUE(cb.is_draw_by_fifty_move_rule());

    search_result = search_engine.search(cb, constraints);

    EXPECT_TRUE(search_result.second.is_draw());
}

TEST(negamax, DISABLED_debug)
{
    constexpr std::string_view FEN = "8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - - 0 1";

    Chess_Board cb;
    cb.set_from_fen(std::string(FEN));

    // Search constraints matching the fastchess constraints used during SPRT.
    Search_Constraints constraints;
    constraints.time_controls[PIECE_COLOR::WHITE].time_remaining = 8000;
    constraints.time_controls[PIECE_COLOR::WHITE].increment      = 80;
    constraints.time_controls[PIECE_COLOR::BLACK].time_remaining = 8000;
    constraints.time_controls[PIECE_COLOR::BLACK].increment      = 80;
    constraints.transposition_table_size                         = 128;

    Search_Engine              search;
    const Search_Engine_Result search_result = search.search(cb, constraints);
    std::cout << "Best move: "
              << search_result.first.to_coordinate_notation(false)
              << " Score: " << search_result.second.to_int()
              << " Is mating score?: "
              << (search_result.second.is_friendly_mate()
                  || search_result.second.is_enemy_mate())
              << std::endl;

    search.get_tt_statistics().print();

    const PIECE_COLOR     moving_side = cb.get_side_to_move();
    Move_Generation_List  moving_side_moves_list;
    Moves_Bitboard_Matrix moving_side_matrix;
    Move_Generator        mg_moving_side(cb);
    mg_moving_side.generate_all_moves<MOVE_GENERATION_TYPE::ALL>(
        moving_side,
        moving_side_moves_list,
        moving_side_matrix);

    const PIECE_COLOR opposing_side =
        (PIECE_COLOR) ((~cb.get_side_to_move()) & 0x1);
    Move_Generation_List  opposing_side_moves_list;
    Moves_Bitboard_Matrix opposing_side_matrix;
    Move_Generator        mg_opposing_side(cb);
    mg_opposing_side.generate_all_moves<MOVE_GENERATION_TYPE::ALL>(
        opposing_side,
        opposing_side_moves_list,
        opposing_side_matrix);

    constexpr std::size_t TEST_CORR_HIST_TABLE_SIZE = 16384;
    Correction_History_Tables<TEST_CORR_HIST_TABLE_SIZE> corr_hist_table;

    Evaluator   e(TUNED_EVALUATION_WEIGHTS,
                cb,
                moving_side_matrix,
                opposing_side_matrix);
    const Score evaluation = e.evaluate(corr_hist_table);
    std::cout << "Evaluation: " << evaluation.to_int() << std::endl;
}
