#include "leaper_attacks.hpp"

// Function that generates all possible pawn attack moves (not forward pushes!)
// for a pawn of a given color on a given square, using bitboards.
Bitboard mask_pawn_attacks(PIECE_COLOR c, const Square& s) {
  // Will store the resulting attack bitboard (set bits = attacked squares)
  Bitboard attacks;

  // Create a bitboard with a single 1-bit at the square "s" (the pawn's
  // location)
  Bitboard pawn_bb;
  pawn_bb.set_square(s);

  // === White pawn attacks ===
  if (c == PIECE_COLOR::WHITE) {
    // For white pawns, attacks are diagonally forward-left and forward-right.
    // Moving diagonally forward-right is equivalent to shifting the pawn's bit
    // 7 positions right (since board is 8 wide, right shift simulates moving
    // north-east).
    const Bitboard RIGHT_ATTACK = (pawn_bb >> 7);

    // Moving diagonally forward-left is equivalent to shifting the pawn's bit
    // 9 positions right (north-west direction).
    const Bitboard LEFT_ATTACK = (pawn_bb >> 9);

    // Prevent wrap-around: only add RIGHT_ATTACK if attack is not on A-file
    // (otherwise shifting would "wrap" across the board).
    if ((RIGHT_ATTACK & NOT_A_FILE).get_board()) {
      attacks |= RIGHT_ATTACK;
    }

    // Prevent wrap-around: only add LEFT_ATTACK if attack is not on H-file.
    if ((LEFT_ATTACK & NOT_H_FILE).get_board()) {
      attacks |= LEFT_ATTACK;
    }

  }
  // === Black pawn attacks ===
  else {
    // For black pawns, the board is mirrored vertically.
    // Their diagonals go "down-left" and "down-right".
    // That corresponds to left shifts instead of right shifts.
    // Similar logic follows:

    const Bitboard LEFT_ATTACK = (pawn_bb << (NUM_OF_RANKS_ON_CHESS_BOARD - 1));
    const Bitboard RIGHT_ATTACK =
        (pawn_bb << (NUM_OF_RANKS_ON_CHESS_BOARD + 1));

    if ((LEFT_ATTACK & NOT_H_FILE).get_board()) {
      attacks |= LEFT_ATTACK;
    }

    if ((RIGHT_ATTACK & NOT_A_FILE).get_board()) {
      attacks |= RIGHT_ATTACK;
    }
  }

  // Return the combined bitboard of all squares attacked by the pawn
  return attacks;
};

// Function that generates all possible knight attack moves for a knight on a
// given square, using bitboards.
Bitboard mask_knight_attacks(const Square& s) {
  // Will store the resulting attack bitboard (set bits = attacked squares)
  Bitboard attacks;

  // Create a bitboard with a single 1-bit at the square "s" (the knight's
  // location)
  Bitboard knight_bb;
  knight_bb.set_square(s);

  // -----------------------
  // NORTHWARD MOVES
  // -----------------------

  // NNW: two steps north (up the board) and one step west
  // Bitshift right by 17 moves the knight this way in bitboard indexing
  const Bitboard NNW_ATTACK = (knight_bb >> 17);

  // Mask out illegal wrap-around (can't be on file H)
  if ((NNW_ATTACK & NOT_H_FILE).get_board()) {
    attacks |= NNW_ATTACK;  // Add to the attack map
  }

  // NNE: two steps north and one step east
  // Shift right by 15 for this knight jump
  const Bitboard NNE_ATTACK = (knight_bb >> 15);

  // Must not start from file A (otherwise it wraps around left edge)
  if ((NNE_ATTACK & NOT_A_FILE).get_board()) {
    attacks |= NNE_ATTACK;
  }

  // NWW: one north, two west (left)
  const Bitboard NWW_ATTACK = (knight_bb >> 10);

  // Exclude starting on files G/H (otherwise it "wraps" past file A)
  if ((NWW_ATTACK & NOT_GH_FILES).get_board()) {
    attacks |= NWW_ATTACK;
  }

  // NEE: one north, two east (right)
  const Bitboard NEE_ATTACK = (knight_bb >> 6);

  // Exclude starting on files A/B (can't go 2 east from there)
  if ((NEE_ATTACK & NOT_AB_FILES).get_board()) {
    attacks |= NEE_ATTACK;
  }

  // -----------------------
  // SOUTHWARD MOVES
  // -----------------------

  // SSE: two steps south (down the board) and one east
  // Bitshift left moves pieces down in this representation
  const Bitboard SSE_ATTACK = (knight_bb << 17);

  // Exclude if starting from file A (can't go east from the edge)
  if ((SSE_ATTACK & NOT_A_FILE).get_board()) {
    attacks |= SSE_ATTACK;
  }

  // SSW: two steps south, one west
  const Bitboard SSW_ATTACK = (knight_bb << 15);

  // Exclude if starting from file H (can't go west from the edge)
  if ((SSW_ATTACK & NOT_H_FILE).get_board()) {
    attacks |= SSW_ATTACK;
  }

  // SEE: one south, two east
  const Bitboard SEE_ATTACK = (knight_bb << 10);

  // Exclude if on files A/B (too far left to move two east)
  if ((SEE_ATTACK & NOT_AB_FILES).get_board()) {
    attacks |= SEE_ATTACK;
  }

  // SWW: one south, two west
  const Bitboard SWW_ATTACK = (knight_bb << 6);

  // Exclude if on files G/H (too far right to move two west)
  if ((SWW_ATTACK & NOT_GH_FILES).get_board()) {
    attacks |= SWW_ATTACK;
  }

  // Return the combined bitboard of all squares attacked by the knight
  return attacks;
}

// Function that generates all possible king attack moves for a king on a given
// square, using bitboards.
Bitboard mask_king_attacks(const Square& s) {
  // Will store the resulting attack bitboard
  // Each "1" in this bitboard marks a square the king can attack
  Bitboard attacks;

  // Create a bitboard with a single 1-bit at the square "s"
  // This represents the king's position
  Bitboard king_bb;
  king_bb.set_square(s);

  // -------------------------------
  // King movement = 1 square in any direction
  // We simulate this by shifting the king's bitboard
  // and masking out illegal wrap-arounds across files
  // -------------------------------

  // North (up one rank)
  const Bitboard N_ATTACK = (king_bb >> 8);
  if (N_ATTACK.get_board()) {
    attacks |= N_ATTACK;
  }

  // North-West (up one rank, left one file)
  // Must ensure it doesn't wrap from file A → H
  const Bitboard NW_ATTACK = (king_bb >> 9);
  if ((NW_ATTACK & NOT_H_FILE).get_board()) {
    attacks |= NW_ATTACK;
  }

  // North-East (up one rank, right one file)
  // Must ensure it doesn't wrap from file H → A
  const Bitboard NE_ATTACK = (king_bb >> 7);
  if ((NE_ATTACK & NOT_A_FILE).get_board()) {
    attacks |= NE_ATTACK;
  }

  // West (left one file)
  // Prevent wrap-around across the H-file
  const Bitboard W_ATTACK = (king_bb >> 1);
  if ((W_ATTACK & NOT_H_FILE).get_board()) {
    attacks |= W_ATTACK;
  }

  // South (down one rank)
  const Bitboard S_ATTACK = (king_bb << 8);
  if (S_ATTACK.get_board()) {
    attacks |= S_ATTACK;
  }

  // South-East (down one rank, right one file)
  // Prevent wrap-around across file A
  const Bitboard SE_ATTACK = (king_bb << 9);
  if ((SE_ATTACK & NOT_A_FILE).get_board()) {
    attacks |= SE_ATTACK;
  }

  // South-West (down one rank, left one file)
  // Prevent wrap-around across file H
  const Bitboard SW_ATTACK = (king_bb << 7);
  if ((SW_ATTACK & NOT_H_FILE).get_board()) {
    attacks |= SW_ATTACK;
  }

  // East (right one file)
  // Prevent wrap-around across file A
  const Bitboard E_ATTACK = (king_bb << 1);
  if ((E_ATTACK & NOT_A_FILE).get_board()) {
    attacks |= E_ATTACK;
  }

  // Return the combined bitboard of all squares attacked by the king
  return attacks;
}
