#include "bishop_magic_bitboards.hpp"
#include "gtest/gtest.h"
#include "rook_magic_bitboards.hpp"

TEST(slider_attacks, bishop_attacks) {
  Bishop_Magic_Bitboards bmagic;

  Bitboard occupancy;
  Bitboard testee = bmagic.get_attacks(Square(ESQUARE::E4), occupancy);
  Bitboard answer;
  answer.set_square(Square(ESQUARE::B1));
  answer.set_square(Square(ESQUARE::C2));
  answer.set_square(Square(ESQUARE::D3));
  answer.set_square(Square(ESQUARE::H1));
  answer.set_square(Square(ESQUARE::G2));
  answer.set_square(Square(ESQUARE::F3));
  answer.set_square(Square(ESQUARE::F5));
  answer.set_square(Square(ESQUARE::G6));
  answer.set_square(Square(ESQUARE::H7));
  answer.set_square(Square(ESQUARE::D5));
  answer.set_square(Square(ESQUARE::C6));
  answer.set_square(Square(ESQUARE::B7));
  answer.set_square(Square(ESQUARE::A8));
  ASSERT_EQ(testee, answer);

  answer.set_board(0);

  occupancy.set_square(Square(ESQUARE::C2));
  occupancy.set_square(Square(ESQUARE::G2));
  occupancy.set_square(Square(ESQUARE::G6));
  occupancy.set_square(Square(ESQUARE::C6));
  testee = bmagic.get_attacks(Square(ESQUARE::E4), occupancy);
  answer.set_square(Square(ESQUARE::C2));
  answer.set_square(Square(ESQUARE::D3));
  answer.set_square(Square(ESQUARE::G2));
  answer.set_square(Square(ESQUARE::F3));
  answer.set_square(Square(ESQUARE::F5));
  answer.set_square(Square(ESQUARE::G6));
  answer.set_square(Square(ESQUARE::D5));
  answer.set_square(Square(ESQUARE::C6));
  ASSERT_EQ(testee, answer);
}

TEST(slider_attacks, rook_attacks) {
  Rook_Magic_Bitboards rmagic;

  Bitboard occupancy;
  Bitboard testee = rmagic.get_attacks(Square(ESQUARE::E4), occupancy);
  Bitboard answer;
  answer.set_square(Square(ESQUARE::E1));
  answer.set_square(Square(ESQUARE::E2));
  answer.set_square(Square(ESQUARE::E3));
  answer.set_square(Square(ESQUARE::E5));
  answer.set_square(Square(ESQUARE::E6));
  answer.set_square(Square(ESQUARE::E7));
  answer.set_square(Square(ESQUARE::E8));
  answer.set_square(Square(ESQUARE::A4));
  answer.set_square(Square(ESQUARE::B4));
  answer.set_square(Square(ESQUARE::C4));
  answer.set_square(Square(ESQUARE::D4));
  answer.set_square(Square(ESQUARE::F4));
  answer.set_square(Square(ESQUARE::G4));
  answer.set_square(Square(ESQUARE::H4));
  ASSERT_EQ(testee, answer);

  answer.set_board(0);

  occupancy.set_square(Square(ESQUARE::E2));
  occupancy.set_square(Square(ESQUARE::G4));
  occupancy.set_square(Square(ESQUARE::E6));
  occupancy.set_square(Square(ESQUARE::C4));
  testee = rmagic.get_attacks(Square(ESQUARE::E4), occupancy);
  answer.set_square(Square(ESQUARE::E2));
  answer.set_square(Square(ESQUARE::E3));
  answer.set_square(Square(ESQUARE::F4));
  answer.set_square(Square(ESQUARE::G4));
  answer.set_square(Square(ESQUARE::E5));
  answer.set_square(Square(ESQUARE::E6));
  answer.set_square(Square(ESQUARE::C4));
  answer.set_square(Square(ESQUARE::D4));
  ASSERT_EQ(testee, answer);
}