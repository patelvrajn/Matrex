#include "chess_board.hpp"

#include <iostream>

Chess_Board::Chess_Board() {
  m_state.castling_rights =
      CASTLING_RIGHTS_FLAGS::W_KINGSIDE | CASTLING_RIGHTS_FLAGS::W_QUEENSIDE |
      CASTLING_RIGHTS_FLAGS::B_KINGSIDE | CASTLING_RIGHTS_FLAGS::B_QUEENSIDE;
  m_state.enpassant_square = ESQUARE::NO_SQUARE;
  m_state.side_to_move = PIECE_COLOR::WHITE;
  m_state.half_move_clock = 0;
  m_state.full_move_count = 0;
  m_piece_bitboards = {};
  m_color_occupancy_bitboards = {};
  m_zobrist_hash = Zobrist_Hash();
  m_zobrist_hash.update_castling_rights(m_state.castling_rights);
}

Bitboard Chess_Board::get_both_color_occupancies() const {
  return (m_color_occupancy_bitboards[PIECE_COLOR::WHITE] |
          m_color_occupancy_bitboards[PIECE_COLOR::BLACK]);
}

Bitboard Chess_Board::get_color_occupancies(PIECE_COLOR c) const {
  return m_color_occupancy_bitboards[c];
}

Bitboard Chess_Board::get_piece_occupancies(PIECE_COLOR c, PIECES p) const {
  return m_piece_bitboards[c][p];
}

Square Chess_Board::get_en_passant_square() const {
  return Square(m_state.enpassant_square);
}

Square Chess_Board::get_en_passant_victim_square() const {
  return (m_state.side_to_move == PIECE_COLOR::WHITE)
             ? Square(Bitboard(Square(m_state.enpassant_square).get_mask() << 8)
                          .get_index_of_high_lsb())
             : Square(Bitboard(Square(m_state.enpassant_square).get_mask() >> 8)
                          .get_index_of_high_lsb());
}

PIECE_COLOR Chess_Board::get_side_to_move() const {
  return m_state.side_to_move;
}

Square Chess_Board::get_king_square(PIECE_COLOR c) const {
  return Square(m_piece_bitboards[c][PIECES::KING].get_index_of_high_lsb());
}

bool Chess_Board::does_white_have_short_castle_rights() const {
  return m_state.castling_rights & CASTLING_RIGHTS_FLAGS::W_KINGSIDE;
}

bool Chess_Board::does_white_have_long_castle_rights() const {
  return m_state.castling_rights & CASTLING_RIGHTS_FLAGS::W_QUEENSIDE;
}

bool Chess_Board::does_black_have_short_castle_rights() const {
  return m_state.castling_rights & CASTLING_RIGHTS_FLAGS::B_KINGSIDE;
}

bool Chess_Board::does_black_have_long_castle_rights() const {
  return m_state.castling_rights & CASTLING_RIGHTS_FLAGS::B_QUEENSIDE;
}

Square Chess_Board::get_castling_rook_source_square(
    PIECE_COLOR color, CASTLING_TYPE castle_type) const {
  if (castle_type == CASTLING_TYPE::KINGSIDE) {
    return m_state.castling_rooks[color].kingside;
  } else {
    return m_state.castling_rooks[color].queenside;
  }
}

void Chess_Board::pretty_print() const {
  for (uint8_t rank = 0; rank < NUM_OF_RANKS_ON_CHESS_BOARD; rank++) {
    // Print the ranks on the left hand side of the board before the first file.
    std::cout << (NUM_OF_RANKS_ON_CHESS_BOARD - rank) << "   ";

    for (uint8_t file = 0; file < NUM_OF_FILES_ON_CHESS_BOARD; file++) {
      Square s(rank, file);

      auto piece = what_piece_is_on_square(s);

      if (piece.second != NO_PIECE) {
        uint8_t unicode_idx = 0;

        if (piece.first == PIECE_COLOR::BLACK) {
          unicode_idx = NUM_OF_UNIQUE_PIECES_PER_PLAYER;
        }

        unicode_idx += piece.second;

        std::cout << UNICODE_PIECES[unicode_idx] << " ";

      } else {
        std::cout << "▢" << " ";
      }
    }

    std::cout << std::endl;
  }

  std::cout << std::endl;
  std::cout << "    ";

  for (uint8_t file = 0; file < NUM_OF_FILES_ON_CHESS_BOARD; file++) {
    char file_char = 'A' + file;
    std::cout << file_char << " ";
  }

  std::cout << std::endl;

  std::cout << "Side To Move: ";
  if (m_state.side_to_move == PIECE_COLOR::WHITE) {
    std::cout << "White";
  }
  if (m_state.side_to_move == PIECE_COLOR::BLACK) {
    std::cout << "Black";
  }
  if (m_state.side_to_move == PIECE_COLOR::NO_COLOR) {
    std::cout << "NONE";
  }
  std::cout << std::endl;

  std::cout << "Enpassant Square: " << SQUARE_STRINGS[m_state.enpassant_square]
            << std::endl;

  std::cout << "Castling Rights: ";
  if (m_state.castling_rights == 0) {
    std::cout << "NONE";
  } else {
    if (m_state.castling_rights | CASTLING_RIGHTS_FLAGS::W_KINGSIDE) {
      std::cout << "K";
    }
    if (m_state.castling_rights | CASTLING_RIGHTS_FLAGS::W_QUEENSIDE) {
      std::cout << "Q";
    }
    if (m_state.castling_rights | CASTLING_RIGHTS_FLAGS::B_KINGSIDE) {
      std::cout << "k";
    }
    if (m_state.castling_rights | CASTLING_RIGHTS_FLAGS::B_QUEENSIDE) {
      std::cout << "q";
    }
  }

  std::cout << std::endl;

  std::cout << "Half-move clock: " << (uint64_t)m_state.half_move_clock
            << std::endl;

  std::cout << "Full move count: " << m_state.full_move_count << std::endl;

  std::cout << "Zobrist Hash: " << m_zobrist_hash.get_hash_value() << std::endl;

  std::cout << std::endl;
}

std::pair<PIECE_COLOR, PIECES> Chess_Board::what_piece_is_on_square(
    const Square& s) const {
  const Bitboard square_bb = Bitboard(s.get_mask());

  for (uint8_t color = 0; color <= PIECE_COLOR::BLACK; color++) {
    for (uint8_t piece = 0; piece <= PIECES::KING; piece++) {
      if ((get_piece_occupancies((PIECE_COLOR)color, (PIECES)piece) &
           square_bb) != 0) {
        return {(PIECE_COLOR)color, (PIECES)piece};
      }
    }
  }

  return {PIECE_COLOR::NO_COLOR, PIECES::NO_PIECE};
}

Undo_Chess_Move Chess_Board::make_move(const Chess_Move& move) {
  Undo_Chess_Move undo_move = {.move = move,
                               .castling_rights = m_state.castling_rights,
                               .half_move_clock = m_state.half_move_clock,
                               .enpassant_square = m_state.enpassant_square};

  calculate_next_board_state(m_state.side_to_move, move);

  if ((move.moving_piece == PIECES::PAWN) || (move.is_capture)) {
    m_state.half_move_clock = 0;
  } else {
    m_state.half_move_clock = m_state.half_move_clock + 1;
  }

  if (m_state.side_to_move == PIECE_COLOR::BLACK) {
    m_state.full_move_count = m_state.full_move_count + 1;
  }

  const ESQUARE previous_enpassant_square = m_state.enpassant_square;
  if (move.is_double_pawn_push) {
    if (m_state.side_to_move == PIECE_COLOR::WHITE) {
      m_state.enpassant_square = ESQUARE(move.destination_square + 8);
    } else {
      m_state.enpassant_square = ESQUARE(move.destination_square - 8);
    }
    if (previous_enpassant_square != ESQUARE::NO_SQUARE) {
      m_zobrist_hash.update_en_passant_square(
          Square(previous_enpassant_square));
    }
    m_zobrist_hash.update_en_passant_square(Square(m_state.enpassant_square));
  } else {
    m_state.enpassant_square = ESQUARE::NO_SQUARE;
  }

  const uint8_t previous_castling_rights = m_state.castling_rights;

  // If either side castles or moves their king, the respective side loses all
  // castling rights.
  if ((move.is_short_castling || move.is_long_castling) ||
      (move.moving_piece == PIECES::KING)) {
    if (m_state.side_to_move == PIECE_COLOR::WHITE) {
      m_state.castling_rights &= ~(CASTLING_RIGHTS_FLAGS::W_KINGSIDE |
                                   CASTLING_RIGHTS_FLAGS::W_QUEENSIDE);
    } else {
      m_state.castling_rights &= ~(CASTLING_RIGHTS_FLAGS::B_KINGSIDE |
                                   CASTLING_RIGHTS_FLAGS::B_QUEENSIDE);
    }
  }

  const ESQUARE white_queenside_rook_square =
      (ESQUARE)get_castling_rook_source_square(PIECE_COLOR::WHITE,
                                               CASTLING_TYPE::QUEENSIDE)
          .get_index();
  const ESQUARE white_kingside_rook_square =
      (ESQUARE)get_castling_rook_source_square(PIECE_COLOR::WHITE,
                                               CASTLING_TYPE::KINGSIDE)
          .get_index();
  const ESQUARE black_queenside_rook_square =
      (ESQUARE)get_castling_rook_source_square(PIECE_COLOR::BLACK,
                                               CASTLING_TYPE::QUEENSIDE)
          .get_index();
  const ESQUARE black_kingside_rook_square =
      (ESQUARE)get_castling_rook_source_square(PIECE_COLOR::BLACK,
                                               CASTLING_TYPE::KINGSIDE)
          .get_index();

  // If white moves their queenside rook or it is captured by black, white loses
  // queenside castling rights.
  if (((m_state.side_to_move == PIECE_COLOR::WHITE) &&
       (move.moving_piece == PIECES::ROOK) &&
       (move.source_square == white_queenside_rook_square)) ||
      ((m_state.side_to_move == PIECE_COLOR::BLACK) &&
       (move.captured_piece == PIECES::ROOK) &&
       (move.destination_square == white_queenside_rook_square))) {
    m_state.castling_rights &= ~(CASTLING_RIGHTS_FLAGS::W_QUEENSIDE);
  }

  // If white moves their kingside rook or it is captured by black, white loses
  // kingside castling rights.
  if (((m_state.side_to_move == PIECE_COLOR::WHITE) &&
       (move.moving_piece == PIECES::ROOK) &&
       (move.source_square == white_kingside_rook_square)) ||
      ((m_state.side_to_move == PIECE_COLOR::BLACK) &&
       (move.captured_piece == PIECES::ROOK) &&
       (move.destination_square == white_kingside_rook_square))) {
    m_state.castling_rights &= ~(CASTLING_RIGHTS_FLAGS::W_KINGSIDE);
  }

  // If black moves their queenside rook or it is captured by white, black loses
  // queenside castling rights.
  if (((m_state.side_to_move == PIECE_COLOR::BLACK) &&
       (move.moving_piece == PIECES::ROOK) &&
       (move.source_square == black_queenside_rook_square)) ||
      ((m_state.side_to_move == PIECE_COLOR::WHITE) &&
       (move.captured_piece == PIECES::ROOK) &&
       (move.destination_square == black_queenside_rook_square))) {
    m_state.castling_rights &= ~(CASTLING_RIGHTS_FLAGS::B_QUEENSIDE);
  }

  // If black moves their kingside rook or it is captured by white, black loses
  // kingside castling rights.
  if (((m_state.side_to_move == PIECE_COLOR::BLACK) &&
       (move.moving_piece == PIECES::ROOK) &&
       (move.source_square == black_kingside_rook_square)) ||
      ((m_state.side_to_move == PIECE_COLOR::WHITE) &&
       (move.captured_piece == PIECES::ROOK) &&
       (move.destination_square == black_kingside_rook_square))) {
    m_state.castling_rights &= ~(CASTLING_RIGHTS_FLAGS::B_KINGSIDE);
  }

  m_zobrist_hash.update_castling_rights(previous_castling_rights);
  m_zobrist_hash.update_castling_rights(m_state.castling_rights);

  m_state.side_to_move = (PIECE_COLOR)((~m_state.side_to_move) & 0x1);
  m_zobrist_hash.flip_side_to_move();

  return undo_move;
}

void Chess_Board::undo_move(Undo_Chess_Move undo_move) {
  m_zobrist_hash.update_castling_rights(m_state.castling_rights);
  if (m_state.enpassant_square != ESQUARE::NO_SQUARE) {
    m_zobrist_hash.update_en_passant_square(Square(m_state.enpassant_square));
  }

  m_state.castling_rights = undo_move.castling_rights;
  m_state.half_move_clock = undo_move.half_move_clock;
  m_state.enpassant_square = undo_move.enpassant_square;
  PIECE_COLOR opposing_side = (PIECE_COLOR)((~m_state.side_to_move) & 0x1);

  m_zobrist_hash.update_castling_rights(m_state.castling_rights);
  if (m_state.enpassant_square != ESQUARE::NO_SQUARE) {
    m_zobrist_hash.update_en_passant_square(Square(m_state.enpassant_square));
  }

  calculate_next_board_state(opposing_side, undo_move.move);

  if (opposing_side == PIECE_COLOR::BLACK) {
    m_state.full_move_count = m_state.full_move_count - 1;
  }

  m_state.side_to_move = opposing_side;
  m_zobrist_hash.flip_side_to_move();
}

Zobrist_Hash Chess_Board::get_zobrist_hash() const { return m_zobrist_hash; }

void Chess_Board::set_from_fen(const std::string& fen) {
  // Nuke the board: wipe every piece and occupancy bitboard like the
  // apocalypse.
  m_piece_bitboards = {};
  m_color_occupancy_bitboards = {};

  m_zobrist_hash = Zobrist_Hash();

  // Extract ONLY the piece placement section (before the first space).
  // FEN format reminder: "<pieces> <side> <castling> <enpassant> <halfmove>
  // <fullmove>"
  const std::string piece_placement = fen.substr(0, fen.find_first_of(' '));
  const uint8_t piece_placement_len = piece_placement.length();

  // Index trackers: idx = position in piece_placement string
  // current_rank = rank we’re parsing
  // start_of_substr = where this rank description starts in the FEN substring
  uint8_t idx = 0;
  uint8_t current_rank = 0;
  uint8_t start_of_substr = 0;

  // Walk through the piece placement string rank by rank, separated by "/"
  while (idx < piece_placement_len) {
    bool is_last_char = (idx == (piece_placement_len - 1));

    // If we hit a "/" (end of a rank) OR we’ve reached the very last char,
    // parse a rank
    if ((piece_placement[idx] == '/') || is_last_char) {
      uint8_t end_of_substr = idx - 1;
      if (is_last_char) {
        end_of_substr = idx;  // include last character
      }
      const uint8_t len_of_substr = (end_of_substr - start_of_substr + 1);

      // Parse the rank into actual bitboards
      place_pieces_from_fen(
          piece_placement.substr(start_of_substr, len_of_substr), len_of_substr,
          current_rank);

      // Prepare to start next rank
      start_of_substr = idx + 1;
      current_rank++;
    }

    idx++;
  }

  // Extract side to move ("w" or "b") from right after the piece placement
  // section
  const std::string side_to_move = fen.substr(piece_placement.length() + 1, 1);

  // Translate char into enum
  switch (side_to_move[0]) {
    case 'w':
      m_state.side_to_move = PIECE_COLOR::WHITE;
      break;
    case 'b':
      m_state.side_to_move = PIECE_COLOR::BLACK;
      m_zobrist_hash.flip_side_to_move();
      break;
  }

  // Calculate where castling rights start (skip piece placement + space + "w "
  // or "b ")
  const uint8_t castling_rights_start_pos = piece_placement.length() + 1 + 2;
  const uint8_t castling_rights_length =
      (fen.find_first_of(' ', castling_rights_start_pos) -
       castling_rights_start_pos);
  const std::string castling_rights =
      fen.substr(castling_rights_start_pos, castling_rights_length);

  // Reset castling rights bitmask
  m_state.castling_rights = 0;

  // Loop through castling characters (KQkq), set appropriate flags
  for (uint8_t idx = 0; idx < castling_rights_length; idx++) {
    if ((castling_rights[idx] >= 'A') && (castling_rights[idx] <= 'H')) {
      const uint8_t rook_file = castling_rights[idx] - 'A';

      if (rook_file < get_king_square(PIECE_COLOR::WHITE).get_file()) {
        m_state.castling_rights |= CASTLING_RIGHTS_FLAGS::W_QUEENSIDE;
        m_state.castling_rooks[PIECE_COLOR::WHITE].queenside =
            Square((NUM_OF_RANKS_ON_CHESS_BOARD - 1), rook_file);
      } else {
        m_state.castling_rights |= CASTLING_RIGHTS_FLAGS::W_KINGSIDE;
        m_state.castling_rooks[PIECE_COLOR::WHITE].kingside =
            Square((NUM_OF_RANKS_ON_CHESS_BOARD - 1), rook_file);
      }

    } else if ((castling_rights[idx] >= 'a') && (castling_rights[idx] <= 'h')) {
      const uint8_t rook_file = castling_rights[idx] - 'a';

      if (rook_file < get_king_square(PIECE_COLOR::BLACK).get_file()) {
        m_state.castling_rights |= CASTLING_RIGHTS_FLAGS::B_QUEENSIDE;
        m_state.castling_rooks[PIECE_COLOR::BLACK].queenside =
            Square(0, rook_file);
      } else {
        m_state.castling_rights |= CASTLING_RIGHTS_FLAGS::B_KINGSIDE;
        m_state.castling_rooks[PIECE_COLOR::BLACK].kingside =
            Square(0, rook_file);
      }

    } else {
      switch (castling_rights[idx]) {
        case 'K':
          m_state.castling_rights |= CASTLING_RIGHTS_FLAGS::W_KINGSIDE;
          m_state.castling_rooks[PIECE_COLOR::WHITE].kingside =
              Square(ESQUARE::H1);
          break;
        case 'Q':
          m_state.castling_rights |= CASTLING_RIGHTS_FLAGS::W_QUEENSIDE;
          m_state.castling_rooks[PIECE_COLOR::WHITE].queenside =
              Square(ESQUARE::A1);
          break;
        case 'k':
          m_state.castling_rights |= CASTLING_RIGHTS_FLAGS::B_KINGSIDE;
          m_state.castling_rooks[PIECE_COLOR::BLACK].kingside =
              Square(ESQUARE::H8);
          break;
        case 'q':
          m_state.castling_rights |= CASTLING_RIGHTS_FLAGS::B_QUEENSIDE;
          m_state.castling_rooks[PIECE_COLOR::BLACK].queenside =
              Square(ESQUARE::A8);
          break;
      }
    }
  }

  m_zobrist_hash.update_castling_rights(m_state.castling_rights);

  // En passant square parsing
  // Find the substring that represents en passant target square ("-" or
  // file+rank)
  const uint8_t enpassant_square_start_pos =
      castling_rights_start_pos + castling_rights_length + 1;
  const uint8_t enpassant_square_start_length =
      (fen.find_first_of(' ', enpassant_square_start_pos) -
       enpassant_square_start_pos);
  const std::string enpassant_square =
      fen.substr(enpassant_square_start_pos, enpassant_square_start_length);

  if (enpassant_square[0] != '-') {
    // Convert file letter ('a'–'h') to 0–7
    const uint8_t file = enpassant_square[0] - 'a';
    // Convert rank number ('1'–'8') to engine rank index (careful: flipped!)
    const uint8_t rank =
        NUM_OF_RANKS_ON_CHESS_BOARD - (enpassant_square[1] - '0');
    const Square s(rank, file);
    m_state.enpassant_square = (ESQUARE)s.get_index();
    m_zobrist_hash.update_en_passant_square(m_state.enpassant_square);
  } else {
    // "-" means no en passant available
    m_state.enpassant_square = ESQUARE::NO_SQUARE;
  }

  // Halfmove clock parsing (for 50-move rule)
  const uint8_t half_move_clock_start_pos =
      enpassant_square_start_pos + enpassant_square_start_length + 1;
  const uint8_t half_move_clock_start_length =
      (fen.find_first_of(' ', half_move_clock_start_pos) -
       half_move_clock_start_pos);
  const std::string half_move_clock =
      fen.substr(half_move_clock_start_pos, half_move_clock_start_length);

  // Convert string to int, store
  m_state.half_move_clock = std::stoi(half_move_clock);

  // Fullmove count parsing (how many complete turns have passed)
  const uint8_t last_space_in_fen = fen.find_last_of(' ');
  const std::string full_move_count = fen.substr(
      last_space_in_fen + 1, ((fen.length() - 1) - last_space_in_fen));

  // Convert to unsigned long long (because we apparently expect games lasting
  // until the heat death of the universe)
  m_state.full_move_count = std::stoull(full_move_count);
}

// Helper: take a substring describing a single rank (e.g. "rnbqkbnr" or
// "p3pppp") and place the corresponding pieces into bitboards
void Chess_Board::place_pieces_from_fen(const std::string& rank_description,
                                        uint8_t length_of_description,
                                        uint8_t rank) {
  uint8_t file = 0;  // Track file (0 = 'a')

  for (uint8_t idx = 0; idx < length_of_description; idx++) {
    const Square s(rank, file);

    if (std::isdigit(rank_description[idx])) {
      // Digit = number of consecutive empty squares
      // Move file forward by that many empties
      file += std::stoi(std::string(1, rank_description[idx]));
    } else {
      // It’s a piece. Determine color by case (lower = black, upper = white)
      if (std::islower(rank_description[idx])) {
        // Mark square occupied by black
        m_color_occupancy_bitboards[PIECE_COLOR::BLACK].set_square(s);
        switch (rank_description[idx]) {
          case 'p':
            m_piece_bitboards[PIECE_COLOR::BLACK][PIECES::PAWN].set_square(s);
            m_zobrist_hash.update_piece(PIECE_COLOR::BLACK, PIECES::PAWN, s);
            break;
          case 'n':
            m_piece_bitboards[PIECE_COLOR::BLACK][PIECES::KNIGHT].set_square(s);
            m_zobrist_hash.update_piece(PIECE_COLOR::BLACK, PIECES::KNIGHT, s);
            break;
          case 'b':
            m_piece_bitboards[PIECE_COLOR::BLACK][PIECES::BISHOP].set_square(s);
            m_zobrist_hash.update_piece(PIECE_COLOR::BLACK, PIECES::BISHOP, s);
            break;
          case 'r':
            m_piece_bitboards[PIECE_COLOR::BLACK][PIECES::ROOK].set_square(s);
            m_zobrist_hash.update_piece(PIECE_COLOR::BLACK, PIECES::ROOK, s);
            break;
          case 'q':
            m_piece_bitboards[PIECE_COLOR::BLACK][PIECES::QUEEN].set_square(s);
            m_zobrist_hash.update_piece(PIECE_COLOR::BLACK, PIECES::QUEEN, s);
            break;
          case 'k':
            m_piece_bitboards[PIECE_COLOR::BLACK][PIECES::KING].set_square(s);
            m_zobrist_hash.update_piece(PIECE_COLOR::BLACK, PIECES::KING, s);
            break;
        }
      } else {
        // Uppercase piece = white
        m_color_occupancy_bitboards[PIECE_COLOR::WHITE].set_square(s);
        switch (rank_description[idx]) {
          case 'P':
            m_piece_bitboards[PIECE_COLOR::WHITE][PIECES::PAWN].set_square(s);
            m_zobrist_hash.update_piece(PIECE_COLOR::WHITE, PIECES::PAWN, s);
            break;
          case 'N':
            m_piece_bitboards[PIECE_COLOR::WHITE][PIECES::KNIGHT].set_square(s);
            m_zobrist_hash.update_piece(PIECE_COLOR::WHITE, PIECES::KNIGHT, s);
            break;
          case 'B':
            m_piece_bitboards[PIECE_COLOR::WHITE][PIECES::BISHOP].set_square(s);
            m_zobrist_hash.update_piece(PIECE_COLOR::WHITE, PIECES::BISHOP, s);
            break;
          case 'R':
            m_piece_bitboards[PIECE_COLOR::WHITE][PIECES::ROOK].set_square(s);
            m_zobrist_hash.update_piece(PIECE_COLOR::WHITE, PIECES::ROOK, s);
            break;
          case 'Q':
            m_piece_bitboards[PIECE_COLOR::WHITE][PIECES::QUEEN].set_square(s);
            m_zobrist_hash.update_piece(PIECE_COLOR::WHITE, PIECES::QUEEN, s);
            break;
          case 'K':
            m_piece_bitboards[PIECE_COLOR::WHITE][PIECES::KING].set_square(s);
            m_zobrist_hash.update_piece(PIECE_COLOR::WHITE, PIECES::KING, s);
            break;
        }
      }

      // Advance file pointer after placing a piece
      file++;
    }
  }
}

bool Chess_Board::operator==(const Chess_Board& other) const {
  return (other.m_state == m_state) &&
         (other.m_piece_bitboards == m_piece_bitboards) &&
         (other.m_color_occupancy_bitboards == m_color_occupancy_bitboards);
}
