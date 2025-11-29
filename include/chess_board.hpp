#pragma once

#include <array>

#include "bitboard.hpp"
#include "chess_move.hpp"
#include "globals.hpp"
#include "zobrist_hash.hpp"

constexpr uint8_t NUM_OF_CASTLING_TYPES = 2;

enum CASTLING_TYPE { KINGSIDE, QUEENSIDE };

enum CASTLING_RIGHTS_FLAGS {
  W_KINGSIDE = 1,
  W_QUEENSIDE = 2,
  B_KINGSIDE = 4,
  B_QUEENSIDE = 8
};

struct Castling_Rooks {
  Square queenside;
  Square kingside;
};

struct chess_board_state {
  PIECE_COLOR side_to_move : 2;  // Needs 2 bits because of NO_COLOR
  ESQUARE enpassant_square : 7;  // Needs 7 bits because of NO_SQUARE
  uint8_t castling_rights : 4;
  uint8_t half_move_clock : 6;  // Doesn't go past 50 so 6 bits.
  std::array<Castling_Rooks, NUM_OF_PLAYERS> castling_rooks;
  uint64_t full_move_count;

  bool operator==(const chess_board_state& other) const {
    return ((other.side_to_move == side_to_move) &&
            (other.enpassant_square == enpassant_square) &&
            (other.castling_rights == castling_rights) &&
            (other.half_move_clock == half_move_clock) &&
            (other.full_move_count == full_move_count));
  }
};

class Chess_Board {
 public:
  Chess_Board();

  void pretty_print() const;

  std::pair<PIECE_COLOR, PIECES> what_piece_is_on_square(const Square& s) const;

  void set_from_fen(const std::string& fen);

  Bitboard get_both_color_occupancies() const;

  Bitboard get_color_occupancies(PIECE_COLOR c) const;

  Bitboard get_piece_occupancies(PIECE_COLOR c, PIECES p) const;

  Square get_en_passant_square() const;

  Square get_en_passant_victim_square() const;

  PIECE_COLOR get_side_to_move() const;

  Square get_king_square(PIECE_COLOR c) const;

  bool does_white_have_short_castle_rights() const;

  bool does_white_have_long_castle_rights() const;

  bool does_black_have_short_castle_rights() const;

  bool does_black_have_long_castle_rights() const;

  Square get_castling_rook_source_square(PIECE_COLOR color,
                                         CASTLING_TYPE castle_type) const;

  Undo_Chess_Move make_move(const Chess_Move& move);

  void undo_move(Undo_Chess_Move undo_move);

  Zobrist_Hash get_zobrist_hash() const;

  bool operator==(const Chess_Board& other) const;

 private:
  std::array<std::array<Bitboard, NUM_OF_UNIQUE_PIECES_PER_PLAYER>,
             NUM_OF_PLAYERS>
      m_piece_bitboards;
  std::array<Bitboard, NUM_OF_PLAYERS> m_color_occupancy_bitboards;

  chess_board_state m_state;

  Zobrist_Hash m_zobrist_hash;

  void place_pieces_from_fen(const std::string& rank_description,
                             uint8_t length_of_description, uint8_t rank);

  inline void calculate_next_board_state(PIECE_COLOR moving_side,
                                         const Chess_Move& move);
};

inline void Chess_Board::calculate_next_board_state(PIECE_COLOR moving_side,
                                                    const Chess_Move& move) {
  const PIECE_COLOR opposing_side = (PIECE_COLOR)((~moving_side) & 0x1);
  const Bitboard src_mask = Square(move.source_square).get_mask();
  const Bitboard dst_mask = Square(move.destination_square).get_mask();

  // Handle captured piece.
  if (move.is_capture) {
    m_piece_bitboards[opposing_side][move.captured_piece] ^= dst_mask;
    m_color_occupancy_bitboards[opposing_side] ^= dst_mask;

    m_zobrist_hash.update_piece(opposing_side, move.captured_piece,
                                Square(move.destination_square));
  }

  // Handle En Passant.
  if (move.is_en_passant) {
    const Bitboard en_passant_mask =
        Square(move.en_passant_victim_square).get_mask();
    m_piece_bitboards[opposing_side][PIECES::PAWN] ^= en_passant_mask;
    m_color_occupancy_bitboards[opposing_side] ^= en_passant_mask;

    m_zobrist_hash.update_piece(opposing_side, PIECES::PAWN,
                                Square(move.en_passant_victim_square));
  }

  // Moving piece source square
  m_piece_bitboards[moving_side][move.moving_piece] ^= src_mask;
  m_color_occupancy_bitboards[moving_side] ^= src_mask;

  m_zobrist_hash.update_piece(moving_side, move.moving_piece,
                              Square(move.source_square));

  // Moving piece target square if not a pawn promotion
  if (!move.is_promotion) {
    m_piece_bitboards[moving_side][move.moving_piece] ^= dst_mask;
    m_color_occupancy_bitboards[moving_side] ^= dst_mask;

    m_zobrist_hash.update_piece(moving_side, move.moving_piece,
                                Square(move.destination_square));
  }
  // Moving piece target square if there is a promotion
  else {
    m_piece_bitboards[moving_side][move.promoted_piece] ^= dst_mask;
    m_color_occupancy_bitboards[moving_side] ^= dst_mask;

    m_zobrist_hash.update_piece(moving_side, move.promoted_piece,
                                Square(move.destination_square));
  }

  // Handle rook movement if castling
  if (move.is_short_castling || move.is_long_castling) {
    const Bitboard rook_src_mask =
        Square(move.castling_rook_source_square).get_mask();
    const Bitboard rook_dst_mask =
        Square(move.castling_rook_destination_square).get_mask();

    m_piece_bitboards[moving_side][PIECES::ROOK] ^= rook_src_mask;
    m_piece_bitboards[moving_side][PIECES::ROOK] ^= rook_dst_mask;

    m_color_occupancy_bitboards[moving_side] ^= rook_src_mask;
    m_color_occupancy_bitboards[moving_side] ^= rook_dst_mask;

    m_zobrist_hash.update_piece(moving_side, PIECES::ROOK,
                                Square(move.castling_rook_source_square));
    m_zobrist_hash.update_piece(moving_side, PIECES::ROOK,
                                Square(move.castling_rook_destination_square));
  }
}
