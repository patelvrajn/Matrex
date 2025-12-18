#include "zobrist_hash.hpp"

#include <random>

Zobrist_Hash_Keys Zobrist_Hash::m_hash_keys =
    Zobrist_Hash::initialize_hash_keys();

Zobrist_Hash::Zobrist_Hash() : m_hash_value(0) {}

uint64_t Zobrist_Hash::get_hash_value() const { return m_hash_value; };

void Zobrist_Hash::update_piece(const PIECE_COLOR color, const PIECES piece,
                                const Square square) {
  m_hash_value ^= m_hash_keys.pieces[color][piece][square.get_index()];
}

void Zobrist_Hash::update_en_passant_square(const Square s) {
  m_hash_value ^= m_hash_keys.en_passant[s.get_index()];
}

void Zobrist_Hash::update_castling_rights(const uint8_t new_rights) {
  m_hash_value ^= m_hash_keys.castling_rights[new_rights];
}

void Zobrist_Hash::flip_side_to_move() {
  m_hash_value ^= m_hash_keys.black_to_move;
}

Zobrist_Hash_Keys Zobrist_Hash::initialize_hash_keys() {
  Zobrist_Hash_Keys keys;

  std::mt19937_64 rng(100);  // Fixed seed for reproducibility.
  std::uniform_int_distribution<uint64_t> dist;

  for (uint8_t player = 0; player < NUM_OF_PLAYERS; player++) {
    for (uint8_t piece = 0; piece < NUM_OF_UNIQUE_PIECES_PER_PLAYER; piece++) {
      for (uint8_t square = 0; square < NUM_OF_SQUARES_ON_CHESS_BOARD;
           square++) {
        keys.pieces[player][piece][square] = dist(rng);
      }
    }
  }

  for (uint8_t square = 0; square < (NUM_OF_SQUARES_ON_CHESS_BOARD + 1);
       square++) {
    keys.en_passant[square] = dist(rng);
  }

  for (uint8_t cr = 0; cr < 16; cr++) {
    keys.castling_rights[cr] = dist(rng);
  }

  keys.black_to_move = dist(rng);

  return keys;
}
