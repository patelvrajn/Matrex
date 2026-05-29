#include "gtest/gtest.h"
#include "static_exchange_evaluation.hpp"

TEST(see_tests, simple_capture)
{
    Chess_Board position;
    position.set_from_fen("3k4/7p/8/8/8/8/8/3K3R w - - 0 1");
    Static_Exchange_Evaluator<int64_t> see(position);
    ASSERT_EQ(see.evaluate(Square(ESQUARE::H7), PIECES::ROOK, 1), 10);
}

TEST(see_tests, all_piece_type_exchange_without_kings)
{
    Chess_Board position;
    position.set_from_fen("2kr4/8/5q2/1n2b3/3P4/2B2N2/1Q6/3RK3 b - - 0 1");
    Static_Exchange_Evaluator<int64_t> see(position);
    ASSERT_EQ(see.evaluate(Square(ESQUARE::D4), PIECES::KNIGHT, 1), -80);
}

TEST(see_tests, all_piece_type_exchange)
{
    Chess_Board position;
    position.set_from_fen("3r4/8/5q2/1n2b3/2kP4/2B1KN2/1Q6/3R4 b - - 0 1");
    Static_Exchange_Evaluator<int64_t> see(position);
    ASSERT_EQ(see.evaluate(Square(ESQUARE::D4), PIECES::KNIGHT, 1), -80);
}

TEST(see_tests, pawn_attack)
{
    Chess_Board position;
    position.set_from_fen(
        "r1bqk2r/pppp1pp1/6n1/2bPp2p/3QP1n1/2N2N2/PPP1BPPP/R1B2RK1 b kq - 0 1");
    Static_Exchange_Evaluator<int64_t> see(position);
    ASSERT_EQ(see.evaluate(Square(ESQUARE::D4), PIECES::PAWN, 1), 110);
}
