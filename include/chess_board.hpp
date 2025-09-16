#pragma once

#include <array>
#include <memory>

#include "bitboard.hpp"
#include "chess_move.hpp"
#include "globals.hpp"

enum CASTLING_RIGHTS_FLAGS {
  W_KINGSIDE = 1,
  W_QUEENSIDE = 2,
  B_KINGSIDE = 4,
  B_QUEENSIDE = 8
};

typedef struct {
  PIECE_COLOR side_to_move : 2;  // Needs 2 bits because of NO_COLOR
  ESQUARE enpassant_square : 7;  // Needs 7 bits because of NO_SQUARE
  uint8_t castling_rights : 4;
  uint8_t half_move_clock : 6;  // Doesn't go past 50 so 6 bits.
  uint64_t full_move_count;
} chess_board_state;

class Chess_Board {
 public:
  Chess_Board();

  Chess_Board(const Bitboard& w_pawn_bb, const Bitboard& w_knight_bb,
              const Bitboard& w_bishop_bb, const Bitboard& w_rook_bb,
              const Bitboard& w_queen_bb, const Bitboard& w_king_bb,
              const Bitboard& b_pawn_bb, const Bitboard& b_knight_bb,
              const Bitboard& b_bishop_bb, const Bitboard& b_rook_bb,
              const Bitboard& b_queen_bb, const Bitboard& b_king_bb);

  void pretty_print() const;

  std::pair<PIECE_COLOR, PIECES> what_piece_is_on_square(const Square& s) const;

  void set_from_fen(const std::string& fen);

  Bitboard get_both_color_occupancies() const;

  Bitboard get_color_occupancies(PIECE_COLOR c) const;

  Bitboard get_piece_occupancies(PIECE_COLOR c, PIECES p) const;

  Square get_en_passant_square() const;

  PIECE_COLOR get_side_to_move() const;

  Square get_king_square(PIECE_COLOR c) const;

  bool does_white_have_short_castle_rights() const;

  bool does_white_have_long_castle_rights() const;

  bool does_black_have_short_castle_rights() const;

  bool does_black_have_long_castle_rights() const;

  void make_move(const Chess_Move& move);

  void undo_moves(uint64_t num_of_moves);

 private:
  std::array<std::array<Bitboard, NUM_OF_UNIQUE_PIECES_PER_PLAYER>,
             NUM_OF_PLAYERS>
      m_piece_bitboards;
  std::array<Bitboard, NUM_OF_PLAYERS> m_color_occupancy_bitboards;

  chess_board_state m_state;

  void place_pieces_from_fen(const std::string& rank_description,
                             uint8_t length_of_description, uint8_t rank);

  std::shared_ptr<Chess_Board> m_previous_board;

  Bitboard calculate_future_occupancy(
      PIECE_COLOR moving_side, PIECE_COLOR color_of_piece_of_interest,
      PIECES piece_of_interest, PIECES moving_piece, Square source_square,
      Square target_square,
      std::pair<PIECE_COLOR, PIECES> who_is_on_target_square,
      PIECES promotion_piece, bool is_en_passant,
      Square en_passant_captured_pawn_square, bool is_castling,
      Square rook_source_square, Square rook_target_square);

  Chess_Board calculate_future_board_state(
      PIECE_COLOR moving_side, PIECES moving_piece, Square source_square,
      Square target_square,
      std::pair<PIECE_COLOR, PIECES> who_is_on_target_square,
      PIECES promotion_piece, bool is_en_passant,
      Square en_passant_captured_pawn_square, bool is_castling,
      Square rook_source_square, Square rook_target_square);
};

inline Bitboard Chess_Board::calculate_future_occupancy(
    PIECE_COLOR moving_side,  // Side making the move
    PIECE_COLOR
        color_of_piece_of_interest,  // Side whose occupancy we're computing
    PIECES piece_of_interest,  // The tracked piece type (e.g., PAWN, KNIGHT...)
    PIECES moving_piece,       // The piece type being moved this turn
    Square source_square,      // From-square of the move
    Square target_square,      // To-square of the move
    std::pair<PIECE_COLOR, PIECES>
        who_is_on_target_square,  // What was on target before the move
    PIECES promotion_piece,       // If promotion, what piece it becomes (else
                                  // PIECES::NO_PIECE)
    bool is_en_passant,           // True if this move is en passant
    Square en_passant_captured_pawn_square,  // Square of pawn captured by en
                                             // passant (not target)
    bool is_castling,                        // True if this move is castling
    Square rook_source_square,               // Rook's start square for castling
    Square rook_target_square  // Rook's landing square for castling
) {
  // --- Start with current occupancy for this piece type & color ---
  Bitboard future_occupancy =
      get_piece_occupancies(color_of_piece_of_interest, piece_of_interest);

  // --- Case A: a piece of interest was captured on the target square (normal
  // capture) --- True when the mover is the enemy and a piece-of-interest was
  // on the target.
  bool is_this_piece_the_target =
      (moving_side != color_of_piece_of_interest) &&
      (who_is_on_target_square.first == color_of_piece_of_interest) &&
      (who_is_on_target_square.second == piece_of_interest);

  // --- Case B: the piece of interest is the one that is moving (source ->
  // target) --- NOTE: for pawns this will be true when a pawn moves, including
  // promotions.
  bool is_this_piece_the_moving_piece =
      (moving_side == color_of_piece_of_interest) &&
      (moving_piece == piece_of_interest);

  // --- Case C: the move is a pawn promotion that produces this piece type ---
  // True when the mover is our side, the mover is a pawn, and the promotion
  // result matches piece_of_interest.
  bool is_this_piece_the_promotion_result =
      (moving_side == color_of_piece_of_interest) &&
      (moving_piece == PIECES::PAWN) && (promotion_piece == piece_of_interest);

  // --- Case D: en passant victim removal (victim is always a pawn) ---
  // True when move is en passant and we are tracking the captured pawn's
  // color/type.
  bool is_this_piece_the_en_passant_victim =
      is_en_passant && (color_of_piece_of_interest != moving_side) &&
      (piece_of_interest == PIECES::PAWN);

  // --- Case E: castling rook moves (rook moves from rook_source ->
  // rook_target) ---
  bool is_this_piece_the_castling_rook =
      is_castling && (moving_side == color_of_piece_of_interest) &&
      (piece_of_interest == PIECES::ROOK);

  // Detect pawn-promotion move (mover was a pawn and a promotion occurred) ---
  // We'll use this to suppress adding a pawn to the target square when
  // promotion happens.
  bool is_pawn_promotion =
      (moving_piece == PIECES::PAWN) && (promotion_piece != PIECES::NO_PIECE);

  // -------------------------------------------------------------------------
  // Apply branchless updates (XOR + masks). Multiplying mask by a bool
  // (0/1) enables or disables that specific toggle. XOR toggles the bit.
  // -------------------------------------------------------------------------

  // 1) Normal capture at target: remove captured piece (if it was of
  // piece_of_interest)
  future_occupancy ^=
      Bitboard(target_square.get_mask() * is_this_piece_the_target);

  // 2) En passant victim removal (victim is not on target square)
  if (en_passant_captured_pawn_square.get_index() != ESQUARE::NO_SQUARE) {
    future_occupancy ^= Bitboard(en_passant_captured_pawn_square.get_mask() *
                                 is_this_piece_the_en_passant_victim);
  }

  // 3) Moving piece: remove from source square (always remove the mover from
  // its source)
  //    This MUST happen for pawn promotions as well (pawn leaves its source
  //    square).
  future_occupancy ^=
      Bitboard(source_square.get_mask() * is_this_piece_the_moving_piece);

  // 4) Moving piece: add to target square UNLESS this is a pawn promotion.
  //    IMPORTANT: For pawn promotions we MUST NOT set the pawn bit on the
  //    target square. The promoted piece (handled below) will occupy the
  //    target square instead.
  future_occupancy ^=
      Bitboard(target_square.get_mask() *
               (is_this_piece_the_moving_piece && !is_pawn_promotion));

  // 5) Promotion result: add the promoted piece at the target square (if it
  // matches piece_of_interest)
  future_occupancy ^=
      Bitboard(target_square.get_mask() * is_this_piece_the_promotion_result);

  // 6) Castling rook: remove rook from its starting square (if we are tracking
  // rooks)
  if (rook_source_square.get_index() != ESQUARE::NO_SQUARE) {
    future_occupancy ^= Bitboard(rook_source_square.get_mask() *
                                 is_this_piece_the_castling_rook);
  }

  // 7) Castling rook: add rook to its landing square (if we are tracking rooks)
  if (rook_target_square.get_index() != ESQUARE::NO_SQUARE) {
    future_occupancy ^= Bitboard(rook_target_square.get_mask() *
                                 is_this_piece_the_castling_rook);
  }

  return future_occupancy;
}
