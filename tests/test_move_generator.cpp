#include "gtest/gtest.h"
#include "move_generator.hpp"
#include "tests/compare.hpp"

TEST(move_generator_tests, free_to_move_king) {
  // Only kings on the board, white to move.
  Chess_Board cb;
  cb.set_from_fen("8/3k4/8/8/8/8/3K4/8 w - - 0 1");

  std::vector<Chess_Move> generated_move_list;
  Move_Generator mg(cb);
  mg.generate_all_moves(generated_move_list);

  std::vector<Chess_Move> expected_move_list;

  Chess_Move move = {
      .source_square = ESQUARE::D2,
      .destination_square = ESQUARE::C1,
      .moving_piece = PIECES::KING,
      .promoted_piece = PIECES::NO_PIECE,
      .captured_piece = PIECES::NO_PIECE,
      .is_capture = false,
      .is_short_castling = false,
      .is_long_castling = false,
      .is_double_pawn_push = false,
      .is_en_passant = false,
      .is_promotion = false,
      .is_check = false,
      .next_board_state = nullptr};  // TODO: Should compare next position.

  expected_move_list.push_back(move);

  move.destination_square = ESQUARE::D1;
  expected_move_list.push_back(move);

  move.destination_square = ESQUARE::E1;
  expected_move_list.push_back(move);

  move.destination_square = ESQUARE::C2;
  expected_move_list.push_back(move);

  move.destination_square = ESQUARE::E2;
  expected_move_list.push_back(move);

  move.destination_square = ESQUARE::C3;
  expected_move_list.push_back(move);

  move.destination_square = ESQUARE::D3;
  expected_move_list.push_back(move);

  move.destination_square = ESQUARE::E3;
  expected_move_list.push_back(move);

  ASSERT_EQ(
      are_vectors_equal<Chess_Move>(generated_move_list, expected_move_list),
      true);

  generated_move_list.clear();
  expected_move_list.clear();

  // Same FEN except it is black to move.
  cb.set_from_fen("8/3k4/8/8/8/8/3K4/8 b - - 0 1");

  mg.set_chess_board(cb);
  mg.generate_all_moves(generated_move_list);

  move.source_square = ESQUARE::D7;
  move.destination_square = ESQUARE::C8;
  expected_move_list.push_back(move);

  move.destination_square = ESQUARE::D8;
  expected_move_list.push_back(move);

  move.destination_square = ESQUARE::E8;
  expected_move_list.push_back(move);

  move.destination_square = ESQUARE::C7;
  expected_move_list.push_back(move);

  move.destination_square = ESQUARE::E7;
  expected_move_list.push_back(move);

  move.destination_square = ESQUARE::C6;
  expected_move_list.push_back(move);

  move.destination_square = ESQUARE::D6;
  expected_move_list.push_back(move);

  move.destination_square = ESQUARE::E6;
  expected_move_list.push_back(move);

  ASSERT_EQ(
      are_vectors_equal<Chess_Move>(generated_move_list, expected_move_list),
      true);
}

TEST(move_generator_tests, king_with_capture_and_limited_movement) {
  Chess_Board cb;
  // cb.set_from_fen("2r1r3/3k4/3N4/8/8/3n4/3K4/2R1R3 w - - 0 1");
  // cb.set_from_fen("3k4/pppppppp/8/8/8/8/PPPPPPPP/3K4 w - - 0 1");
  cb.set_from_fen(
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
  // cb.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0
  // 1"); cb.set_from_fen("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ -
  // 1 8"); cb.set_from_fen("R6R/3Q4/1Q4Q1/4Q3/2Q4Q/Q4Q2/pp1Q4/kBNNK1B1 w - - 0
  // 1");
  cb.pretty_print();

  std::vector<Chess_Move> generated_move_list;
  Move_Generator mg(cb);
  mg.generate_all_moves(generated_move_list);

  std::cout << "Generated move list size: "
            << std::to_string(generated_move_list.size()) << std::endl;

  std::vector<Chess_Move> expected_move_list;

  // for (Chess_Move cm : generated_move_list) {
  //   cm.pretty_print();
  // }
}
