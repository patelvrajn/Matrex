#include "gtest/gtest.h"
#include "static_exchange_evaluation.hpp"

TEST(see_tests, simple_capture)
{
    Chess_Board position;
    position.set_from_fen("3k4/7p/8/8/8/8/8/3K3R w - - 0 1");
    Static_Exchange_Evaluator<int64_t> see(position, Square(ESQUARE::H7));
    ASSERT_EQ(see.evaluate(), 10);
}

TEST(see_tests, defended_piece_with_attacking_king)
{
    Chess_Board position;
    position.set_from_fen("3k4/r6p/7K/8/8/8/8/8 w - - 0 1");
    Static_Exchange_Evaluator<int64_t> see(position, Square(ESQUARE::H7));
    ASSERT_EQ(see.evaluate(), 0);
}

TEST(see_tests, all_piece_type_exchange_without_kings)
{
    Chess_Board position;
    position.set_from_fen("2kr4/8/5q2/1n2b3/3P4/2B2N2/1Q6/3RK3 b - - 0 1");
    Static_Exchange_Evaluator<int64_t> see(position, Square(ESQUARE::D4));
    ASSERT_EQ(see.evaluate(), -80);
}

TEST(see_tests, all_piece_type_exchange)
{
    Chess_Board position;
    position.set_from_fen("3r4/8/5q2/1n2b3/2kP4/2B1KN2/1Q6/3R4 b - - 0 1");
    Static_Exchange_Evaluator<int64_t> see(position, Square(ESQUARE::D4));
    ASSERT_EQ(see.evaluate(), -90);
}
