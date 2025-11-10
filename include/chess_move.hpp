#pragma once

#include <array>
#include <iostream>

#include "globals.hpp"
#include "score.hpp"
#include "square.hpp"

constexpr uint16_t MAXIMUM_NUM_OF_MOVES_IN_A_POSITION = 256;

typedef uint16_t Move_Score;  // Not the same as a search evaluation score.

struct Chess_Move {
  ESQUARE source_square : 7;
  ESQUARE destination_square : 7;
  PIECES moving_piece : 4;
  PIECES promoted_piece : 4;
  PIECES captured_piece : 4;
  bool is_capture : 1;
  bool is_short_castling : 1;
  bool is_long_castling : 1;
  ESQUARE castling_rook_source_square : 7;
  ESQUARE castling_rook_destination_square : 7;
  bool is_double_pawn_push : 1;
  bool is_en_passant : 1;
  ESQUARE en_passant_victim_square : 7;
  bool is_promotion : 1;
  Move_Score score = 0;

  std::string to_coordinate_notation(bool is_frc) const {
    std::string source_square_str = SQUARE_STRINGS[source_square];
    std::string destination_square_str = "";

    if ((is_short_castling || is_long_castling) && is_frc) {
      destination_square_str = SQUARE_STRINGS[castling_rook_source_square];
    } else {
      destination_square_str = SQUARE_STRINGS[destination_square];
    }

    std::string promotion_string = "";

    switch (promoted_piece) {
      case (PIECES::KNIGHT):
        promotion_string = "n";
        break;
      case (PIECES::BISHOP):
        promotion_string = "b";
        break;
      case (PIECES::ROOK):
        promotion_string = "r";
        break;
      case (PIECES::QUEEN):
        promotion_string = "q";
        break;
      default:
        promotion_string = "";
    }

    return (source_square_str + destination_square_str + promotion_string);
  }

  void pretty_print() const {
    std::cout << "Move (" << SQUARE_STRINGS[source_square] << " to "
              << SQUARE_STRINGS[destination_square] << "):" << std::endl;
    std::cout << "\tMoving piece: " << PIECE_STRINGS[moving_piece] << "\n"
              << "\tPromoted piece: " << PIECE_STRINGS[promoted_piece] << "\n"
              << "\tCaptured piece: " << PIECE_STRINGS[captured_piece] << "\n"
              << "\tis_capture: " << is_capture << "\n"
              << "\tis_short_castling: " << is_short_castling << "\n"
              << "\tis_long_castling: " << is_long_castling << "\n"
              << "\tCastling Rook Source Square: "
              << SQUARE_STRINGS[castling_rook_source_square] << "\n"
              << "\tCastling Rook Target Square: "
              << SQUARE_STRINGS[castling_rook_destination_square] << "\n"
              << "\tis_double_pawn_push: " << is_double_pawn_push << "\n"
              << "\tis_en_passant: " << is_en_passant << "\n"
              << "\tEn Passsant Victim Square: "
              << SQUARE_STRINGS[en_passant_victim_square] << "\n"
              << "\tis_promotion: " << is_promotion << "\n"
              << "\tScore: " << score << std::endl;
  }

  bool is_same_move(const Chess_Move& m) const {
    return (m.source_square == source_square) &&
           (m.destination_square == destination_square) &&
           (m.moving_piece == moving_piece) &&
           (m.promoted_piece == promoted_piece) &&
           (m.captured_piece == captured_piece) &&
           (m.is_capture == is_capture) &&
           (m.is_short_castling == is_short_castling) &&
           (m.is_long_castling == is_long_castling) &&
           (m.castling_rook_source_square == castling_rook_source_square) &&
           (m.castling_rook_destination_square ==
            castling_rook_destination_square) &&
           (m.is_double_pawn_push == is_double_pawn_push) &&
           (m.is_en_passant == is_en_passant) &&
           (m.en_passant_victim_square == en_passant_victim_square) &&
           (m.is_promotion == is_promotion);
  }

  inline auto operator<=>(const Chess_Move& other) const {
    return (score <=> other.score);
  }
};

struct Undo_Chess_Move {
  Chess_Move move;
  uint8_t castling_rights : 4;
  uint8_t half_move_clock : 6;
  ESQUARE enpassant_square : 7;
};

class Chess_Move_List {
 public:
  Chess_Move_List();

  inline void append(const Chess_Move& move);

  Chess_Move* begin() const;
  Chess_Move* end() const;

  int16_t get_max_index() const;

  Chess_Move& operator[](uint16_t index);

  void sort();

 private:
  int16_t m_max_index;
  std::array<Chess_Move, MAXIMUM_NUM_OF_MOVES_IN_A_POSITION> m_list;
};

inline void Chess_Move_List::append(const Chess_Move& move) {
  m_list[m_max_index] = move;
  m_max_index++;
}
