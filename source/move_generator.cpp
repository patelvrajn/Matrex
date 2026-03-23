#include "move_generator.hpp"

#include <algorithm>
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

Moves_Bitboard_Matrix::Moves_Bitboard_Matrix()
    : m_piece_index_masks{}, m_matrix{} {
  std::fill(&m_max_indices[0], &m_max_indices[0] + sizeof(m_max_indices),
            int8_t(-1));

  std::fill(&m_index_mappings[0][0][0],
            &m_index_mappings[0][0][0] + sizeof(m_index_mappings), int8_t(-1));
}

void Moves_Bitboard_Matrix::set_move(PIECE_COLOR color, PIECES piece,
                                     Square piece_square, Square move_square) {
  // Index mappings - given a piece and color and the square the piece is on,
  // find the index of a moves bitboard in the bitboard matrix.
  int8_t& index = m_index_mappings[color][piece][piece_square.get_index()];

  if (index == -1) {
    m_max_indices[color]++;        // Increment max index for this color.
    index = m_max_indices[color];  // Assign new max index to index mappings.
    // Update piece index mask - set the bit corresponding to this index - each
    // piece has a bitmask representing which indices in the matrix correspond
    // to it.
    m_piece_index_masks[color][piece] |= (1 << index);
    // Zero-initialize bitboard in the matrix.
    m_matrix[color][index] = {piece, piece_square, Bitboard(0)};
  }

  m_matrix[color][index].bitboard.set_square(move_square);
}

bool Moves_Bitboard_Matrix::get_moves_bitboards(PIECE_COLOR color, PIECES piece,
                                                Square piece_square,
                                                Moves_Bitboard& output) const {
  const int8_t index = m_index_mappings[color][piece][piece_square.get_index()];
  if (index == -1) {
    return false;
  }
  output = m_matrix[color][index];
  return true;
}

bool Moves_Bitboard_Matrix::get_piece_moves_bitboards(
    PIECE_COLOR color, PIECES piece,
    std::vector<Moves_Bitboard>& output) const {
  uint16_t piece_index_mask = m_piece_index_masks[color][piece];

  // No moves exist for that piece if the piece for that color exists.
  if (piece_index_mask == 0) {
    return false;
  }

  // Loop over the bits in the index mask of that piece to get indices of the
  // relevant Move Bitboards then push the indexed Move Bitboards to output.
  while (piece_index_mask) {
    const uint8_t index = __builtin_ctzll(piece_index_mask);
    output.push_back(m_matrix[color][index]);
    piece_index_mask &= (piece_index_mask - 1);
  }

  return true;
}

// Called externally to check if the side to move is in check. Never done
// internally to check if a specific side is in check so we may assume
// m_chess_board.get_side_to_move() is sufficient.
bool Move_Generator::is_side_to_move_in_check() {
  const PIECE_COLOR moving_side = m_chess_board.get_side_to_move();
  if (moving_side == PIECE_COLOR::WHITE) {
    generate_check_mask<PIECE_COLOR::WHITE>();
  } else {
    generate_check_mask<PIECE_COLOR::BLACK>();
  }
  return m_side_to_move_in_check;
}

bool Move_Generator::is_pinned(const Bitboard& pinned,
                               const Square& source_square) const {
  return ((pinned & Bitboard(source_square.get_mask())).get_board() != 0);
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
