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

    test_fen = "3r1rk1/ppp2pbp/1qn2np1/3pp1B1/3PP1b1/Q1N2NP1/PPP2PBP/3R1RK1 w - - 0 1";

    position.set_from_fen(test_fen);
    ASSERT_EQ(test_fen, position.to_fen());

    test_fen = "r1b2rk1/3p1ppp/2nbpn2/pp2N3/1qpP1B1Q/5NP1/PPP1PPBP/R4RK1 b - d3 0 1";

    position.set_from_fen(test_fen);
    ASSERT_EQ(test_fen, position.to_fen());
}
