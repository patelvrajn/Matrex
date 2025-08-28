#pragma once

#include <array>

#include "bitboard.hpp"
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

  void pretty_print() const;

  std::pair<PIECE_COLOR, PIECES> what_piece_is_on_square(Square& s) const;

  void set_from_fen(const std::string& fen);

  Bitboard get_both_occupancies();

 private:
  std::array<std::array<Bitboard, NUM_OF_UNIQUE_PIECES_PER_PLAYER>,
             NUM_OF_PLAYERS>
      m_piece_bitboards;
  std::array<Bitboard, NUM_OF_PLAYERS> m_color_occupancy_bitboards;

  chess_board_state m_state;

  void place_pieces_from_fen(const std::string& rank_description,
                             uint8_t length_of_description, uint8_t rank);
};
