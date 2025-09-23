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
  Bitboard generate_pinned() const;
  Bitboard get_pin_mask(const Bitboard& pinned,
                        const Square& source_square) const;
  Bitboard attackers_to_square(const Square& s,
                               const Bitboard& white_pawn_occupancy,
                               const Bitboard& white_knight_occupancy,
                               const Bitboard& white_bishop_occupancy,
                               const Bitboard& white_rook_occupancy,
                               const Bitboard& white_queen_occupancy,
                               const Bitboard& white_king_occupancy,
                               const Bitboard& black_pawn_occupancy,
                               const Bitboard& black_knight_occupancy,
                               const Bitboard& black_bishop_occupancy,
                               const Bitboard& black_rook_occupancy,
                               const Bitboard& black_queen_occupancy,
                               const Bitboard& black_king_occupancy);
  Bitboard attackers_to_square(const Square& s);
  Bitboard is_our_king_ring_attacked();

  void generate_pawn_promotions(const Square& source_square,
                                const Square& target_square,
                                std::vector<Chess_Move>& output);

  template <PIECES moving_piece>
  void generate_minor_and_major_piece_moves(Attacks a, const Bitboard& pinned,
                                            std::vector<Chess_Move>& output);
};

template <PIECES moving_piece>
inline void Move_Generator::generate_minor_and_major_piece_moves(
    Attacks a, const Bitboard& pinned, std::vector<Chess_Move>& output) {
  const PIECE_COLOR our_side = m_chess_board.get_side_to_move();
  const PIECE_COLOR opposing_side = (PIECE_COLOR)((~our_side) & 0x1);

  const Bitboard check_mask = generate_check_mask();

  const Bitboard enemy_or_empty =
      m_chess_board.get_color_occupancies(opposing_side) |
      (~(m_chess_board.get_both_color_occupancies()));

  Bitboard piece_occupancies =
      m_chess_board.get_piece_occupancies(our_side, moving_piece);

  const Bitboard both_color_occupancies =
      m_chess_board.get_both_color_occupancies();

  while (piece_occupancies.get_board()) {
    Square source_square = Square(piece_occupancies.get_index_of_high_lsb());

    Bitboard pin_mask = get_pin_mask(pinned, source_square);

    uint64_t piece_attacks =
        ((moving_piece == PIECES::KNIGHT) *
         a.get_knight_attacks(source_square).get_board()) +
        ((moving_piece == PIECES::BISHOP) *
         a.get_bishop_attacks(source_square, both_color_occupancies)
             .get_board()) +
        ((moving_piece == PIECES::ROOK) *
         a.get_rook_attacks(source_square, both_color_occupancies)
             .get_board()) +
        ((moving_piece == PIECES::QUEEN) *
         a.get_queen_attacks(source_square, both_color_occupancies)
             .get_board());

    Bitboard piece_moves =
        Bitboard(piece_attacks) & enemy_or_empty & check_mask & pin_mask;

    while (piece_moves.get_board()) {
      Square target_square = piece_moves.get_index_of_high_lsb();

      const auto who_is_on_target_square =
          m_chess_board.what_piece_is_on_square(target_square);

      Chess_Move move = {
          .source_square = ESQUARE(source_square.get_index()),
          .destination_square = ESQUARE(target_square.get_index()),
          .moving_piece = moving_piece,
          .promoted_piece = PIECES::NO_PIECE,
          .captured_piece = who_is_on_target_square.second,
          .is_capture = (who_is_on_target_square.first == opposing_side),
          .is_short_castling = false,
          .is_long_castling = false,
          .castling_rook_source_square = ESQUARE::NO_SQUARE,
          .castling_rook_destination_square = ESQUARE::NO_SQUARE,
          .is_double_pawn_push = false,
          .is_en_passant = false,
          .en_passant_victim_square = ESQUARE::NO_SQUARE,
          .is_promotion = false};

      output.push_back(move);

      piece_moves.unset_square(Square(piece_moves.get_index_of_high_lsb()));
    }

    piece_occupancies.unset_square(
        Square(piece_occupancies.get_index_of_high_lsb()));
  }
}
