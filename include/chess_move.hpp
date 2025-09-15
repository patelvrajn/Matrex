#pragma once

#include <iostream>
#include <memory>

#include "square.hpp"

struct Chess_Move {
  ESQUARE source_square : 7;
  ESQUARE destination_square : 7;
  PIECES moving_piece : 4;
  PIECES promoted_piece : 4;
  PIECES captured_piece : 4;
  bool is_capture : 1;
  bool is_short_castling : 1;
  bool is_long_castling : 1;
  bool is_double_pawn_push : 1;
  bool is_en_passant : 1;
  bool is_promotion : 1;
  bool is_check : 1;
  std::shared_ptr<Chess_Board> next_board_state;

  void pretty_print() const {
    std::cout << "Move (" << SQUARE_STRINGS[source_square] << " to "
              << SQUARE_STRINGS[destination_square] << "):" << std::endl;
    std::cout << "\tMoving piece: " << PIECE_STRINGS[moving_piece] << "\n"
              << "\tPromoted piece: " << PIECE_STRINGS[promoted_piece] << "\n"
              << "\tCaptured piece: " << PIECE_STRINGS[captured_piece] << "\n"
              << "\tis_capture: " << is_capture << "\n"
              << "\tis_short_castling: " << is_short_castling << "\n"
              << "\tis_long_castling: " << is_long_castling << "\n"
              << "\tis_double_pawn_push: " << is_double_pawn_push << "\n"
              << "\tis_en_passant: " << is_en_passant << "\n"
              << "\tis_promotion: " << is_promotion << "\n"
              << "\tis_check: " << is_check << std::endl;
  }

  bool operator==(const Chess_Move& m) const {
    return (m.source_square == source_square) &&
           (m.destination_square == destination_square) &&
           (m.moving_piece == moving_piece) &&
           (m.promoted_piece == promoted_piece) &&
           (m.captured_piece == captured_piece) &&
           (m.is_capture == is_capture) &&
           (m.is_short_castling == is_short_castling) &&
           (m.is_long_castling == is_long_castling) &&
           (m.is_double_pawn_push == is_double_pawn_push) &&
           (m.is_en_passant == is_en_passant) &&
           (m.is_promotion == is_promotion) && (m.is_check == is_check);
  }
};
