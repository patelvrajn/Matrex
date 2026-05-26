#include "gtest/gtest.h"
#include "static_exchange_evaluation.hpp"

TEST(static_exchange_evaluation, simple_capture)
{
    Chess_Board position;
    position.set_from_fen("3k4/7p/8/8/8/8/8/3K3R w - - 0 1");
    Static_Exchange_Evaluator<int64_t> see(position, Square(ESQUARE::H7));
    ASSERT_EQ(see.evaluate(), 10);
}
