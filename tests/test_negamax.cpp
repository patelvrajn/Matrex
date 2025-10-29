#include "gtest/gtest.h"
#include "search.hpp"

TEST(negamax, mating) {
  constexpr std::string_view FENS[] = {
      "4r2k/1p3rbp/2p1N1p1/p3n3/P2NB1nq/1P6/4R1P1/B1Q2RK1 b - - 4 32",
      "4r3/1pp2rbk/6pn/4n3/P3BN1q/1PB2bPP/8/2Q1RRK1 b - - 0 31",
      "4r1k1/4r1p1/8/p2R1P1K/5P1P/1QP3q1/1P6/3R4 b - - 0 1"};

  Chess_Board cb;

  for (uint8_t fen_idx = 0; fen_idx < 3; fen_idx++) {
    cb.set_from_fen(std::string(FENS[fen_idx]));
    Search_Engine search(cb);
    const uint16_t distance_to_mate = (NUM_OF_PLAYERS * (fen_idx + 1));
    const Search_Engine_Result search_result = search.negamax(distance_to_mate);
    EXPECT_EQ(search_result.second.to_int(),
              (ESCORE::WINNING_MATE_MAX - (distance_to_mate - 1)));
  }
}

TEST(negamax, consistent_scoring) {
  constexpr uint16_t SEARCH_DEPTH = 5;

  Chess_Board cb;
  cb.set_from_fen(
      "r1bq1rk1/ppp2ppp/2n1p3/b2p4/3PnB2/P1N1P3/1PP2PPP/1K1RQBNR b - - 2 9");

  Search_Engine first_search(cb);
  const Search_Engine_Result first_search_result =
      first_search.negamax(SEARCH_DEPTH);

  cb.make_move(first_search_result.first);

  Search_Engine second_search(cb);
  const Search_Engine_Result second_search_result =
      second_search.negamax(SEARCH_DEPTH - 1);

  // The score from the first search made at depth D must be the negated score
  // of a second search (made after making the best move) made at depth (D-1).
  ASSERT_EQ(first_search_result.second.to_int(),
            -second_search_result.second.to_int());
}
