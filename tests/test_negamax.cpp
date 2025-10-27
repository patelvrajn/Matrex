#include "gtest/gtest.h"
#include "search.hpp"

TEST(negamax, mating) {
  constexpr std::string_view FENS[] = {
      "4r2k/1p3rbp/2p1N1p1/p3n3/P2NB1nq/1P6/4R1P1/B1Q2RK1 b - - 4 32",
      "4r3/1pp2rbk/6pn/4n3/P3BN1q/1PB2bPP/8/2Q1RRK1 b - - 0 31",
      "4r1k1/4r1p1/8/p2R1P1K/5P1P/1QP3q1/1P6/3R4 b - - 0 1",
      "3qr2k/1p3rbp/2p3p1/p7/P2pBNn1/1P3n2/6P1/B1Q1RR1K b - - 1 30",
      "4rb1k/2pqn2p/6pn/ppp3N1/P1QP2b1/1P2p3/2B3PP/B3RRK1 w - - 0 24",
      "4rr2/1p4bk/2p3pn/B3n2b/P4N1q/1P5P/6PK/1BQ1RR2 b - - 1 31",
      "5r2/1pB2rbk/6pn/4n2q/P3B3/1P5P/4R1P1/2Q2R1K b - - 3 33"};

  Chess_Board cb;

  for (uint8_t distance_to_mate = 0; distance_to_mate < 8; distance_to_mate++) {
    cb.set_from_fen(std::string(FENS[distance_to_mate]));
    Search_Engine search(cb);
    Search_Engine_Result search_result = search.negamax(distance_to_mate + 1);
    EXPECT_EQ(search_result.second.to_int(),
              (ESCORE::WINNING_MATE_MAX - (distance_to_mate + 1)));
  }
}
