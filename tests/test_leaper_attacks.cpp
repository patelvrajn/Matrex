#include "gtest/gtest.h"
#include "leaper_attacks.hpp"

TEST(leaper_attacks, pawn_attacks) {
  Bitboard testee = mask_pawn_attacks(PIECE_COLOR::WHITE, Square(ESQUARE::E4));
  Bitboard answer;
  answer.set_square(Square(ESQUARE::D5));
  answer.set_square(Square(ESQUARE::F5));
  ASSERT_EQ(testee, answer);

  answer.set_board(0);

  testee = mask_pawn_attacks(PIECE_COLOR::WHITE, Square(ESQUARE::A7));
  answer.set_square(Square(ESQUARE::B8));
  ASSERT_EQ(testee, answer);

  answer.set_board(0);

  testee = mask_pawn_attacks(PIECE_COLOR::WHITE, Square(ESQUARE::H7));
  answer.set_square(Square(ESQUARE::G8));
  ASSERT_EQ(testee, answer);

  answer.set_board(0);

  testee = mask_pawn_attacks(PIECE_COLOR::BLACK, Square(ESQUARE::E4));
  answer.set_square(Square(ESQUARE::D3));
  answer.set_square(Square(ESQUARE::F3));
  ASSERT_EQ(testee, answer);

  answer.set_board(0);

  Square s3(ESQUARE::H2);
  testee = mask_pawn_attacks(PIECE_COLOR::BLACK, Square(ESQUARE::H2));
  answer.set_square(Square(ESQUARE::G1));
  ASSERT_EQ(testee, answer);

  answer.set_board(0);

  testee = mask_pawn_attacks(PIECE_COLOR::BLACK, Square(ESQUARE::A2));
  answer.set_square(Square(ESQUARE::B1));
  ASSERT_EQ(testee, answer);
}

TEST(leaper_attacks, knight_attacks) {
  Bitboard testee = mask_knight_attacks(Square(ESQUARE::A1));
  Bitboard answer;
  answer.set_square(Square(ESQUARE::C2));
  answer.set_square(Square(ESQUARE::B3));
  ASSERT_EQ(testee, answer);

  answer.set_board(0);

  testee = mask_knight_attacks(Square(ESQUARE::A8));
  answer.set_square(Square(ESQUARE::B6));
  answer.set_square(Square(ESQUARE::C7));
  ASSERT_EQ(testee, answer);

  answer.set_board(0);

  testee = mask_knight_attacks(Square(ESQUARE::H1));
  answer.set_square(Square(ESQUARE::F2));
  answer.set_square(Square(ESQUARE::G3));
  ASSERT_EQ(testee, answer);

  answer.set_board(0);

  testee = mask_knight_attacks(Square(ESQUARE::H8));
  answer.set_square(Square(ESQUARE::G6));
  answer.set_square(Square(ESQUARE::F7));
  ASSERT_EQ(testee, answer);

  answer.set_board(0);

  testee = mask_knight_attacks(Square(ESQUARE::A5));
  answer.set_square(Square(ESQUARE::B3));
  answer.set_square(Square(ESQUARE::C4));
  answer.set_square(Square(ESQUARE::B7));
  answer.set_square(Square(ESQUARE::C6));
  ASSERT_EQ(testee, answer);

  answer.set_board(0);

  testee = mask_knight_attacks(Square(ESQUARE::B5));
  answer.set_square(Square(ESQUARE::A3));
  answer.set_square(Square(ESQUARE::A7));
  answer.set_square(Square(ESQUARE::C3));
  answer.set_square(Square(ESQUARE::C7));
  answer.set_square(Square(ESQUARE::D4));
  answer.set_square(Square(ESQUARE::D6));
  ASSERT_EQ(testee, answer);

  answer.set_board(0);

  testee = mask_knight_attacks(Square(ESQUARE::E5));
  answer.set_square(Square(ESQUARE::C4));
  answer.set_square(Square(ESQUARE::C6));
  answer.set_square(Square(ESQUARE::D3));
  answer.set_square(Square(ESQUARE::D7));
  answer.set_square(Square(ESQUARE::F3));
  answer.set_square(Square(ESQUARE::F7));
  answer.set_square(Square(ESQUARE::G4));
  answer.set_square(Square(ESQUARE::G6));
  ASSERT_EQ(testee, answer);

  answer.set_board(0);

  testee = mask_knight_attacks(Square(ESQUARE::G5));
  answer.set_square(Square(ESQUARE::H3));
  answer.set_square(Square(ESQUARE::H7));
  answer.set_square(Square(ESQUARE::F3));
  answer.set_square(Square(ESQUARE::F7));
  answer.set_square(Square(ESQUARE::E4));
  answer.set_square(Square(ESQUARE::E6));
  ASSERT_EQ(testee, answer);

  answer.set_board(0);

  testee = mask_knight_attacks(Square(ESQUARE::H5));
  answer.set_square(Square(ESQUARE::G3));
  answer.set_square(Square(ESQUARE::G7));
  answer.set_square(Square(ESQUARE::F4));
  answer.set_square(Square(ESQUARE::F6));
  ASSERT_EQ(testee, answer);

  answer.set_board(0);

  testee = mask_knight_attacks(Square(ESQUARE::D1));
  answer.set_square(Square(ESQUARE::B2));
  answer.set_square(Square(ESQUARE::F2));
  answer.set_square(Square(ESQUARE::C3));
  answer.set_square(Square(ESQUARE::E3));
  ASSERT_EQ(testee, answer);

  answer.set_board(0);

  testee = mask_knight_attacks(Square(ESQUARE::D2));
  answer.set_square(Square(ESQUARE::B1));
  answer.set_square(Square(ESQUARE::B3));
  answer.set_square(Square(ESQUARE::C4));
  answer.set_square(Square(ESQUARE::E4));
  answer.set_square(Square(ESQUARE::F1));
  answer.set_square(Square(ESQUARE::F3));
  ASSERT_EQ(testee, answer);

  answer.set_board(0);

  testee = mask_knight_attacks(Square(ESQUARE::D7));
  answer.set_square(Square(ESQUARE::B6));
  answer.set_square(Square(ESQUARE::B8));
  answer.set_square(Square(ESQUARE::C5));
  answer.set_square(Square(ESQUARE::E5));
  answer.set_square(Square(ESQUARE::F6));
  answer.set_square(Square(ESQUARE::F8));
  ASSERT_EQ(testee, answer);

  answer.set_board(0);

  testee = mask_knight_attacks(Square(ESQUARE::D8));
  answer.set_square(Square(ESQUARE::B7));
  answer.set_square(Square(ESQUARE::F7));
  answer.set_square(Square(ESQUARE::C6));
  answer.set_square(Square(ESQUARE::E6));
  ASSERT_EQ(testee, answer);
}

TEST(leaper_attacks, king_attacks) {
  Bitboard testee = mask_king_attacks(Square(ESQUARE::A1));
  Bitboard answer;
  answer.set_square(Square(ESQUARE::A2));
  answer.set_square(Square(ESQUARE::B1));
  answer.set_square(Square(ESQUARE::B2));
  ASSERT_EQ(testee, answer);

  answer.set_board(0);

  testee = mask_king_attacks(Square(ESQUARE::A4));
  answer.set_square(Square(ESQUARE::A3));
  answer.set_square(Square(ESQUARE::A5));
  answer.set_square(Square(ESQUARE::B3));
  answer.set_square(Square(ESQUARE::B4));
  answer.set_square(Square(ESQUARE::B5));
  ASSERT_EQ(testee, answer);

  answer.set_board(0);

  testee = mask_king_attacks(Square(ESQUARE::A8));
  answer.set_square(Square(ESQUARE::A7));
  answer.set_square(Square(ESQUARE::B7));
  answer.set_square(Square(ESQUARE::B8));
  ASSERT_EQ(testee, answer);

  answer.set_board(0);

  testee = mask_king_attacks(Square(ESQUARE::D1));
  answer.set_square(Square(ESQUARE::C1));
  answer.set_square(Square(ESQUARE::C2));
  answer.set_square(Square(ESQUARE::D2));
  answer.set_square(Square(ESQUARE::E1));
  answer.set_square(Square(ESQUARE::E2));
  ASSERT_EQ(testee, answer);

  answer.set_board(0);

  testee = mask_king_attacks(Square(ESQUARE::D4));
  answer.set_square(Square(ESQUARE::C3));
  answer.set_square(Square(ESQUARE::C4));
  answer.set_square(Square(ESQUARE::C5));
  answer.set_square(Square(ESQUARE::D3));
  answer.set_square(Square(ESQUARE::D5));
  answer.set_square(Square(ESQUARE::E3));
  answer.set_square(Square(ESQUARE::E4));
  answer.set_square(Square(ESQUARE::E5));
  ASSERT_EQ(testee, answer);

  answer.set_board(0);

  testee = mask_king_attacks(Square(ESQUARE::D8));
  answer.set_square(Square(ESQUARE::C7));
  answer.set_square(Square(ESQUARE::C8));
  answer.set_square(Square(ESQUARE::D7));
  answer.set_square(Square(ESQUARE::E7));
  answer.set_square(Square(ESQUARE::E8));
  ASSERT_EQ(testee, answer);

  answer.set_board(0);

  testee = mask_king_attacks(Square(ESQUARE::H1));
  answer.set_square(Square(ESQUARE::G1));
  answer.set_square(Square(ESQUARE::G2));
  answer.set_square(Square(ESQUARE::H2));
  ASSERT_EQ(testee, answer);

  answer.set_board(0);

  testee = mask_king_attacks(Square(ESQUARE::H4));
  answer.set_square(Square(ESQUARE::G3));
  answer.set_square(Square(ESQUARE::G4));
  answer.set_square(Square(ESQUARE::G5));
  answer.set_square(Square(ESQUARE::H3));
  answer.set_square(Square(ESQUARE::H5));
  ASSERT_EQ(testee, answer);

  answer.set_board(0);

  testee = mask_king_attacks(Square(ESQUARE::H8));
  answer.set_square(Square(ESQUARE::G7));
  answer.set_square(Square(ESQUARE::G8));
  answer.set_square(Square(ESQUARE::H7));
  ASSERT_EQ(testee, answer);
}
