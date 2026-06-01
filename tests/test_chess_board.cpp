#include "gtest/gtest.h"
#include "chess_board.hpp"
#include "globals.hpp"

TEST(chess_board_tests, to_fen)
{
    std::string test_fen = std::string(START_POSITION_FEN);

    Chess_Board position;
    position.set_from_fen(test_fen);
    
    ASSERT_EQ(test_fen, position.to_fen());

    test_fen = "rnbqk2r/p1p1bp1p/3ppnp1/1B6/Pp1NPBQ1/1P1PN3/2P2PPP/R4RK1 b kq a3 0 1";

    position.set_from_fen(test_fen);
    ASSERT_EQ(test_fen, position.to_fen());
}
