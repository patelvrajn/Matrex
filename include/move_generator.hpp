#pragma once

#include <memory>

#include "attacks.hpp"
#include "chess_board.hpp"
#include "chess_move.hpp"

extern const Bitboard FIRST_RANK;
extern const Bitboard SECOND_RANK;
extern const Bitboard SEVENTH_RANK;
extern const Bitboard EIGHTH_RANK;

extern const std::array<std::array<Square, NUM_OF_CASTLING_TYPES>,
                        NUM_OF_PLAYERS>
    CASTLING_KING_DESTINATION_SQUARES;

extern const std::array<std::array<Square, NUM_OF_CASTLING_TYPES>,
                        NUM_OF_PLAYERS>
    CASTLING_ROOK_DESTINATION_SQUARES;

enum MOVE_GENERATION_TYPE { ALL, TACTICAL, QUIET };

class Move_Generator {
 public:
  Move_Generator(const Chess_Board& cb);

  template <MOVE_GENERATION_TYPE gen_type>
  void generate_all_moves(Chess_Move_List& output);

  bool is_side_to_move_in_check();

 private:
  const Chess_Board& m_chess_board;

  bool m_enpassantable_checker;
  bool m_side_to_move_in_check;

  Bitboard generate_check_mask();
  Bitboard generate_pinned() const;
  bool is_pinned(const Bitboard& pinned, const Square& source_square) const;
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

  inline void generate_pawn_promotions(const Square& source_square,
                                       const Square& target_square,
                                       Chess_Move_List& output);

  template <PIECE_COLOR moving_side>
  inline void generate_single_push_promotion_pawn_moves(
      const Bitboard& pinned, const Bitboard& check_mask,
      Chess_Move_List& output);

  template <PIECE_COLOR moving_side>
  inline void generate_single_push_non_promotion_pawn_moves(
      const Bitboard& pinned, const Bitboard& check_mask,
      Chess_Move_List& output);

  template <PIECE_COLOR moving_side>
  inline void generate_double_push_pawn_moves(const Bitboard& pinned,
                                              const Bitboard& check_mask,
                                              Chess_Move_List& output);

  template <PIECE_COLOR moving_side>
  inline void generate_en_passant_captures(const Bitboard& pinned,
                                           const Bitboard& check_mask,
                                           Chess_Move_List& output);

  template <PIECE_COLOR moving_side>
  inline void generate_non_promotion_pawn_captures(const Bitboard& pinned,
                                                   const Bitboard& check_mask,
                                                   Chess_Move_List& output);

  template <PIECE_COLOR moving_side>
  inline void generate_promotion_pawn_captures(const Bitboard& pinned,
                                               const Bitboard& check_mask,
                                               Chess_Move_List& output);

  template <MOVE_GENERATION_TYPE gen_type, PIECE_COLOR moving_side,
            PIECES moving_piece>
  inline void generate_minor_and_major_piece_moves(const Bitboard& pinned,
                                                   const Bitboard& check_mask,
                                                   Chess_Move_List& output);

  template <MOVE_GENERATION_TYPE gen_type, PIECE_COLOR moving_side>
  inline void generate_king_moves(Chess_Move_List& output);

  template <PIECE_COLOR moving_side, CASTLING_TYPE castle_type>
  inline void generate_castling_moves(const Bitboard& pinned,
                                      Chess_Move_List& output);
};

inline void Move_Generator::generate_pawn_promotions(
    const Square& source_square, const Square& destination_square,
    Chess_Move_List& output) {
  const auto who_is_on_destination_square =
      m_chess_board.what_piece_is_on_square(destination_square);

  // Pawn to knight promotion
  Chess_Move move = {
      .source_square = (ESQUARE)source_square.get_index(),
      .destination_square = (ESQUARE)destination_square.get_index(),
      .moving_piece = PIECES::PAWN,
      .promoted_piece = PIECES::KNIGHT,
      .captured_piece = who_is_on_destination_square.second,
      .is_capture = (who_is_on_destination_square.second != PIECES::NO_PIECE),
      .is_short_castling = false,
      .is_long_castling = false,
      .castling_rook_source_square = ESQUARE::NO_SQUARE,
      .castling_rook_destination_square = ESQUARE::NO_SQUARE,
      .is_double_pawn_push = false,
      .is_en_passant = false,
      .en_passant_victim_square = ESQUARE::NO_SQUARE,
      .is_promotion = true};

  output.append(move);

  // Pawn to bishop promotion.
  move.promoted_piece = PIECES::BISHOP;
  output.append(move);

  // Pawn to rook promotion.
  move.promoted_piece = PIECES::ROOK;
  output.append(move);

  // Pawn to queen promotion.
  move.promoted_piece = PIECES::QUEEN;
  output.append(move);
}

template <PIECE_COLOR moving_side>
inline void Move_Generator::generate_single_push_promotion_pawn_moves(
    const Bitboard& pinned, const Bitboard& check_mask,
    Chess_Move_List& output) {
  const Bitboard pawns =
      m_chess_board.get_piece_occupancies(moving_side, PIECES::PAWN);

  const Bitboard single_pawn_pushes =
      (moving_side == PIECE_COLOR::WHITE)
          ? (pawns >> 8) & (~m_chess_board.get_both_color_occupancies())
          : (pawns << 8) & (~m_chess_board.get_both_color_occupancies());

  const Bitboard promotion_single_pushes =
      (moving_side == PIECE_COLOR::WHITE) ? (single_pawn_pushes & EIGHTH_RANK)
                                          : (single_pawn_pushes & FIRST_RANK);

  for (const Square& destination_square : promotion_single_pushes) {
    const Square source_square((moving_side == PIECE_COLOR::WHITE)
                                   ? (destination_square.get_index() + 8)
                                   : (destination_square.get_index() - 8));

    const Bitboard pin_mask = get_pin_mask(pinned, source_square);

    const Bitboard pawn_moves =
        Bitboard(destination_square.get_mask()) & check_mask & pin_mask;

    if (pawn_moves != 0) {
      generate_pawn_promotions(source_square, destination_square, output);
    }
  }
}

template <PIECE_COLOR moving_side>
inline void Move_Generator::generate_single_push_non_promotion_pawn_moves(
    const Bitboard& pinned, const Bitboard& check_mask,
    Chess_Move_List& output) {
  const Bitboard pawns =
      m_chess_board.get_piece_occupancies(moving_side, PIECES::PAWN);

  const Bitboard single_pawn_pushes =
      (moving_side == PIECE_COLOR::WHITE)
          ? (pawns >> 8) & (~m_chess_board.get_both_color_occupancies())
          : (pawns << 8) & (~m_chess_board.get_both_color_occupancies());

  const Bitboard promotion_single_pushes =
      (moving_side == PIECE_COLOR::WHITE) ? (single_pawn_pushes & EIGHTH_RANK)
                                          : (single_pawn_pushes & FIRST_RANK);

  const Bitboard non_promotion_single_pushes =
      single_pawn_pushes & (~promotion_single_pushes);

  for (const Square& destination_square : non_promotion_single_pushes) {
    const Square source_square((moving_side == PIECE_COLOR::WHITE)
                                   ? (destination_square.get_index() + 8)
                                   : (destination_square.get_index() - 8));

    const Bitboard pin_mask = get_pin_mask(pinned, source_square);

    const Bitboard pawn_moves =
        Bitboard(destination_square.get_mask()) & check_mask & pin_mask;

    if (pawn_moves != 0) {
      const Chess_Move move = {
          .source_square = (ESQUARE)source_square.get_index(),
          .destination_square = (ESQUARE)destination_square.get_index(),
          .moving_piece = PIECES::PAWN,
          .promoted_piece = PIECES::NO_PIECE,
          .captured_piece = PIECES::NO_PIECE,
          .is_capture = false,
          .is_short_castling = false,
          .is_long_castling = false,
          .castling_rook_source_square = ESQUARE::NO_SQUARE,
          .castling_rook_destination_square = ESQUARE::NO_SQUARE,
          .is_double_pawn_push = false,
          .is_en_passant = false,
          .en_passant_victim_square = ESQUARE::NO_SQUARE,
          .is_promotion = false};

      output.append(move);
    }
  }
}

template <PIECE_COLOR moving_side>
inline void Move_Generator::generate_double_push_pawn_moves(
    const Bitboard& pinned, const Bitboard& check_mask,
    Chess_Move_List& output) {
  const Bitboard pawns =
      m_chess_board.get_piece_occupancies(moving_side, PIECES::PAWN);

  const Bitboard not_occupied = (~m_chess_board.get_both_color_occupancies());

  const Bitboard double_pawn_pushes =
      (moving_side == PIECE_COLOR::WHITE)
          ? (((((pawns & SECOND_RANK) >> 8) & not_occupied) >> 8) &
             not_occupied)
          : (((((pawns & SEVENTH_RANK) << 8) & not_occupied) << 8) &
             not_occupied);

  for (const Square& destination_square : double_pawn_pushes) {
    const Square source_square((moving_side == PIECE_COLOR::WHITE)
                                   ? (destination_square.get_index() + 16)
                                   : (destination_square.get_index() - 16));

    const Bitboard pin_mask = get_pin_mask(pinned, source_square);

    const Bitboard pawn_moves =
        Bitboard(destination_square.get_mask()) & check_mask & pin_mask;

    if (pawn_moves != 0) {
      const Chess_Move move = {
          .source_square = (ESQUARE)source_square.get_index(),
          .destination_square = (ESQUARE)destination_square.get_index(),
          .moving_piece = PIECES::PAWN,
          .promoted_piece = PIECES::NO_PIECE,
          .captured_piece = PIECES::NO_PIECE,
          .is_capture = false,
          .is_short_castling = false,
          .is_long_castling = false,
          .castling_rook_source_square = ESQUARE::NO_SQUARE,
          .castling_rook_destination_square = ESQUARE::NO_SQUARE,
          .is_double_pawn_push = true,
          .is_en_passant = false,
          .en_passant_victim_square = ESQUARE::NO_SQUARE,
          .is_promotion = false};

      output.append(move);
    }
  }
}

template <PIECE_COLOR moving_side>
inline void Move_Generator::generate_en_passant_captures(
    const Bitboard& pinned, const Bitboard& check_mask,
    Chess_Move_List& output) {
  Attacks a;

  constexpr PIECE_COLOR opposing_side = (PIECE_COLOR)((~moving_side) & 0x1);

  const Square king_square = m_chess_board.get_king_square(moving_side);

  Square en_passant_square = m_chess_board.get_en_passant_square();
  if (en_passant_square.get_index() != ESQUARE::NO_SQUARE) {
    const Bitboard en_passant_pawns =
        a.get_pawn_attacks(en_passant_square, opposing_side) &
        m_chess_board.get_piece_occupancies(moving_side, PIECES::PAWN);

    for (const Square& source_square : en_passant_pawns) {
      const Square en_passant_victim_square =
          m_chess_board.get_en_passant_victim_square();

      const Bitboard pin_mask = get_pin_mask(pinned, source_square);

      // Special case where the checker can be removed via en passant.
      const Bitboard can_remove_checker =
          m_chess_board.get_en_passant_square().get_mask() &
          ((uint64_t)(-(uint64_t)(m_enpassantable_checker == true)));

      const Bitboard can_do_enpassant = Bitboard(en_passant_square.get_mask()) &
                                        (check_mask | can_remove_checker) &
                                        pin_mask;

      // Takes care of the situation where an enemy rook or queen is on the same
      // rank as a friendly enpassant alongside the friendly king. En passant is
      // not possible because the friendly king would be in check after the
      // move.
      const Bitboard en_passant_rank_mask =
          Bitboard::get_rank_mask(source_square);
      const Bitboard occupancy_without_en_passant_pawns =
          m_chess_board.get_both_color_occupancies() ^
          en_passant_victim_square.get_mask() ^ source_square.get_mask();
      const Bitboard enemy_orthogonal_movers =
          m_chess_board.get_piece_occupancies(opposing_side, PIECES::ROOK) |
          m_chess_board.get_piece_occupancies(opposing_side, PIECES::QUEEN);
      const Bitboard ortogonal_attacks_to_king_without_en_passant_pawns =
          (a.get_rook_attacks(king_square, occupancy_without_en_passant_pawns) &
           en_passant_rank_mask) &
          enemy_orthogonal_movers;

      if ((can_do_enpassant != 0) &&
          (ortogonal_attacks_to_king_without_en_passant_pawns == 0)) {
        const Chess_Move move = {
            .source_square = (ESQUARE)source_square.get_index(),
            .destination_square = (ESQUARE)en_passant_square.get_index(),
            .moving_piece = PIECES::PAWN,
            .promoted_piece = PIECES::NO_PIECE,
            .captured_piece =
                PIECES::NO_PIECE,  // Yes we capture a PIECES::PAWN but this
                                   // becomes a hinderance when writing the
                                   // calculate next board state function.
            .is_capture = false,   // Same hindereance here as explained above.
            .is_short_castling = false,
            .is_long_castling = false,
            .castling_rook_source_square = ESQUARE::NO_SQUARE,
            .castling_rook_destination_square = ESQUARE::NO_SQUARE,
            .is_double_pawn_push = false,
            .is_en_passant = true,
            .en_passant_victim_square =
                (ESQUARE)en_passant_victim_square.get_index(),
            .is_promotion = false};

        output.append(move);
      }
    }
  }
}

template <PIECE_COLOR moving_side>
inline void Move_Generator::generate_non_promotion_pawn_captures(
    const Bitboard& pinned, const Bitboard& check_mask,
    Chess_Move_List& output) {
  Attacks a;

  constexpr PIECE_COLOR opposing_side = (PIECE_COLOR)((~moving_side) & 0x1);

  const Bitboard pawns =
      m_chess_board.get_piece_occupancies(moving_side, PIECES::PAWN);

  for (const Square& source_square : pawns) {
    const Bitboard pin_mask = get_pin_mask(pinned, source_square);

    const Bitboard pawn_attacks =
        a.get_pawn_attacks(source_square, moving_side) &
        m_chess_board.get_color_occupancies(opposing_side) & check_mask &
        pin_mask;

    const Bitboard promotable_pawn_attacks = (moving_side == PIECE_COLOR::WHITE)
                                                 ? pawn_attacks & EIGHTH_RANK
                                                 : pawn_attacks & FIRST_RANK;
    const Bitboard non_promotable_pawn_attacks =
        pawn_attacks & (~promotable_pawn_attacks);

    for (const Square& destination_square : non_promotable_pawn_attacks) {
      const auto who_is_on_destination_square =
          m_chess_board.what_piece_is_on_square(destination_square);

      const Chess_Move move = {
          .source_square = (ESQUARE)source_square.get_index(),
          .destination_square = (ESQUARE)destination_square.get_index(),
          .moving_piece = PIECES::PAWN,
          .promoted_piece = PIECES::NO_PIECE,
          .captured_piece = who_is_on_destination_square.second,
          .is_capture = true,
          .is_short_castling = false,
          .is_long_castling = false,
          .castling_rook_source_square = ESQUARE::NO_SQUARE,
          .castling_rook_destination_square = ESQUARE::NO_SQUARE,
          .is_double_pawn_push = false,
          .is_en_passant = false,
          .en_passant_victim_square = ESQUARE::NO_SQUARE,
          .is_promotion = false};

      output.append(move);
    }
  }
}

template <PIECE_COLOR moving_side>
inline void Move_Generator::generate_promotion_pawn_captures(
    const Bitboard& pinned, const Bitboard& check_mask,
    Chess_Move_List& output) {
  Attacks a;

  constexpr PIECE_COLOR opposing_side = (PIECE_COLOR)((~moving_side) & 0x1);

  const Bitboard pawns =
      m_chess_board.get_piece_occupancies(moving_side, PIECES::PAWN);

  for (const Square& source_square : pawns) {
    const Bitboard pin_mask = get_pin_mask(pinned, source_square);

    const Bitboard pawn_attacks =
        a.get_pawn_attacks(source_square, moving_side) &
        m_chess_board.get_color_occupancies(opposing_side) & check_mask &
        pin_mask;

    const Bitboard promotable_pawn_attacks = (moving_side == PIECE_COLOR::WHITE)
                                                 ? pawn_attacks & EIGHTH_RANK
                                                 : pawn_attacks & FIRST_RANK;

    for (const Square& destination_square : promotable_pawn_attacks) {
      generate_pawn_promotions(source_square, destination_square, output);
    }
  }
}

template <MOVE_GENERATION_TYPE gen_type, PIECE_COLOR moving_side,
          PIECES moving_piece>
inline void Move_Generator::generate_minor_and_major_piece_moves(
    const Bitboard& pinned, const Bitboard& check_mask,
    Chess_Move_List& output) {
  Attacks a;

  constexpr PIECE_COLOR opposing_side = (PIECE_COLOR)((~moving_side) & 0x1);

  const Bitboard both_color_occupancies =
      m_chess_board.get_both_color_occupancies();

  Bitboard enemy_or_empty;

  if constexpr (gen_type == MOVE_GENERATION_TYPE::TACTICAL) {
    enemy_or_empty = m_chess_board.get_color_occupancies(opposing_side);
  }

  if constexpr (gen_type == MOVE_GENERATION_TYPE::QUIET) {
    enemy_or_empty = (~(both_color_occupancies));
  }

  if constexpr (gen_type == MOVE_GENERATION_TYPE::ALL) {
    enemy_or_empty = m_chess_board.get_color_occupancies(opposing_side) |
                     (~(both_color_occupancies));
  }

  const Bitboard piece_occupancies =
      m_chess_board.get_piece_occupancies(moving_side, moving_piece);

  for (const Square& source_square : piece_occupancies) {
    const Bitboard pin_mask = get_pin_mask(pinned, source_square);

    const uint64_t piece_attacks =
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

    const Bitboard piece_moves =
        Bitboard(piece_attacks) & enemy_or_empty & check_mask & pin_mask;

    for (const Square& destination_square : piece_moves) {
      const auto who_is_on_destination_square =
          m_chess_board.what_piece_is_on_square(destination_square);

      const Chess_Move move = {
          .source_square = (ESQUARE)(source_square.get_index()),
          .destination_square = (ESQUARE)(destination_square.get_index()),
          .moving_piece = moving_piece,
          .promoted_piece = PIECES::NO_PIECE,
          .captured_piece = who_is_on_destination_square.second,
          .is_capture = (who_is_on_destination_square.first == opposing_side),
          .is_short_castling = false,
          .is_long_castling = false,
          .castling_rook_source_square = ESQUARE::NO_SQUARE,
          .castling_rook_destination_square = ESQUARE::NO_SQUARE,
          .is_double_pawn_push = false,
          .is_en_passant = false,
          .en_passant_victim_square = ESQUARE::NO_SQUARE,
          .is_promotion = false};

      output.append(move);
    }
  }
}

template <MOVE_GENERATION_TYPE gen_type, PIECE_COLOR moving_side>
inline void Move_Generator::generate_king_moves(Chess_Move_List& output) {
  Attacks a;

  constexpr PIECE_COLOR opposing_side = (PIECE_COLOR)((~moving_side) & 0x1);

  const Bitboard both_color_occupancies =
      m_chess_board.get_both_color_occupancies();

  Bitboard enemy_or_empty;

  if constexpr (gen_type == MOVE_GENERATION_TYPE::TACTICAL) {
    enemy_or_empty = m_chess_board.get_color_occupancies(opposing_side);
  }

  if constexpr (gen_type == MOVE_GENERATION_TYPE::QUIET) {
    enemy_or_empty = (~(both_color_occupancies));
  }

  if constexpr (gen_type == MOVE_GENERATION_TYPE::ALL) {
    enemy_or_empty = m_chess_board.get_color_occupancies(opposing_side) |
                     (~(both_color_occupancies));
  }

  const Square king_square = m_chess_board.get_king_square(moving_side);

  const Bitboard king_moves = a.get_king_attacks(king_square) & enemy_or_empty &
                              (~is_our_king_ring_attacked());

  for (const Square& destination_square : king_moves) {
    const auto who_is_on_destination_square =
        m_chess_board.what_piece_is_on_square(destination_square);

    Chess_Move move = {
        .source_square = (ESQUARE)king_square.get_index(),
        .destination_square = (ESQUARE)destination_square.get_index(),
        .moving_piece = PIECES::KING,
        .promoted_piece = PIECES::NO_PIECE,
        .captured_piece = who_is_on_destination_square.second,
        .is_capture = (who_is_on_destination_square.first == opposing_side),
        .is_short_castling = false,
        .is_long_castling = false,
        .castling_rook_source_square = ESQUARE::NO_SQUARE,
        .castling_rook_destination_square = ESQUARE::NO_SQUARE,
        .is_double_pawn_push = false,
        .is_en_passant = false,
        .en_passant_victim_square = ESQUARE::NO_SQUARE,
        .is_promotion = false};

    output.append(move);
  }
}

// A generalized function to generate legal castling moves in both standard
// chess and Fischer Random chess.
template <PIECE_COLOR moving_side, CASTLING_TYPE castle_type>
inline void Move_Generator::generate_castling_moves(const Bitboard& pinned,
                                                    Chess_Move_List& output) {
  bool has_castling_rights;

  if constexpr ((moving_side == PIECE_COLOR::WHITE) &&
                (castle_type == CASTLING_TYPE::KINGSIDE)) {
    has_castling_rights = m_chess_board.does_white_have_short_castle_rights();
  } else if constexpr ((moving_side == PIECE_COLOR::WHITE) &&
                       (castle_type == CASTLING_TYPE::QUEENSIDE)) {
    has_castling_rights = m_chess_board.does_white_have_long_castle_rights();
  } else if constexpr ((moving_side == PIECE_COLOR::BLACK) &&
                       (castle_type == CASTLING_TYPE::KINGSIDE)) {
    has_castling_rights = m_chess_board.does_black_have_short_castle_rights();
  } else if constexpr ((moving_side == PIECE_COLOR::BLACK) &&
                       (castle_type == CASTLING_TYPE::QUEENSIDE)) {
    has_castling_rights = m_chess_board.does_black_have_long_castle_rights();
  }

  if (has_castling_rights) {
    constexpr PIECE_COLOR opposing_side = (PIECE_COLOR)((~moving_side) & 0x1);

    const Bitboard enemy_occupancies =
        m_chess_board.get_color_occupancies(opposing_side);

    const Bitboard both_occupancies =
        m_chess_board.get_both_color_occupancies();

    const Square king_source_square =
        m_chess_board.get_king_square(moving_side);

    const Square rook_source_square =
        m_chess_board.get_castling_rook_source_square(moving_side, castle_type);

    // Are squares between the moving side's king's source square to destination
    // square (excluding source square, including destination square, excluding
    // the rook's (the rook being castled with) square) empty? Also, if the
    // destination square is the same as the source square ensure this is
    // counted as an empty square thus, we use the following XOR trick:
    // (both_occupancies ^ king_source_square.get_mask()).
    const Square king_destination_square =
        CASTLING_KING_DESTINATION_SQUARES[moving_side][castle_type];

    const Bitboard king_should_be_empty_squares =
        ((Bitboard::get_between_squares_mask(king_source_square,
                                             king_destination_square) |
          king_destination_square.get_mask()) &
         (~rook_source_square.get_mask()));

    bool is_king_squares_empty =
        ((king_should_be_empty_squares &
          (both_occupancies ^ king_source_square.get_mask())) == 0);

    // Are squares between the moving side's rook's (the rook being castled
    // with) source square to destination square (excluding source square,
    // including destination square, excluding the moving side's king's square)
    // empty? Just like with the king, ensure that if the destination square and
    // source square are the same then it counts an empty square, a similar XOR
    // trick is used here. Additionally, in Fischer Random Chess it is possible
    // to pin a castling rook to the friendly king in this case, castling is not
    // allowed.
    const Square rook_destination_square =
        CASTLING_ROOK_DESTINATION_SQUARES[moving_side][castle_type];

    const Bitboard rook_should_be_empty_squares =
        ((Bitboard::get_between_squares_mask(rook_source_square,
                                             rook_destination_square) |
          rook_destination_square.get_mask()) &
         (~king_source_square.get_mask()));

    bool is_rook_squares_empty =
        ((rook_should_be_empty_squares &
          (both_occupancies ^ rook_source_square.get_mask())) == 0);

    bool is_rook_pinned = is_pinned(pinned, rook_source_square);

    // Is the path from the king's source square to destination square
    // (including source & destination square) not attacked?
    const Bitboard king_should_be_not_attacked_squares =
        Bitboard::get_between_squares_mask(king_source_square,
                                           king_destination_square) |
        king_source_square.get_mask() | king_destination_square.get_mask();

    bool is_king_safe = true;
    for (const Square& s : king_should_be_not_attacked_squares) {
      const bool is_square_safe =
          ((attackers_to_square(s) & enemy_occupancies).get_board() == 0);
      is_king_safe = (is_king_safe && is_square_safe);
    }

    if (is_king_squares_empty && is_rook_squares_empty && is_king_safe &&
        (!is_rook_pinned)) {
      const Chess_Move move = {
          .source_square = (ESQUARE)king_source_square.get_index(),
          .destination_square = (ESQUARE)king_destination_square.get_index(),
          .moving_piece = PIECES::KING,
          .promoted_piece = PIECES::NO_PIECE,
          .captured_piece = PIECES::NO_PIECE,
          .is_capture = false,
          .is_short_castling = (castle_type == CASTLING_TYPE::KINGSIDE),
          .is_long_castling = (castle_type == CASTLING_TYPE::QUEENSIDE),
          .castling_rook_source_square =
              (ESQUARE)rook_source_square.get_index(),
          .castling_rook_destination_square =
              (ESQUARE)rook_destination_square.get_index(),
          .is_double_pawn_push = false,
          .is_en_passant = false,
          .en_passant_victim_square = ESQUARE::NO_SQUARE,
          .is_promotion = false};

      output.append(move);
    }
  }
}

template <MOVE_GENERATION_TYPE gen_type>
void Move_Generator::generate_all_moves(Chess_Move_List& output) {
  const PIECE_COLOR moving_side = m_chess_board.get_side_to_move();

  const Bitboard check_mask = generate_check_mask();
  const Bitboard pinned = generate_pinned();

  if (moving_side == PIECE_COLOR::WHITE) {
    /***************************************************************************
     * TACTICAL MOVE GENERATION
     ***************************************************************************/
    if constexpr ((gen_type == MOVE_GENERATION_TYPE::ALL) ||
                  (gen_type == MOVE_GENERATION_TYPE::TACTICAL)) {
      generate_single_push_promotion_pawn_moves<PIECE_COLOR::WHITE>(
          pinned, check_mask, output);
      generate_en_passant_captures<PIECE_COLOR::WHITE>(pinned, check_mask,
                                                       output);
      generate_non_promotion_pawn_captures<PIECE_COLOR::WHITE>(
          pinned, check_mask, output);
      generate_promotion_pawn_captures<PIECE_COLOR::WHITE>(pinned, check_mask,
                                                           output);
    }

    /***************************************************************************
     * QUIET MOVE GENERATION
     ***************************************************************************/
    if constexpr ((gen_type == MOVE_GENERATION_TYPE::ALL) ||
                  (gen_type == MOVE_GENERATION_TYPE::QUIET)) {
      generate_single_push_non_promotion_pawn_moves<PIECE_COLOR::WHITE>(
          pinned, check_mask, output);
      generate_double_push_pawn_moves<PIECE_COLOR::WHITE>(pinned, check_mask,
                                                          output);
      generate_castling_moves<PIECE_COLOR::WHITE, CASTLING_TYPE::KINGSIDE>(
          pinned, output);
      generate_castling_moves<PIECE_COLOR::WHITE, CASTLING_TYPE::QUEENSIDE>(
          pinned, output);
    }

    /***************************************************************************
     * GENERAL MOVE GENERATION
     ***************************************************************************/
    generate_minor_and_major_piece_moves<gen_type, PIECE_COLOR::WHITE,
                                         PIECES::KNIGHT>(pinned, check_mask,
                                                         output);
    generate_minor_and_major_piece_moves<gen_type, PIECE_COLOR::WHITE,
                                         PIECES::BISHOP>(pinned, check_mask,
                                                         output);
    generate_minor_and_major_piece_moves<gen_type, PIECE_COLOR::WHITE,
                                         PIECES::ROOK>(pinned, check_mask,
                                                       output);
    generate_minor_and_major_piece_moves<gen_type, PIECE_COLOR::WHITE,
                                         PIECES::QUEEN>(pinned, check_mask,
                                                        output);
    generate_king_moves<gen_type, PIECE_COLOR::WHITE>(output);

  } else {
    /***************************************************************************
     * TACTICAL MOVE GENERATION
     ***************************************************************************/
    if constexpr ((gen_type == MOVE_GENERATION_TYPE::ALL) ||
                  (gen_type == MOVE_GENERATION_TYPE::TACTICAL)) {
      generate_single_push_promotion_pawn_moves<PIECE_COLOR::BLACK>(
          pinned, check_mask, output);
      generate_en_passant_captures<PIECE_COLOR::BLACK>(pinned, check_mask,
                                                       output);
      generate_non_promotion_pawn_captures<PIECE_COLOR::BLACK>(
          pinned, check_mask, output);
      generate_promotion_pawn_captures<PIECE_COLOR::BLACK>(pinned, check_mask,
                                                           output);
    }

    /***************************************************************************
     * QUIET MOVE GENERATION
     ***************************************************************************/
    if constexpr ((gen_type == MOVE_GENERATION_TYPE::ALL) ||
                  (gen_type == MOVE_GENERATION_TYPE::QUIET)) {
      generate_single_push_non_promotion_pawn_moves<PIECE_COLOR::BLACK>(
          pinned, check_mask, output);
      generate_double_push_pawn_moves<PIECE_COLOR::BLACK>(pinned, check_mask,
                                                          output);
      generate_castling_moves<PIECE_COLOR::BLACK, CASTLING_TYPE::KINGSIDE>(
          pinned, output);
      generate_castling_moves<PIECE_COLOR::BLACK, CASTLING_TYPE::QUEENSIDE>(
          pinned, output);
    }

    /***************************************************************************
     * GENERAL MOVE GENERATION
     ***************************************************************************/
    generate_minor_and_major_piece_moves<gen_type, PIECE_COLOR::BLACK,
                                         PIECES::KNIGHT>(pinned, check_mask,
                                                         output);
    generate_minor_and_major_piece_moves<gen_type, PIECE_COLOR::BLACK,
                                         PIECES::BISHOP>(pinned, check_mask,
                                                         output);
    generate_minor_and_major_piece_moves<gen_type, PIECE_COLOR::BLACK,
                                         PIECES::ROOK>(pinned, check_mask,
                                                       output);
    generate_minor_and_major_piece_moves<gen_type, PIECE_COLOR::BLACK,
                                         PIECES::QUEEN>(pinned, check_mask,
                                                        output);
    generate_king_moves<gen_type, PIECE_COLOR::BLACK>(output);
  }
}
