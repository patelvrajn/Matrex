#pragma once

#include <array>
#include <cstdint>

#include "globals.hpp"
#include "square.hpp"

struct Zobrist_Hash_Keys {
  std::array<std::array<std::array<uint64_t, NUM_OF_SQUARES_ON_CHESS_BOARD>,
                        NUM_OF_UNIQUE_PIECES_PER_PLAYER>,
             NUM_OF_PLAYERS>
      pieces;
  std::array<uint64_t, NUM_OF_SQUARES_ON_CHESS_BOARD> en_passant;
  std::array<uint64_t, 16> castling_rights;
  uint64_t black_to_move;
};

class Zobrist_Hash {
 public:
  Zobrist_Hash();

  uint64_t get_hash_value() const;

  void update_piece(const PIECE_COLOR color, const PIECES piece,
                    const Square square);
  void update_en_passant_square(const Square s);
  void update_castling_rights(const uint8_t new_rights);
  void flip_side_to_move();

  inline auto operator<=>(const Zobrist_Hash& other) const;

 private:
  uint64_t m_hash_value;

  static Zobrist_Hash_Keys m_hash_keys;

  static Zobrist_Hash_Keys initialize_hash_keys();
};

inline auto Zobrist_Hash::operator<=>(const Zobrist_Hash& other) const {
  return (get_hash_value() <=> other.get_hash_value());
}
