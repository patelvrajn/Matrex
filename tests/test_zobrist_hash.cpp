#include "chess_board.hpp"
#include "globals.hpp"
#include "gtest/gtest.h"
#include "zobrist_hash.hpp"

TEST(zobrist_hash, zobrist_hash) {
  // The PGN for the game used in this test is:
  // 1. Nc3 Nf6 2. Ne4 Nxe4 3. d4 g6 4. d5 e5 5. dxe6 Bg7 6. Nf3 O-O 7. e7 d6 8.
  // e8=Q

  Chess_Board cb;
  cb.set_from_fen(std::string(START_POSITION_FEN));

  Zobrist_Hash expected_hash;
  expected_hash.update_piece(PIECE_COLOR::WHITE, PIECES::ROOK,
                             Square(ESQUARE::A1));
  expected_hash.update_piece(PIECE_COLOR::WHITE, PIECES::KNIGHT,
                             Square(ESQUARE::B1));
  expected_hash.update_piece(PIECE_COLOR::WHITE, PIECES::BISHOP,
                             Square(ESQUARE::C1));
  expected_hash.update_piece(PIECE_COLOR::WHITE, PIECES::QUEEN,
                             Square(ESQUARE::D1));
  expected_hash.update_piece(PIECE_COLOR::WHITE, PIECES::KING,
                             Square(ESQUARE::E1));
  expected_hash.update_piece(PIECE_COLOR::WHITE, PIECES::BISHOP,
                             Square(ESQUARE::F1));
  expected_hash.update_piece(PIECE_COLOR::WHITE, PIECES::KNIGHT,
                             Square(ESQUARE::G1));
  expected_hash.update_piece(PIECE_COLOR::WHITE, PIECES::ROOK,
                             Square(ESQUARE::H1));

  expected_hash.update_piece(PIECE_COLOR::WHITE, PIECES::PAWN,
                             Square(ESQUARE::A2));
  expected_hash.update_piece(PIECE_COLOR::WHITE, PIECES::PAWN,
                             Square(ESQUARE::B2));
  expected_hash.update_piece(PIECE_COLOR::WHITE, PIECES::PAWN,
                             Square(ESQUARE::C2));
  expected_hash.update_piece(PIECE_COLOR::WHITE, PIECES::PAWN,
                             Square(ESQUARE::D2));
  expected_hash.update_piece(PIECE_COLOR::WHITE, PIECES::PAWN,
                             Square(ESQUARE::E2));
  expected_hash.update_piece(PIECE_COLOR::WHITE, PIECES::PAWN,
                             Square(ESQUARE::F2));
  expected_hash.update_piece(PIECE_COLOR::WHITE, PIECES::PAWN,
                             Square(ESQUARE::G2));
  expected_hash.update_piece(PIECE_COLOR::WHITE, PIECES::PAWN,
                             Square(ESQUARE::H2));

  expected_hash.update_piece(PIECE_COLOR::BLACK, PIECES::ROOK,
                             Square(ESQUARE::A8));
  expected_hash.update_piece(PIECE_COLOR::BLACK, PIECES::KNIGHT,
                             Square(ESQUARE::B8));
  expected_hash.update_piece(PIECE_COLOR::BLACK, PIECES::BISHOP,
                             Square(ESQUARE::C8));
  expected_hash.update_piece(PIECE_COLOR::BLACK, PIECES::QUEEN,
                             Square(ESQUARE::D8));
  expected_hash.update_piece(PIECE_COLOR::BLACK, PIECES::KING,
                             Square(ESQUARE::E8));
  expected_hash.update_piece(PIECE_COLOR::BLACK, PIECES::BISHOP,
                             Square(ESQUARE::F8));
  expected_hash.update_piece(PIECE_COLOR::BLACK, PIECES::KNIGHT,
                             Square(ESQUARE::G8));
  expected_hash.update_piece(PIECE_COLOR::BLACK, PIECES::ROOK,
                             Square(ESQUARE::H8));

  expected_hash.update_piece(PIECE_COLOR::BLACK, PIECES::PAWN,
                             Square(ESQUARE::A7));
  expected_hash.update_piece(PIECE_COLOR::BLACK, PIECES::PAWN,
                             Square(ESQUARE::B7));
  expected_hash.update_piece(PIECE_COLOR::BLACK, PIECES::PAWN,
                             Square(ESQUARE::C7));
  expected_hash.update_piece(PIECE_COLOR::BLACK, PIECES::PAWN,
                             Square(ESQUARE::D7));
  expected_hash.update_piece(PIECE_COLOR::BLACK, PIECES::PAWN,
                             Square(ESQUARE::E7));
  expected_hash.update_piece(PIECE_COLOR::BLACK, PIECES::PAWN,
                             Square(ESQUARE::F7));
  expected_hash.update_piece(PIECE_COLOR::BLACK, PIECES::PAWN,
                             Square(ESQUARE::G7));
  expected_hash.update_piece(PIECE_COLOR::BLACK, PIECES::PAWN,
                             Square(ESQUARE::H7));

  expected_hash.update_castling_rights(
      CASTLING_RIGHTS_FLAGS::W_KINGSIDE | CASTLING_RIGHTS_FLAGS::W_QUEENSIDE |
      CASTLING_RIGHTS_FLAGS::B_KINGSIDE | CASTLING_RIGHTS_FLAGS::B_QUEENSIDE);

  expected_hash.update_en_passant_square(Square(ESQUARE::NO_SQUARE));

  EXPECT_EQ(cb.get_zobrist_hash().get_hash_value(),
            expected_hash.get_hash_value());

  // Test moving piece zobrist hash update
  //                         W
  cb.make_moves_from_string("b1c3", false);

  expected_hash.update_piece(PIECE_COLOR::WHITE, PIECES::KNIGHT,
                             Square(ESQUARE::B1));
  expected_hash.update_piece(PIECE_COLOR::WHITE, PIECES::KNIGHT,
                             Square(ESQUARE::C3));
  expected_hash.flip_side_to_move();

  EXPECT_EQ(cb.get_zobrist_hash().get_hash_value(),
            expected_hash.get_hash_value());

  // Test capture piece zobrist hash update
  //                         B    W    B
  cb.make_moves_from_string("g8f6 c3e4 f6e4", false);

  expected_hash.update_piece(PIECE_COLOR::WHITE, PIECES::KNIGHT,
                             Square(ESQUARE::C3));

  expected_hash.update_piece(PIECE_COLOR::BLACK, PIECES::KNIGHT,
                             Square(ESQUARE::G8));
  expected_hash.update_piece(PIECE_COLOR::BLACK, PIECES::KNIGHT,
                             Square(ESQUARE::E4));

  expected_hash.flip_side_to_move();

  EXPECT_EQ(cb.get_zobrist_hash().get_hash_value(),
            expected_hash.get_hash_value());

  // Test en passant square logic.
  //                         W    B    W    B
  cb.make_moves_from_string("d2d4 g7g6 d4d5 e7e5", false);

  expected_hash.update_en_passant_square(Square(ESQUARE::NO_SQUARE));
  expected_hash.update_en_passant_square(Square(ESQUARE::E6));

  expected_hash.update_piece(PIECE_COLOR::WHITE, PIECES::PAWN,
                             Square(ESQUARE::D2));
  expected_hash.update_piece(PIECE_COLOR::WHITE, PIECES::PAWN,
                             Square(ESQUARE::D5));

  expected_hash.update_piece(PIECE_COLOR::BLACK, PIECES::PAWN,
                             Square(ESQUARE::G7));
  expected_hash.update_piece(PIECE_COLOR::BLACK, PIECES::PAWN,
                             Square(ESQUARE::G6));

  expected_hash.update_piece(PIECE_COLOR::BLACK, PIECES::PAWN,
                             Square(ESQUARE::E7));
  expected_hash.update_piece(PIECE_COLOR::BLACK, PIECES::PAWN,
                             Square(ESQUARE::E5));

  EXPECT_EQ(cb.get_zobrist_hash().get_hash_value(),
            expected_hash.get_hash_value());

  // Test en passant capture logic.
  //                         W
  cb.make_moves_from_string("d5e6", false);

  expected_hash.update_en_passant_square(Square(ESQUARE::E6));
  expected_hash.update_en_passant_square(Square(ESQUARE::NO_SQUARE));

  expected_hash.update_piece(PIECE_COLOR::WHITE, PIECES::PAWN,
                             Square(ESQUARE::D5));
  expected_hash.update_piece(PIECE_COLOR::WHITE, PIECES::PAWN,
                             Square(ESQUARE::E6));

  expected_hash.update_piece(PIECE_COLOR::BLACK, PIECES::PAWN,
                             Square(ESQUARE::E5));

  expected_hash.flip_side_to_move();

  EXPECT_EQ(cb.get_zobrist_hash().get_hash_value(),
            expected_hash.get_hash_value());

  // Test castling logic.
  //                         B    W    B
  cb.make_moves_from_string("f8g7 g1f3 e8g8", false);

  expected_hash.update_piece(PIECE_COLOR::BLACK, PIECES::BISHOP,
                             Square(ESQUARE::F8));
  expected_hash.update_piece(PIECE_COLOR::BLACK, PIECES::BISHOP,
                             Square(ESQUARE::G7));

  expected_hash.update_piece(PIECE_COLOR::WHITE, PIECES::KNIGHT,
                             Square(ESQUARE::G1));
  expected_hash.update_piece(PIECE_COLOR::WHITE, PIECES::KNIGHT,
                             Square(ESQUARE::F3));

  expected_hash.update_piece(PIECE_COLOR::BLACK, PIECES::KING,
                             Square(ESQUARE::E8));
  expected_hash.update_piece(PIECE_COLOR::BLACK, PIECES::KING,
                             Square(ESQUARE::G8));
  expected_hash.update_piece(PIECE_COLOR::BLACK, PIECES::ROOK,
                             Square(ESQUARE::H8));
  expected_hash.update_piece(PIECE_COLOR::BLACK, PIECES::ROOK,
                             Square(ESQUARE::F8));
  expected_hash.update_castling_rights(
      CASTLING_RIGHTS_FLAGS::W_KINGSIDE | CASTLING_RIGHTS_FLAGS::W_QUEENSIDE |
      CASTLING_RIGHTS_FLAGS::B_KINGSIDE | CASTLING_RIGHTS_FLAGS::B_QUEENSIDE);
  expected_hash.update_castling_rights(CASTLING_RIGHTS_FLAGS::W_KINGSIDE |
                                       CASTLING_RIGHTS_FLAGS::W_QUEENSIDE);

  expected_hash.flip_side_to_move();

  EXPECT_EQ(cb.get_zobrist_hash().get_hash_value(),
            expected_hash.get_hash_value());

  // Test pawn promotion logic.
  //                         W    B    W
  cb.make_moves_from_string("e6e7 d7d6 e7e8q", false);

  expected_hash.update_piece(PIECE_COLOR::WHITE, PIECES::PAWN,
                             Square(ESQUARE::E6));
  expected_hash.update_piece(PIECE_COLOR::WHITE, PIECES::QUEEN,
                             Square(ESQUARE::E8));

  expected_hash.update_piece(PIECE_COLOR::BLACK, PIECES::PAWN,
                             Square(ESQUARE::D7));
  expected_hash.update_piece(PIECE_COLOR::BLACK, PIECES::PAWN,
                             Square(ESQUARE::D6));

  expected_hash.flip_side_to_move();

  EXPECT_EQ(cb.get_zobrist_hash().get_hash_value(),
            expected_hash.get_hash_value());
}
