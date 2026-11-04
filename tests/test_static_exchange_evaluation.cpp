#include "gtest/gtest.h"
#include "static_exchange_evaluation.hpp"

TEST(see_tests, simple_capture)
{
    Chess_Board position;
    position.set_from_fen("3k4/7p/8/8/8/8/8/3K3R w - - 0 1");
    Static_Exchange_Evaluator<int64_t> see(position);

    Chess_Move move         = Chess_Move::null_move();
    move.source_square      = ESQUARE::H1;
    move.destination_square = ESQUARE::H7;

    ASSERT_EQ(see.evaluate(move, 1), 10);
}

TEST(see_tests, all_piece_type_exchange_without_kings)
{
    Chess_Board position;
    position.set_from_fen("2kr4/8/5q2/1n2b3/3P4/2B2N2/1Q6/3RK3 b - - 0 1");
    Static_Exchange_Evaluator<int64_t> see(position);

    Chess_Move move         = Chess_Move::null_move();
    move.source_square      = ESQUARE::B5;
    move.destination_square = ESQUARE::D4;

    ASSERT_EQ(see.evaluate(move, 1), -20);
}

TEST(see_tests, all_piece_type_exchange)
{
    Chess_Board position;
    position.set_from_fen("3r4/8/5q2/1n2b3/2kP4/2B1KN2/1Q6/3R4 b - - 0 1");
    Static_Exchange_Evaluator<int64_t> see(position);

    Chess_Move move         = Chess_Move::null_move();
    move.source_square      = ESQUARE::B5;
    move.destination_square = ESQUARE::D4;

    ASSERT_EQ(see.evaluate(move, 1), -20);
}

TEST(see_tests, pawn_attack)
{
    Chess_Board position;
    position.set_from_fen(
        "r1bqk2r/pppp1pp1/6n1/2bPp2p/3QP1n1/2N2N2/PPP1BPPP/R1B2RK1 b kq - 0 1");
    Static_Exchange_Evaluator<int64_t> see(position);

    Chess_Move move         = Chess_Move::null_move();
    move.source_square      = ESQUARE::E5;
    move.destination_square = ESQUARE::D4;

    ASSERT_EQ(see.evaluate(move, 1), 90);
}

TEST(see_tests, this_side_has_more_attackers)
{
    Chess_Board position;
    position.set_from_fen(
        "rn1qkbnr/ppp1pppp/4b3/3p4/4P3/1BN5/PPPP1PPP/R1BQK1NR w KQkq - 0 1");
    Static_Exchange_Evaluator<int64_t> see(position);

    Chess_Move move         = Chess_Move::null_move();
    move.source_square      = ESQUARE::E4;
    move.destination_square = ESQUARE::D5;

    EXPECT_EQ(see.evaluate(move, 1), 10);
}

TEST(see_tests, king_is_always_last_attacker)
{
    Chess_Board position;
    position.set_from_fen(
        "4r2k/1p3rbp/2p1N1p1/p3n3/P2NB1nq/1P6/4R1P1/B1Q2RK1 b - - 4 32");
    Static_Exchange_Evaluator<int64_t> see_one(position);

    Chess_Move move         = Chess_Move::null_move();
    move.source_square      = ESQUARE::F7;
    move.destination_square = ESQUARE::F1;

    ASSERT_EQ(see_one.evaluate(move, 1), 0);

    position.set_from_fen(
        "4r2k/1p4bp/2p1N1p1/p3n3/P2NB1nq/1P6/4R1P1/B1Q2rK1 w - - 0 33");
    Static_Exchange_Evaluator<int64_t> see_two(position);

    move.source_square      = ESQUARE::C1;
    move.destination_square = ESQUARE::F1;

    ASSERT_EQ(see_two.evaluate(move, 1), 50);
}

TEST(see_tests, king_cannot_capture_guarded_piece)
{
    Chess_Board position;
    position.set_from_fen(
        "4r2k/1p4rp/2p1N1p1/p7/P2BB1nq/1P3P2/4R3/2Q2RK1 w - - 1 35");
    Static_Exchange_Evaluator<int64_t> see(position);

    Chess_Move move         = Chess_Move::null_move();
    move.source_square      = ESQUARE::D4;
    move.destination_square = ESQUARE::G7;

    EXPECT_EQ(see.evaluate(move, 1), 50);
}

TEST(see_tests, a_cheaper_hidden_attacker)
{
    Chess_Board position;
    position.set_from_fen(
        "r4rk1/p1p3pp/bp1n1n2/5p2/4p3/3P2N1/PPB2PPP/R1B1QRK1 w - - 0 1");
    Static_Exchange_Evaluator<int64_t> see(position);

    Chess_Move move         = Chess_Move::null_move();
    move.source_square      = ESQUARE::D3;
    move.destination_square = ESQUARE::E4;

    EXPECT_EQ(see.evaluate(move, 1), 5);
}

TEST(see_tests, capture_with_promotion_partial_exchange)
{
    Chess_Board position;
    position.set_from_fen("3k2r1/5P1b/7N/8/8/8/8/3K4 w - - 0 1");
    Static_Exchange_Evaluator<int64_t> see(position);

    Chess_Move move         = Chess_Move::null_move();
    move.source_square      = ESQUARE::H6;
    move.destination_square = ESQUARE::G8;
    move.is_promotion       = true;

    EXPECT_EQ(see.evaluate(move, 1), 50);
}

TEST(see_tests, capture_with_promotion_optimal_full_exchange)
{
    Chess_Board position;
    position.set_from_fen("R5n1/4nP1b/1k6/8/8/8/K7/8 w - - 0 1");
    Static_Exchange_Evaluator<int64_t> see(position);

    Chess_Move move         = Chess_Move::null_move();
    move.source_square      = ESQUARE::A8;
    move.destination_square = ESQUARE::G8;
    move.is_promotion       = true;

    EXPECT_EQ(see.evaluate(move, 1), 0);
}
