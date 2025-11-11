#include "move_generator.hpp"

#include <iostream>

const Bitboard FIRST_RANK = Bitboard(18374686479671623680ULL);
const Bitboard SECOND_RANK = Bitboard(71776119061217280ULL);
const Bitboard SEVENTH_RANK = Bitboard(65280ULL);
const Bitboard EIGHTH_RANK = Bitboard(255ULL);

const std::array<std::array<Square, NUM_OF_CASTLING_TYPES>, NUM_OF_PLAYERS>
    CASTLING_KING_DESTINATION_SQUARES = {
        {{Square(ESQUARE::G1), Square(ESQUARE::C1)},
         {Square(ESQUARE::G8), Square(ESQUARE::C8)}}};

const std::array<std::array<Square, NUM_OF_CASTLING_TYPES>, NUM_OF_PLAYERS>
    CASTLING_ROOK_DESTINATION_SQUARES = {
        {{Square(ESQUARE::F1), Square(ESQUARE::D1)},
         {Square(ESQUARE::F8), Square(ESQUARE::D8)}}};

Move_Generator::Move_Generator(const Chess_Board& cb)
    : m_chess_board(cb),
      m_enpassantable_checker(false),
      m_side_to_move_in_check(false) {}

// Function: generate_check_mask
// Purpose: Creates a "check mask" bitboard used during move generation.
//          - If no check: all moves are legal (mask = all 1's).
//          - If single check: only moves that block or capture the checker are
//          legal.
//          - If double check: only king moves are legal (mask = empty).
Bitboard Move_Generator::generate_check_mask() {
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
  } else {
    // Checkers is not empty, side to move is in check.
    m_side_to_move_in_check = true;
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
  //   (1) Capture the checking piece directly, OR
  //   (2) Block the attack ray (only possible against sliding checkers:
  //       bishop/rook/queen).
  //
  // Special case: if the checker is a pawn that just advanced two squares and
  // is en-passant capturable, an en passant capture could neutralize the check.
  // We do NOT add the en passant target square to the generic mask here,
  // because that would incorrectly allow other pieces to "move" there.
  // Instead, we record this condition in `m_enpassantable_checker`, and the
  // pawn move generator will allow the en passant capture specifically.
  const Square checker_square = Square(checkers.get_index_of_high_lsb());

  if (m_chess_board.get_en_passant_square().get_index() != ESQUARE::NO_SQUARE) {
    if (m_chess_board.get_en_passant_victim_square() == checker_square) {
      m_enpassantable_checker = true;
    }
  }

  return (Bitboard(checkers.get_between_squares_mask(our_king_square,
                                                     checker_square)) |
          checkers);
}

bool Move_Generator::is_side_to_move_in_check() {
  generate_check_mask();
  return m_side_to_move_in_check;
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

bool Move_Generator::is_pinned(const Bitboard& pinned,
                               const Square& source_square) const {
  return ((pinned & Bitboard(source_square.get_mask())).get_board() != 0);
}

Bitboard Move_Generator::get_pin_mask(const Bitboard& pinned,
                                      const Square& source_square) const {
  // Get which side is moving
  const PIECE_COLOR our_side = m_chess_board.get_side_to_move();

  const Square our_king_square = m_chess_board.get_king_square(our_side);

  if (is_pinned(pinned, source_square)) {
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
