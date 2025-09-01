#pragma once

#include <array>

#include "bishop_magic_bitboards.hpp"
#include "rook_magic_bitboards.hpp"

constexpr uint8_t NUM_OF_BISHOP_RAY_DIRECTIONS = 4;
constexpr uint8_t NUM_OF_ROOK_RAY_DIRECTIONS = 4;

class Attacks {
 public:
  Attacks();

  Bitboard get_pawn_attacks(const Square& s, PIECE_COLOR c) const;
  Bitboard get_knight_attacks(const Square& s) const;
  Bitboard get_king_attacks(const Square& s) const;
  Bitboard get_bishop_attacks(const Square& s, const Bitboard& occupancy);
  Bitboard get_rook_attacks(const Square& s, const Bitboard& occupancy);
  Bitboard get_queen_attacks(const Square& s, const Bitboard& occupancy);
  Bitboard get_bishop_rays(const Square& s, uint8_t direction) const;
  Bitboard get_rook_rays(const Square& s, uint8_t direction) const;

 private:
  static bool m_is_attack_tables_initialized;

  // Slider Attack Tables (Construct on First Use Idiom to avoid the Static
  // Initialization Order Fiasco)
  Bishop_Magic_Bitboards& m_bishop_attack_tables();
  Rook_Magic_Bitboards& m_rook_attack_tables();

  // Slider Rays Table
  static std::array<std::array<Bitboard, NUM_OF_BISHOP_RAY_DIRECTIONS>,
                    NUM_OF_SQUARES_ON_CHESS_BOARD>
      m_bishop_attack_rays_table;
  static std::array<std::array<Bitboard, NUM_OF_ROOK_RAY_DIRECTIONS>,
                    NUM_OF_SQUARES_ON_CHESS_BOARD>
      m_rook_attack_rays_table;

  // Leaper Attack Tables
  static std::array<std::array<Bitboard, NUM_OF_SQUARES_ON_CHESS_BOARD>,
                    NUM_OF_PLAYERS>
      m_pawn_attack_tables;
  static std::array<Bitboard, NUM_OF_SQUARES_ON_CHESS_BOARD>
      m_knight_attack_tables;
  static std::array<Bitboard, NUM_OF_SQUARES_ON_CHESS_BOARD>
      m_king_attack_tables;

  void init_leaper_attacks();

  void init_slider_rays();

  Bitboard generate_slider_rays_on_square(Square s, int8_t rank_direction,
                                          int8_t file_direction);
};
