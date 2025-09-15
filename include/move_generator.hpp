#include <memory>
#include <vector>

#include "attacks.hpp"
#include "chess_board.hpp"
#include "chess_move.hpp"

const Bitboard FIRST_RANK(18374686479671623680ULL);
const Bitboard SECOND_RANK(71776119061217280ULL);
const Bitboard SEVENTH_RANK(65280ULL);
const Bitboard EIGHTH_RANK(255ULL);
const Bitboard WHITE_SHORT_CASTLE_SQUARES(6917529027641081856ULL);
const Bitboard WHITE_LONG_CASTLE_SQUARES(1008806316530991104ULL);
const Bitboard BLACK_SHORT_CASTLE_SQUARES(96ULL);
const Bitboard BLACK_LONG_CASTLE_SQUARES(14ULL);

class Move_Generator {
 public:
  Move_Generator(const Chess_Board& cb);

  void set_chess_board(const Chess_Board& cb);

  void generate_all_moves(std::vector<Chess_Move>& output);

 private:
  Chess_Board m_chess_board;

  Bitboard generate_check_mask() const;
  Bitboard generate_pin_hv(const Bitboard& one_hot_piece_bitboard) const;
  Bitboard generate_pin_d(const Bitboard& one_hot_piece_bitboard) const;
  Bitboard attackers_to_square(const Square& s);
  Bitboard is_our_king_ring_attacked();

  bool is_check(PIECE_COLOR moving_side, Square opposing_king_square,
                Chess_Board cb);

  void generate_pawn_promotions(const Square& source_square,
                                const Square& target_square,
                                PIECE_COLOR moving_side,
                                const Square& opposing_king_square,
                                std::vector<Chess_Move>& output);

  void generate_minor_and_major_piece_moves(const PIECES moving_piece,
                                            Attacks a,
                                            std::vector<Chess_Move>& output);

  Bitboard calculate_future_occupancy(
      PIECE_COLOR moving_side, PIECE_COLOR color_of_piece_of_interest,
      PIECES piece_of_interest, PIECES moving_piece, Square source_square,
      Square target_square,
      std::pair<PIECE_COLOR, PIECES> who_is_on_target_square,
      PIECES promotion_piece, bool is_en_passant,
      Square en_passant_captured_pawn_square, bool is_castling,
      Square rook_source_square, Square rook_target_square);

  std::shared_ptr<Chess_Board> calculate_future_board_state(
      PIECE_COLOR moving_side, PIECES moving_piece, Square source_square,
      Square target_square,
      std::pair<PIECE_COLOR, PIECES> who_is_on_target_square,
      PIECES promotion_piece, bool is_en_passant,
      Square en_passant_captured_pawn_square, bool is_castling,
      Square rook_source_square, Square rook_target_square);
};
