#include "gtest/gtest.h"
#include "leaper_attacks.hpp"

TEST(leaper_attacks, pawn_attacks) {
  // A white pawn on E4 should attack D5 and F5.
  Bitboard b = mask_pawn_attacks(PIECE_COLOR::WHITE, Square(ESQUARE::E4));
  ASSERT_EQ(b.get_board(), 671088640);

  // A white pawn on A7 should attack ONLY B8.
  b = mask_pawn_attacks(PIECE_COLOR::WHITE, Square(ESQUARE::A7));
  ASSERT_EQ(b.get_board(), 2);

  // A white pawn on H7 should attack ONLY G8.
  b = mask_pawn_attacks(PIECE_COLOR::WHITE, Square(ESQUARE::H7));
  ASSERT_EQ(b.get_board(), 64);

  // A black pawn on E4 should attack D3 and F3.
  b = mask_pawn_attacks(PIECE_COLOR::BLACK, Square(ESQUARE::E4));
  ASSERT_EQ(b.get_board(), 43980465111040);

  // A black pawn on H2 should attack ONLY G1.
  Square s3(ESQUARE::H2);
  b = mask_pawn_attacks(PIECE_COLOR::BLACK, Square(ESQUARE::H2));
  ASSERT_EQ(b.get_board(), 4611686018427387904);

  // A black pawn on A2 should attack ONLY B1.
  b = mask_pawn_attacks(PIECE_COLOR::BLACK, Square(ESQUARE::A2));
  ASSERT_EQ(b.get_board(), 144115188075855872);
}

TEST(leaper_attacks, knight_attacks) {
  // A knight on A1 should attack C2 and B3.
  Bitboard b = mask_knight_attacks(Square(ESQUARE::A1));
  ASSERT_EQ(b.get_board(), 1128098930098176);

  // A knight on A8 should attack B6 and C7.
  b = mask_knight_attacks(Square(ESQUARE::A8));
  ASSERT_EQ(b.get_board(), 132096);

  // A knight on H1 should attack F2 and G3.
  b = mask_knight_attacks(Square(ESQUARE::H1));
  ASSERT_EQ(b.get_board(), 9077567998918656);

  // A knight on H8 should attack G6 and F7.
  b = mask_knight_attacks(Square(ESQUARE::H8));
  ASSERT_EQ(b.get_board(), 4202496);

  // A knight on A5 should attack B3, C4, B7, and C6.
  b = mask_knight_attacks(Square(ESQUARE::A5));
  ASSERT_EQ(b.get_board(), 2216203387392);

  // A knight on B5 should attack A3, A7, C3, C7, D4, and D6.
  b = mask_knight_attacks(Square(ESQUARE::B5));
  ASSERT_EQ(b.get_board(), 5531918402816);

  // A knight on E5 should attack C4, C6, D3, D7, F3, F7, G4, and G6.
  b = mask_knight_attacks(Square(ESQUARE::E5));
  ASSERT_EQ(b.get_board(), 44272527353856);

  // A knight on G5 should attack H3, H7, F3, F7, E4, and E6.
  b = mask_knight_attacks(Square(ESQUARE::G5));
  ASSERT_EQ(b.get_board(), 175990581010432);

  // A knight on H5 should attack G3, G7, F4, and F6.
  b = mask_knight_attacks(Square(ESQUARE::H5));
  ASSERT_EQ(b.get_board(), 70506185244672);

  // A knight on D1 should attack B2, F2, C3, and E3.
  b = mask_knight_attacks(Square(ESQUARE::D1));
  ASSERT_EQ(b.get_board(), 9592139440717824);

  // A knight on D2 should attack B1, B3, C4, E4, F1, and F3.
  b = mask_knight_attacks(Square(ESQUARE::D2));
  ASSERT_EQ(b.get_board(), 2449995666584240128);

  // A knight on D7 should attack B6, B8, C5, E5, F6, and F8.
  b = mask_knight_attacks(Square(ESQUARE::D7));
  ASSERT_EQ(b.get_board(), 337772578);

  // A knight on D8 should attack B7, F7, C6, and E6.
  b = mask_knight_attacks(Square(ESQUARE::D8));
  ASSERT_EQ(b.get_board(), 1319424);
}

TEST(leaper_attacks, king_attacks) {
  // A king on A1 should attack A2, B1, and B2.
  Bitboard b = mask_king_attacks(Square(ESQUARE::A1));
  ASSERT_EQ(b.get_board(), 144959613005987840);

  // A king on A4 should attack A3, A5, B3, B4, and B5.
  b = mask_king_attacks(Square(ESQUARE::A4));
  ASSERT_EQ(b.get_board(), 3307175149568);

  // A king on A8 should attack A7, B7, and B8.
  b = mask_king_attacks(Square(ESQUARE::A8));
  ASSERT_EQ(b.get_board(), 770);

  // A king on D1 should attack C1, C2, D2, E1, and E2.
  b = mask_king_attacks(Square(ESQUARE::D1));
  ASSERT_EQ(b.get_board(), 1449033180106457088);

  // A king on D4 should attack C3, C4, C5, D3, D5, E3, E4, and E5.
  b = mask_king_attacks(Square(ESQUARE::D4));
  ASSERT_EQ(b.get_board(), 30872694685696);

  // A king on D8 should attack C7, C8, D7, E7, and E8.
  b = mask_king_attacks(Square(ESQUARE::D8));
  ASSERT_EQ(b.get_board(), 7188);

  // A king on H1 should attack G1, G2, and H2.
  b = mask_king_attacks(Square(ESQUARE::H1));
  ASSERT_EQ(b.get_board(), 4665729213955833856);

  // A king on H4 should attack G3, G4, G5, H3 and H5.
  b = mask_king_attacks(Square(ESQUARE::H4));
  ASSERT_EQ(b.get_board(), 211384331665408);

  // A king on H8 should attack G7, G8, and H7.
  b = mask_king_attacks(Square(ESQUARE::H8));
  ASSERT_EQ(b.get_board(), 49216);
}
