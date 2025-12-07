#include "chess_board.hpp"
#include "globals.hpp"
#include "gtest/gtest.h"
#include "zobrist_hash.hpp"

TEST(zobrist_hash, zobrist_hash) {
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

  EXPECT_EQ(cb.get_zobrist_hash().get_hash_value(),
            expected_hash.get_hash_value());
}
