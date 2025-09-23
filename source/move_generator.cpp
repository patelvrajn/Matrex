#include "move_generator.hpp"

#include <iostream>

Move_Generator::Move_Generator(const Chess_Board& cb) : m_chess_board(cb) {}

void Move_Generator::set_chess_board(const Chess_Board& cb) {
  m_chess_board = cb;
}

// Function: generate_check_mask
// Purpose: Creates a "check mask" bitboard used during move generation.
//          - If no check: all moves are legal (mask = all 1's).
//          - If single check: only moves that block or capture the checker are
//          legal.
//          - If double check: only king moves are legal (mask = empty).
Bitboard Move_Generator::generate_check_mask() const {
  Bitboard checkers;  // Bitboard that will hold all pieces currently checking
                      // the king.
  Attacks
      a;  // Attack helper class to compute attack masks for different pieces.

  // The color of our pieces.
  const PIECE_COLOR our_side = m_chess_board.get_side_to_move();

  // Figure out the color of the *opponent* (the side delivering checks).
  // get_side_to_move() = side whose turn it is.
  // ~ flips the bit, & 0x1 ensures we only keep 1-bit color info (WHITE=0,
  // BLACK=1).
  const PIECE_COLOR opposing_side = (PIECE_COLOR)((~our_side) & 0x1);

  // The square our king is on.
  const Square our_king_square = m_chess_board.get_king_square(our_side);

  // The squares on a bitboard occupied by any side's pieces.
  const Bitboard both_color_occupancies =
      m_chess_board.get_both_color_occupancies();

  // --- Find all enemy pawns that give check ---
  // 1. Get pawn attack squares relative to the king's position.
  // 2. AND it with all opponent pawns currently on the board.
  // 3. Result: opponent pawns that are directly attacking our king.
  checkers |=
      (a.get_pawn_attacks(our_king_square, our_side) &
       m_chess_board.get_piece_occupancies(opposing_side, PIECES::PAWN));

  // --- Find all enemy knights giving check ---
  // Similar logic: knight attack mask from king's square & opponent knights.
  checkers |=
      (a.get_knight_attacks(our_king_square) &
       m_chess_board.get_piece_occupancies(opposing_side, PIECES::KNIGHT));

  // --- Find all enemy bishops or queens giving check (diagonal sliders) ---
  // Use bishop attack rays from the king's square, intersect with enemy bishops
  // + queens.
  checkers |=
      (a.get_bishop_attacks(our_king_square, both_color_occupancies) &
       (m_chess_board.get_piece_occupancies(opposing_side, PIECES::BISHOP) |
        m_chess_board.get_piece_occupancies(opposing_side, PIECES::QUEEN)));

  // --- Find all enemy rooks or queens giving check (orthogonal sliders) ---
  // Use rook attack rays from the king's square, intersect with enemy rooks +
  // queens.
  checkers |=
      (a.get_rook_attacks(our_king_square, both_color_occupancies) &
       (m_chess_board.get_piece_occupancies(opposing_side, PIECES::ROOK) |
        m_chess_board.get_piece_occupancies(opposing_side, PIECES::QUEEN)));

  // --- Case 1: No check at all ---
  // If "checkers" is empty (king is not under check),
  // return a mask of all 1s (~0ULL), meaning all moves are legal.
  if (checkers == Bitboard(0)) {
    return Bitboard(~0ULL);
  }

  // --- Case 2: Double check detection ---
  // A double check means TWO pieces are giving check simultaneously.
  // To detect: temporarily clear the least significant checker
  // and see if any other bits remain set.
  Bitboard double_check = checkers;
  double_check.unset_square(Square(checkers.get_index_of_high_lsb()));

  // If still non-empty, it means we had more than one checker.
  // In double check, ONLY king moves are legal, so return empty mask.
  if (double_check.get_board()) {
    return Bitboard(0);
  }

  // --- Case 3: Single check ---
  // If only ONE checker exists, then legal moves must either:
  //   (1) Capture the checking piece, OR
  //   (2) Block the attack ray (only possible against sliders:
  //   bishop/rook/queen).
  // So we generate a mask of all squares on the line between the checker and
  // the king, then OR it with the checker’s square.
  return (Bitboard(checkers.get_between_squares_mask(
              our_king_square, Square(checkers.get_index_of_high_lsb()))) |
          checkers);
}

Bitboard Move_Generator::generate_pinned() const {
  Attacks a;

  // Get which side is moving
  const PIECE_COLOR our_side = m_chess_board.get_side_to_move();

  // Opposite color (side not moving) — bit hack: flip 0 ↔ 1
  const PIECE_COLOR opposing_side = (PIECE_COLOR)((~our_side) & 0x1);

  const Bitboard our_king_occupancy =
      m_chess_board.get_piece_occupancies(our_side, PIECES::KING);
  const Square our_king_square = m_chess_board.get_king_square(our_side);

  const Bitboard friendly_pieces =
      m_chess_board.get_color_occupancies(our_side);
  const Bitboard enemy_pieces =
      m_chess_board.get_color_occupancies(opposing_side);

  const Bitboard enemy_orthogonal_pieces =
      m_chess_board.get_piece_occupancies(opposing_side, PIECES::ROOK) |
      m_chess_board.get_piece_occupancies(opposing_side, PIECES::QUEEN);
  const Bitboard enemy_diagonal_pieces =
      m_chess_board.get_piece_occupancies(opposing_side, PIECES::BISHOP) |
      m_chess_board.get_piece_occupancies(opposing_side, PIECES::QUEEN);

  const Bitboard orthogonal_rays =
      a.get_rook_attacks(our_king_square, enemy_pieces);
  const Bitboard diagonal_rays =
      a.get_bishop_attacks(our_king_square, enemy_pieces);

  Bitboard orthogonal_pinners = enemy_orthogonal_pieces & orthogonal_rays;
  Bitboard diagonal_pinners = enemy_diagonal_pieces & diagonal_rays;

  Bitboard pinned;

  while (orthogonal_pinners.get_board()) {
    const Square pinner_square =
        Square(orthogonal_pinners.get_index_of_high_lsb());

    const Bitboard ray_from_pinner_to_king =
        orthogonal_rays & a.get_rook_attacks(pinner_square, our_king_occupancy);

    const Bitboard potentially_pinned =
        ray_from_pinner_to_king & friendly_pieces;

    if (potentially_pinned.high_bit_count() == 1) {
      pinned |= potentially_pinned;
    }

    orthogonal_pinners.unset_square(pinner_square);
  }

  while (diagonal_pinners.get_board()) {
    const Square pinner_square =
        Square(diagonal_pinners.get_index_of_high_lsb());

    const Bitboard ray_from_pinner_to_king =
        diagonal_rays & a.get_bishop_attacks(pinner_square, our_king_occupancy);

    const Bitboard potentially_pinned =
        ray_from_pinner_to_king & friendly_pieces;

    if (potentially_pinned.high_bit_count() == 1) {
      pinned |= potentially_pinned;
    }

    diagonal_pinners.unset_square(pinner_square);
  }

  return pinned;
}

Bitboard Move_Generator::get_pin_mask(const Bitboard& pinned,
                                      const Square& source_square) const {
  // Get which side is moving
  const PIECE_COLOR our_side = m_chess_board.get_side_to_move();

  const Square our_king_square = m_chess_board.get_king_square(our_side);

  bool is_pinned =
      ((pinned & Bitboard(source_square.get_mask())).get_board() != 0);

  if (is_pinned) {
    return Bitboard::get_infinite_ray(our_king_square, source_square);
  } else {
    return Bitboard((uint64_t)-1);
  }
}

// ======================================================
// Function: Move_Generator::attackers_to_square
// Purpose : Return a bitboard of ALL pieces (any color)
//           that are currently attacking a given square.
// ======================================================
Bitboard Move_Generator::attackers_to_square(const Square& s) {
  return attackers_to_square(
      s, m_chess_board.get_piece_occupancies(PIECE_COLOR::WHITE, PIECES::PAWN),
      m_chess_board.get_piece_occupancies(PIECE_COLOR::WHITE, PIECES::KNIGHT),
      m_chess_board.get_piece_occupancies(PIECE_COLOR::WHITE, PIECES::BISHOP),
      m_chess_board.get_piece_occupancies(PIECE_COLOR::WHITE, PIECES::ROOK),
      m_chess_board.get_piece_occupancies(PIECE_COLOR::WHITE, PIECES::QUEEN),
      m_chess_board.get_piece_occupancies(PIECE_COLOR::WHITE, PIECES::KING),
      m_chess_board.get_piece_occupancies(PIECE_COLOR::BLACK, PIECES::PAWN),
      m_chess_board.get_piece_occupancies(PIECE_COLOR::BLACK, PIECES::KNIGHT),
      m_chess_board.get_piece_occupancies(PIECE_COLOR::BLACK, PIECES::BISHOP),
      m_chess_board.get_piece_occupancies(PIECE_COLOR::BLACK, PIECES::ROOK),
      m_chess_board.get_piece_occupancies(PIECE_COLOR::BLACK, PIECES::QUEEN),
      m_chess_board.get_piece_occupancies(PIECE_COLOR::BLACK, PIECES::KING));
}

// ======================================================
// Function: Move_Generator::attackers_to_square
// Purpose : Return a bitboard of ALL pieces (any color)
//           that are currently attacking a given square.
// ======================================================
Bitboard Move_Generator::attackers_to_square(
    const Square& s, const Bitboard& white_pawn_occupancy,
    const Bitboard& white_knight_occupancy,
    const Bitboard& white_bishop_occupancy,
    const Bitboard& white_rook_occupancy, const Bitboard& white_queen_occupancy,
    const Bitboard& white_king_occupancy, const Bitboard& black_pawn_occupancy,
    const Bitboard& black_knight_occupancy,
    const Bitboard& black_bishop_occupancy,
    const Bitboard& black_rook_occupancy, const Bitboard& black_queen_occupancy,
    const Bitboard& black_king_occupancy) {
  Bitboard attackers;  // Final result: union of all attackers
  Attacks
      a;  // Utility class for attack masks (knight jumps, pawn attacks, etc.)

  Bitboard both_color_occupancies =
      white_pawn_occupancy | white_knight_occupancy | white_bishop_occupancy |
      white_rook_occupancy | white_queen_occupancy | white_king_occupancy |
      black_pawn_occupancy | black_knight_occupancy | black_bishop_occupancy |
      black_rook_occupancy | black_queen_occupancy | black_king_occupancy;

  // ======================================================
  // 1. PAWN ATTACKS
  // ======================================================
  // Pawns attack diagonally forward, but the direction depends on their color.
  // To check if 's' is attacked by a WHITE pawn:
  //   - Imagine a BLACK pawn standing on 's' → get the squares *that BLACK
  //   pawn* would attack.
  //   - Intersect that with all actual WHITE pawns on the board.
  // The inversion is required because pawn attacks are asymmetric.
  attackers |=
      (a.get_pawn_attacks(s, PIECE_COLOR::WHITE) & black_pawn_occupancy);

  attackers |=
      (a.get_pawn_attacks(s, PIECE_COLOR::BLACK) & white_pawn_occupancy);

  // ======================================================
  // 2. KNIGHT ATTACKS
  // ======================================================
  // Knights are simple: their attack pattern is independent of board occupancy.
  // Generate knight moves from 's' and see if ANY white or black knight sits on
  // them.
  attackers |= (a.get_knight_attacks(s) &
                (white_knight_occupancy | black_knight_occupancy));

  // ======================================================
  // 3. BISHOP + QUEEN ATTACKS
  // ======================================================
  // Bishops slide diagonally, and queens can also move like bishops.
  // Generate all diagonal attacks from 's' (accounting for blockers with
  // occupancy), then intersect with *all bishops + queens* on the board,
  // regardless of color.
  attackers |= (a.get_bishop_attacks(s, both_color_occupancies) &
                (white_bishop_occupancy | white_queen_occupancy |
                 black_bishop_occupancy | black_queen_occupancy));

  // ======================================================
  // 4. ROOK + QUEEN ATTACKS
  // ======================================================
  // Rooks slide horizontally/vertically, and queens can also move like rooks.
  // Generate rook attacks from 's' (with occupancy), then intersect with *all
  // rooks + queens* on the board.
  attackers |= (a.get_rook_attacks(s, both_color_occupancies) &
                (white_rook_occupancy | white_queen_occupancy |
                 black_rook_occupancy | black_queen_occupancy));

  // ======================================================
  // 5. KING ATTACKS
  // ======================================================
  // Kings attack their 8 surrounding squares.
  // Generate king moves from 's' and see if either king occupies those squares.
  attackers |=
      (a.get_king_attacks(s) & (white_king_occupancy | black_king_occupancy));

  // ======================================================
  // Done! 'attackers' now contains all pieces (both sides)
  // that can legally attack square 's' under the current occupancy.
  // ======================================================
  return attackers;
}

// ======================================================================
// Function : Move_Generator::is_our_king_ring_attacked
// Purpose  : Determine which squares in the "king ring" (the 8 surrounding
//            squares around *our* king) are currently under attack
//            by the opponent.
// Returns  : Bitboard mask of attacked king-ring squares.
// ======================================================================
Bitboard Move_Generator::is_our_king_ring_attacked() {
  Bitboard mask;  // Final result: set of attacked squares around our king
  Attacks a;      // Attack pattern generator (precomputed masks + magics)

  // ------------------------------------------------------
  // Determine sides
  // ------------------------------------------------------

  // Side whose king we are checking (the side to move).
  const PIECE_COLOR our_side = m_chess_board.get_side_to_move();

  // The OPPOSING side (not our_side).
  // Trick: (~our_side) flips bits, & 0x1 makes it 0 ↔ 1.
  // Example: WHITE=0, BLACK=1 → flips nicely.
  const PIECE_COLOR opposing_side = (PIECE_COLOR)((~our_side) & 0x1);

  // ------------------------------------------------------
  // Get our king’s position
  // ------------------------------------------------------

  // Find the square that our king occupies.
  // Required because we want to look at the 8 surrounding squares.
  const Square our_king_square = m_chess_board.get_king_square(our_side);

  // Build a "king ring" bitboard: all squares a king could move to
  // if it stood on our_king_square (i.e. its 8 neighbors).
  Bitboard king_ring = a.get_king_attacks(our_king_square);

  // ------------------------------------------------------
  // Loop through each square in the king ring
  // ------------------------------------------------------

  while (king_ring.get_board()) {
    // Extract the *lowest set bit* (one square from king ring).
    // `get_index_of_high_lsb()` = index of least significant bit set.
    const Square king_ring_square = Square(king_ring.get_index_of_high_lsb());

    Bitboard black_king_occupancy = Bitboard(
        m_chess_board.get_piece_occupancies(PIECE_COLOR::BLACK, PIECES::KING)
            .get_board() *
        (our_side == PIECE_COLOR::WHITE));
    Bitboard white_king_occupancy = Bitboard(
        m_chess_board.get_piece_occupancies(PIECE_COLOR::WHITE, PIECES::KING)
            .get_board() *
        (our_side == PIECE_COLOR::BLACK));

    Bitboard king_ring_square_attackers = attackers_to_square(
        king_ring_square,
        m_chess_board.get_piece_occupancies(PIECE_COLOR::WHITE, PIECES::PAWN),
        m_chess_board.get_piece_occupancies(PIECE_COLOR::WHITE, PIECES::KNIGHT),
        m_chess_board.get_piece_occupancies(PIECE_COLOR::WHITE, PIECES::BISHOP),
        m_chess_board.get_piece_occupancies(PIECE_COLOR::WHITE, PIECES::ROOK),
        m_chess_board.get_piece_occupancies(PIECE_COLOR::WHITE, PIECES::QUEEN),
        white_king_occupancy,
        m_chess_board.get_piece_occupancies(PIECE_COLOR::BLACK, PIECES::PAWN),
        m_chess_board.get_piece_occupancies(PIECE_COLOR::BLACK, PIECES::KNIGHT),
        m_chess_board.get_piece_occupancies(PIECE_COLOR::BLACK, PIECES::BISHOP),
        m_chess_board.get_piece_occupancies(PIECE_COLOR::BLACK, PIECES::ROOK),
        m_chess_board.get_piece_occupancies(PIECE_COLOR::BLACK, PIECES::QUEEN),
        black_king_occupancy);

    // Check if that king-ring square is attacked:
    //   - attackers_to_square(square) → all pieces attacking this square
    //   - & m_chess_board.get_color_occupancies(opposing_side) → restrict to
    //   enemy side
    //   - If result != 0 → at least one enemy piece attacks this square.
    if ((king_ring_square_attackers &
         m_chess_board.get_color_occupancies(opposing_side))
            .get_board()) {
      // If attacked, set that square’s bit in our result mask.
      mask |= Bitboard(king_ring_square.get_mask());
    }

    // Remove this square from the king_ring bitboard (unset processed square).
    king_ring.unset_square(king_ring_square);
  }

  // ------------------------------------------------------
  // Done: return the mask of all attacked king-ring squares
  // ------------------------------------------------------
  return mask;
}

void Move_Generator::generate_all_moves(std::vector<Chess_Move>& output) {
  Attacks a;

  const PIECE_COLOR our_side = m_chess_board.get_side_to_move();
  const PIECE_COLOR opposing_side = (PIECE_COLOR)((~our_side) & 0x1);

  const Bitboard is_white_the_moving_side_mask =
      Bitboard((uint64_t)(-((uint64_t)(our_side == PIECE_COLOR::WHITE))));
  const Bitboard is_black_the_moving_side_mask =
      Bitboard((uint64_t)(-((uint64_t)(our_side == PIECE_COLOR::BLACK))));

  const Bitboard check_mask = generate_check_mask();
  const Bitboard pinned = generate_pinned();

  const Bitboard enemy_or_empty =
      m_chess_board.get_color_occupancies(opposing_side) |
      (~(m_chess_board.get_both_color_occupancies()));

  Bitboard our_king_occupancies =
      m_chess_board.get_piece_occupancies(our_side, PIECES::KING);

  Square our_king_square = Square(our_king_occupancies.get_index_of_high_lsb());

  // Pawn Moves + Checkmask.
  // pawns >>= 8 = + 1 rank for white.
  // pawns <<= 8 = + 1 rank for black.

  // Single Push if next rank is empty.
  // Single Push if next rank is empty & promotion.
  Bitboard w_pawns =
      m_chess_board.get_piece_occupancies(PIECE_COLOR::WHITE, PIECES::PAWN);
  Bitboard b_pawns =
      m_chess_board.get_piece_occupancies(PIECE_COLOR::BLACK, PIECES::PAWN);

  const Bitboard w_single_pawn_pushes =
      (w_pawns >> 8) & (~m_chess_board.get_both_color_occupancies());
  const Bitboard b_single_pawn_pushes =
      (b_pawns << 8) & (~m_chess_board.get_both_color_occupancies());

  Bitboard w_promotion_single_pushes =
      w_single_pawn_pushes & EIGHTH_RANK & is_white_the_moving_side_mask;
  Bitboard b_promotion_single_pushes =
      b_single_pawn_pushes & FIRST_RANK & is_black_the_moving_side_mask;

  while (w_promotion_single_pushes.get_board()) {
    ESQUARE source_square = (ESQUARE)((uint8_t)(w_promotion_single_pushes << 8)
                                          .get_index_of_high_lsb());
    ESQUARE target_square =
        (ESQUARE)((uint8_t)w_promotion_single_pushes.get_index_of_high_lsb());

    Bitboard pin_mask = get_pin_mask(pinned, source_square);

    Bitboard pawn_moves =
        Bitboard(Square(target_square).get_mask()) & check_mask & pin_mask;

    if (pawn_moves != 0) {
      generate_pawn_promotions(Square(source_square), Square(target_square),
                               output);
    }

    w_promotion_single_pushes.unset_square(Square(target_square));
  }

  while (b_promotion_single_pushes.get_board()) {
    ESQUARE source_square = (ESQUARE)((uint8_t)(b_promotion_single_pushes >> 8)
                                          .get_index_of_high_lsb());
    ESQUARE target_square =
        (ESQUARE)((uint8_t)b_promotion_single_pushes.get_index_of_high_lsb());

    Bitboard pin_mask = get_pin_mask(pinned, source_square);

    Bitboard pawn_moves =
        Bitboard(Square(target_square).get_mask()) & check_mask & pin_mask;

    if (pawn_moves != 0) {
      generate_pawn_promotions(Square(source_square), Square(target_square),
                               output);
    }

    b_promotion_single_pushes.unset_square(Square(target_square));
  }

  // Reset promotion single pushes for non-promotion single pushes logic
  w_promotion_single_pushes =
      w_single_pawn_pushes & EIGHTH_RANK & is_white_the_moving_side_mask;
  b_promotion_single_pushes =
      b_single_pawn_pushes & FIRST_RANK & is_black_the_moving_side_mask;

  Bitboard w_non_promotion_single_pushes = w_single_pawn_pushes &
                                           (~w_promotion_single_pushes) &
                                           is_white_the_moving_side_mask;
  Bitboard b_non_promotion_single_pushes = b_single_pawn_pushes &
                                           (~b_promotion_single_pushes) &
                                           is_black_the_moving_side_mask;

  while (w_non_promotion_single_pushes.get_board()) {
    ESQUARE source_square =
        (ESQUARE)((uint8_t)(w_non_promotion_single_pushes << 8)
                      .get_index_of_high_lsb());
    ESQUARE target_square =
        (ESQUARE)((uint8_t)
                      w_non_promotion_single_pushes.get_index_of_high_lsb());

    Bitboard pin_mask = get_pin_mask(pinned, source_square);

    Bitboard pawn_moves =
        Bitboard(Square(target_square).get_mask()) & check_mask & pin_mask;

    if (pawn_moves != 0) {
      Chess_Move move = {.source_square = source_square,
                         .destination_square = target_square,
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

      output.push_back(move);
    }

    w_non_promotion_single_pushes.unset_square(Square(target_square));
  }

  while (b_non_promotion_single_pushes.get_board()) {
    ESQUARE source_square =
        (ESQUARE)((uint8_t)(b_non_promotion_single_pushes >> 8)
                      .get_index_of_high_lsb());
    ESQUARE target_square =
        (ESQUARE)((uint8_t)
                      b_non_promotion_single_pushes.get_index_of_high_lsb());

    Bitboard pin_mask = get_pin_mask(pinned, source_square);

    Bitboard pawn_moves =
        Bitboard(Square(target_square).get_mask()) & check_mask & pin_mask;

    if (pawn_moves != 0) {
      Chess_Move move = {.source_square = source_square,
                         .destination_square = target_square,
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

      output.push_back(move);
    }

    b_non_promotion_single_pushes.unset_square(Square(target_square));
  }

  // Double Push if rank+2 is empty and pawn is still on 2nd or 7th rank.
  Bitboard w_double_pawn_pushes =
      (((((w_pawns & SECOND_RANK) >> 8) &
         (~m_chess_board.get_both_color_occupancies())) >>
        8) &
       (~m_chess_board.get_both_color_occupancies())) &
      is_white_the_moving_side_mask;
  Bitboard b_double_pawn_pushes =
      (((((b_pawns & SEVENTH_RANK) << 8) &
         (~m_chess_board.get_both_color_occupancies()))
        << 8) &
       (~m_chess_board.get_both_color_occupancies())) &
      is_black_the_moving_side_mask;

  while (w_double_pawn_pushes.get_board()) {
    ESQUARE source_square = (ESQUARE)((uint8_t)(w_double_pawn_pushes << 16)
                                          .get_index_of_high_lsb());
    ESQUARE target_square =
        (ESQUARE)((uint8_t)w_double_pawn_pushes.get_index_of_high_lsb());

    Bitboard pin_mask = get_pin_mask(pinned, source_square);

    Bitboard pawn_moves =
        Bitboard(Square(target_square).get_mask()) & check_mask & pin_mask;

    if (pawn_moves != 0) {
      Chess_Move move = {.source_square = source_square,
                         .destination_square = target_square,
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

      output.push_back(move);
    }

    w_double_pawn_pushes.unset_square(Square(target_square));
  }

  while (b_double_pawn_pushes.get_board()) {
    ESQUARE source_square = (ESQUARE)((uint8_t)(b_double_pawn_pushes >> 16)
                                          .get_index_of_high_lsb());
    ESQUARE target_square =
        (ESQUARE)((uint8_t)b_double_pawn_pushes.get_index_of_high_lsb());

    Bitboard pin_mask = get_pin_mask(pinned, source_square);

    Bitboard pawn_moves =
        Bitboard(Square(target_square).get_mask()) & check_mask & pin_mask;

    if (pawn_moves != 0) {
      Chess_Move move = {.source_square = source_square,
                         .destination_square = target_square,
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

      output.push_back(move);
    }

    b_double_pawn_pushes.unset_square(Square(target_square));
  }

  // En passant capture
  Square en_passant_square = m_chess_board.get_en_passant_square();
  if (en_passant_square.get_index() != ESQUARE::NO_SQUARE) {
    Bitboard en_passant_pawns =
        a.get_pawn_attacks(en_passant_square, opposing_side) &
        m_chess_board.get_piece_occupancies(our_side, PIECES::PAWN);

    while (en_passant_pawns.get_board()) {
      ESQUARE source_square = (ESQUARE)en_passant_pawns.get_index_of_high_lsb();

      Bitboard pin_mask = get_pin_mask(pinned, source_square);

      ESQUARE target_square = (ESQUARE)en_passant_square.get_index();

      Bitboard can_do_enpassant =
          Bitboard(Square(target_square).get_mask()) & check_mask & pin_mask;

      if (can_do_enpassant.get_board() != 0) {
        Chess_Move move = {
            .source_square = source_square,
            .destination_square = target_square,
            .moving_piece = PIECES::PAWN,
            .promoted_piece = PIECES::NO_PIECE,
            .captured_piece =
                PIECES::NO_PIECE,  // Yes we capture a PIECES::PAWN but this
                                   // becomes a hinderance when writing the
                                   // calculate temporal occupancy function.
            .is_capture = true,
            .is_short_castling = false,
            .is_long_castling = false,
            .castling_rook_source_square = ESQUARE::NO_SQUARE,
            .castling_rook_destination_square = ESQUARE::NO_SQUARE,
            .is_double_pawn_push = false,
            .is_en_passant = true,
            .en_passant_victim_square = ESQUARE::NO_SQUARE,
            .is_promotion = false};

        if (our_side == PIECE_COLOR::WHITE) {
          Square victim_pawn_square =
              Square(Bitboard(Square(target_square).get_mask() << 8)
                         .get_index_of_high_lsb());

          move.en_passant_victim_square =
              (ESQUARE)victim_pawn_square.get_index();
          output.push_back(move);

        } else {
          Square victim_pawn_square =
              Square(Bitboard(Square(target_square).get_mask() >> 8)
                         .get_index_of_high_lsb());

          move.en_passant_victim_square =
              (ESQUARE)victim_pawn_square.get_index();
          output.push_back(move);
        }
      }
      en_passant_pawns.unset_square(Square(source_square));
    }
  }

  w_pawns &= is_white_the_moving_side_mask;
  b_pawns &= is_black_the_moving_side_mask;

  // Captures if square is occupied by enemy & deal with promotions.
  while (w_pawns.get_board()) {
    ESQUARE source_square = (ESQUARE)((uint8_t)w_pawns.get_index_of_high_lsb());

    Bitboard pin_mask = get_pin_mask(pinned, source_square);

    Bitboard pawn_attacks =
        a.get_pawn_attacks(Square(source_square), PIECE_COLOR::WHITE) &
        m_chess_board.get_color_occupancies(PIECE_COLOR::BLACK) & check_mask &
        pin_mask;

    Bitboard promotable_pawn_attacks = pawn_attacks & EIGHTH_RANK;
    Bitboard non_promotable_pawn_attacks =
        pawn_attacks & (~promotable_pawn_attacks);

    while (promotable_pawn_attacks.get_board()) {
      ESQUARE target_square =
          (ESQUARE)((uint8_t)promotable_pawn_attacks.get_index_of_high_lsb());

      generate_pawn_promotions(Square(source_square), Square(target_square),
                               output);

      promotable_pawn_attacks.unset_square(Square(target_square));
    }

    while (non_promotable_pawn_attacks.get_board()) {
      ESQUARE target_square =
          (ESQUARE)((uint8_t)
                        non_promotable_pawn_attacks.get_index_of_high_lsb());

      const auto who_is_on_target_square =
          m_chess_board.what_piece_is_on_square(Square(target_square));

      Chess_Move move = {.source_square = source_square,
                         .destination_square = target_square,
                         .moving_piece = PIECES::PAWN,
                         .promoted_piece = PIECES::NO_PIECE,
                         .captured_piece = who_is_on_target_square.second,
                         .is_capture = true,
                         .is_short_castling = false,
                         .is_long_castling = false,
                         .castling_rook_source_square = ESQUARE::NO_SQUARE,
                         .castling_rook_destination_square = ESQUARE::NO_SQUARE,
                         .is_double_pawn_push = false,
                         .is_en_passant = false,
                         .en_passant_victim_square = ESQUARE::NO_SQUARE,
                         .is_promotion = false};

      output.push_back(move);

      non_promotable_pawn_attacks.unset_square(Square(target_square));
    }

    w_pawns.unset_square(Square(source_square));
  }

  while (b_pawns.get_board()) {
    ESQUARE source_square = (ESQUARE)((uint8_t)b_pawns.get_index_of_high_lsb());

    Bitboard pin_mask = get_pin_mask(pinned, source_square);

    Bitboard pawn_attacks =
        a.get_pawn_attacks(Square(source_square), PIECE_COLOR::BLACK) &
        m_chess_board.get_color_occupancies(PIECE_COLOR::WHITE) & check_mask &
        pin_mask;

    Bitboard promotable_pawn_attacks = pawn_attacks & FIRST_RANK;
    Bitboard non_promotable_pawn_attacks =
        pawn_attacks & (~promotable_pawn_attacks);

    while (promotable_pawn_attacks.get_board()) {
      ESQUARE target_square =
          (ESQUARE)((uint8_t)promotable_pawn_attacks.get_index_of_high_lsb());

      generate_pawn_promotions(Square(source_square), Square(target_square),
                               output);

      promotable_pawn_attacks.unset_square(Square(target_square));
    }

    while (non_promotable_pawn_attacks.get_board()) {
      ESQUARE target_square =
          (ESQUARE)((uint8_t)
                        non_promotable_pawn_attacks.get_index_of_high_lsb());

      const auto who_is_on_target_square =
          m_chess_board.what_piece_is_on_square(Square(target_square));

      Chess_Move move = {.source_square = source_square,
                         .destination_square = target_square,
                         .moving_piece = PIECES::PAWN,
                         .promoted_piece = PIECES::NO_PIECE,
                         .captured_piece = who_is_on_target_square.second,
                         .is_capture = true,
                         .is_short_castling = false,
                         .is_long_castling = false,
                         .castling_rook_source_square = ESQUARE::NO_SQUARE,
                         .castling_rook_destination_square = ESQUARE::NO_SQUARE,
                         .is_double_pawn_push = false,
                         .is_en_passant = false,
                         .en_passant_victim_square = ESQUARE::NO_SQUARE,
                         .is_promotion = false};

      output.push_back(move);

      non_promotable_pawn_attacks.unset_square(Square(target_square));
    }

    b_pawns.unset_square(Square(source_square));
  }

  generate_minor_and_major_piece_moves<PIECES::KNIGHT>(a, pinned, output);
  generate_minor_and_major_piece_moves<PIECES::BISHOP>(a, pinned, output);
  generate_minor_and_major_piece_moves<PIECES::ROOK>(a, pinned, output);
  generate_minor_and_major_piece_moves<PIECES::QUEEN>(a, pinned, output);

  // King Moves
  // Attack_Mask[King Square] & EnemyOrEmpty & ~(Square attacked by enemy)
  Bitboard king_moves = a.get_king_attacks(our_king_square) & enemy_or_empty &
                        (~is_our_king_ring_attacked());

  while (king_moves.get_board()) {
    Square target_square = Square(king_moves.get_index_of_high_lsb());

    const auto who_is_on_target_square =
        m_chess_board.what_piece_is_on_square(target_square);

    Chess_Move move = {
        .source_square = ESQUARE(our_king_square.get_index()),
        .destination_square = ESQUARE(target_square.get_index()),
        .moving_piece = PIECES::KING,
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

    king_moves.unset_square(Square(king_moves.get_index_of_high_lsb()));
  }

  // Castling Moves
  const bool are_white_short_castle_squares_empty = {
      ((~m_chess_board.get_both_color_occupancies()) &
       WHITE_SHORT_CASTLE_SQUARES) == WHITE_SHORT_CASTLE_SQUARES};
  const bool is_f1_square_safe =
      ((attackers_to_square(Square(ESQUARE::F1)) &
        m_chess_board.get_color_occupancies(PIECE_COLOR::BLACK))
           .get_board() == 0);
  const bool is_g1_square_safe =
      ((attackers_to_square(Square(ESQUARE::G1)) &
        m_chess_board.get_color_occupancies(PIECE_COLOR::BLACK))
           .get_board() == 0);
  const bool are_white_short_castle_squares_safe =
      is_f1_square_safe && is_g1_square_safe;
  const bool is_white_king_not_in_check =
      ((attackers_to_square(our_king_square) &
        m_chess_board.get_color_occupancies(PIECE_COLOR::BLACK))
           .get_board() == 0);
  const bool is_white_the_moving_side = (our_side == PIECE_COLOR::WHITE);
  if (m_chess_board.does_white_have_short_castle_rights() &&
      are_white_short_castle_squares_empty &&
      are_white_short_castle_squares_safe && is_white_king_not_in_check &&
      is_white_the_moving_side) {
    Chess_Move move = {.source_square = ESQUARE::E1,
                       .destination_square = ESQUARE::G1,
                       .moving_piece = PIECES::KING,
                       .promoted_piece = PIECES::NO_PIECE,
                       .captured_piece = PIECES::NO_PIECE,
                       .is_capture = false,
                       .is_short_castling = true,
                       .is_long_castling = false,
                       .castling_rook_source_square = ESQUARE::H1,
                       .castling_rook_destination_square = ESQUARE::F1,
                       .is_double_pawn_push = false,
                       .is_en_passant = false,
                       .en_passant_victim_square = ESQUARE::NO_SQUARE,
                       .is_promotion = false};

    output.push_back(move);
  }

  const bool are_white_long_castle_squares_empty =
      (((~m_chess_board.get_both_color_occupancies()) &
        WHITE_LONG_CASTLE_SQUARES) == WHITE_LONG_CASTLE_SQUARES);
  const bool is_c1_square_safe =
      ((attackers_to_square(Square(ESQUARE::C1)) &
        m_chess_board.get_color_occupancies(PIECE_COLOR::BLACK))
           .get_board() == 0);
  const bool is_d1_square_safe =
      ((attackers_to_square(Square(ESQUARE::D1)) &
        m_chess_board.get_color_occupancies(PIECE_COLOR::BLACK))
           .get_board() == 0);
  const bool are_white_long_castle_squares_safe =
      is_c1_square_safe && is_d1_square_safe;

  if (m_chess_board.does_white_have_long_castle_rights() &&
      are_white_long_castle_squares_empty &&
      are_white_long_castle_squares_safe && is_white_king_not_in_check &&
      is_white_the_moving_side) {
    Chess_Move move = {.source_square = ESQUARE::E1,
                       .destination_square = ESQUARE::C1,
                       .moving_piece = PIECES::KING,
                       .promoted_piece = PIECES::NO_PIECE,
                       .captured_piece = PIECES::NO_PIECE,
                       .is_capture = false,
                       .is_short_castling = false,
                       .is_long_castling = true,
                       .castling_rook_source_square = ESQUARE::A1,
                       .castling_rook_destination_square = ESQUARE::D1,
                       .is_double_pawn_push = false,
                       .is_en_passant = false,
                       .en_passant_victim_square = ESQUARE::NO_SQUARE,
                       .is_promotion = false};

    output.push_back(move);
  }

  const bool are_black_short_castle_squares_empty = {
      ((~m_chess_board.get_both_color_occupancies()) &
       BLACK_SHORT_CASTLE_SQUARES) == BLACK_SHORT_CASTLE_SQUARES};
  const bool is_f8_square_safe =
      ((attackers_to_square(Square(ESQUARE::F8)) &
        m_chess_board.get_color_occupancies(PIECE_COLOR::WHITE))
           .get_board() == 0);
  const bool is_g8_square_safe =
      ((attackers_to_square(Square(ESQUARE::G8)) &
        m_chess_board.get_color_occupancies(PIECE_COLOR::WHITE))
           .get_board() == 0);
  const bool are_black_short_castle_squares_safe =
      is_f8_square_safe && is_g8_square_safe;
  const bool is_black_king_not_in_check =
      ((attackers_to_square(our_king_square) &
        m_chess_board.get_color_occupancies(PIECE_COLOR::WHITE))
           .get_board() == 0);
  const bool is_black_the_moving_side = (our_side == PIECE_COLOR::BLACK);
  if (m_chess_board.does_black_have_short_castle_rights() &&
      are_black_short_castle_squares_empty &&
      are_black_short_castle_squares_safe && is_black_king_not_in_check &&
      is_black_the_moving_side) {
    Chess_Move move = {.source_square = ESQUARE::E8,
                       .destination_square = ESQUARE::G8,
                       .moving_piece = PIECES::KING,
                       .promoted_piece = PIECES::NO_PIECE,
                       .captured_piece = PIECES::NO_PIECE,
                       .is_capture = false,
                       .is_short_castling = true,
                       .is_long_castling = false,
                       .castling_rook_source_square = ESQUARE::H8,
                       .castling_rook_destination_square = ESQUARE::F8,
                       .is_double_pawn_push = false,
                       .is_en_passant = false,
                       .en_passant_victim_square = ESQUARE::NO_SQUARE,
                       .is_promotion = false};

    output.push_back(move);
  }

  const bool are_black_long_castle_squares_empty =
      (((~m_chess_board.get_both_color_occupancies()) &
        BLACK_LONG_CASTLE_SQUARES) == BLACK_LONG_CASTLE_SQUARES);
  const bool is_c8_square_safe =
      ((attackers_to_square(Square(ESQUARE::C8)) &
        m_chess_board.get_color_occupancies(PIECE_COLOR::WHITE))
           .get_board() == 0);
  const bool is_d8_square_safe =
      ((attackers_to_square(Square(ESQUARE::D8)) &
        m_chess_board.get_color_occupancies(PIECE_COLOR::WHITE))
           .get_board() == 0);
  const bool are_black_long_castle_squares_safe =
      is_c8_square_safe && is_d8_square_safe;

  if (m_chess_board.does_black_have_long_castle_rights() &&
      are_black_long_castle_squares_empty &&
      are_black_long_castle_squares_safe && is_black_king_not_in_check &&
      is_black_the_moving_side) {
    Chess_Move move = {.source_square = ESQUARE::E8,
                       .destination_square = ESQUARE::C8,
                       .moving_piece = PIECES::KING,
                       .promoted_piece = PIECES::NO_PIECE,
                       .captured_piece = PIECES::NO_PIECE,
                       .is_capture = false,
                       .is_short_castling = false,
                       .is_long_castling = true,
                       .castling_rook_source_square = ESQUARE::A8,
                       .castling_rook_destination_square = ESQUARE::D8,
                       .is_double_pawn_push = false,
                       .is_en_passant = false,
                       .en_passant_victim_square = ESQUARE::NO_SQUARE,
                       .is_promotion = false};

    output.push_back(move);
  }
}

void Move_Generator::generate_pawn_promotions(const Square& source_square,
                                              const Square& target_square,
                                              std::vector<Chess_Move>& output) {
  // Pawn to knight promotion
  Chess_Move move = {.source_square = (ESQUARE)source_square.get_index(),
                     .destination_square = (ESQUARE)target_square.get_index(),
                     .moving_piece = PIECES::PAWN,
                     .promoted_piece = PIECES::KNIGHT,
                     .captured_piece = PIECES::NO_PIECE,
                     .is_capture = false,
                     .is_short_castling = false,
                     .is_long_castling = false,
                     .castling_rook_source_square = ESQUARE::NO_SQUARE,
                     .castling_rook_destination_square = ESQUARE::NO_SQUARE,
                     .is_double_pawn_push = false,
                     .is_en_passant = false,
                     .en_passant_victim_square = ESQUARE::NO_SQUARE,
                     .is_promotion = true};

  output.push_back(move);

  // Pawn to bishop promotion.
  move.promoted_piece = PIECES::BISHOP;
  output.push_back(move);

  // Pawn to rook promotion.
  move.promoted_piece = PIECES::ROOK;
  output.push_back(move);

  // Pawn to queen promotion.
  move.promoted_piece = PIECES::QUEEN;
  output.push_back(move);
}