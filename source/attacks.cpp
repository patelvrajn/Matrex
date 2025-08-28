#include "attacks.hpp"

#include "leaper_attacks.hpp"

bool Attacks::m_is_attack_tables_initialized = false;

std::array<std::array<Bitboard, NUM_OF_SQUARES_ON_CHESS_BOARD>, NUM_OF_PLAYERS>
    Attacks::m_pawn_attack_tables{};
std::array<Bitboard, NUM_OF_SQUARES_ON_CHESS_BOARD>
    Attacks::m_knight_attack_tables{};
std::array<Bitboard, NUM_OF_SQUARES_ON_CHESS_BOARD>
    Attacks::m_king_attack_tables{};

Bishop_Magic_Bitboards Attacks::m_bishop_attack_tables;
Rook_Magic_Bitboards Attacks::m_rook_attack_tables;

Attacks::Attacks() {
  if (!m_is_attack_tables_initialized) {
    init_leaper_attacks();
  }
}

Bitboard Attacks::get_pawn_attacks(const Square& s, PIECE_COLOR c) const {
  return m_pawn_attack_tables[c][s.get_index()];
}

Bitboard Attacks::get_knight_attacks(const Square& s) const {
  return m_knight_attack_tables[s.get_index()];
}

Bitboard Attacks::get_king_attacks(const Square& s) const {
  return m_king_attack_tables[s.get_index()];
}

Bitboard Attacks::get_bishop_attacks(const Square& s,
                                     const Bitboard& occupancy) const {
  return m_bishop_attack_tables.get_attacks(s, occupancy);
}

Bitboard Attacks::get_rook_attacks(const Square& s,
                                   const Bitboard& occupancy) const {
  return m_rook_attack_tables.get_attacks(s, occupancy);
}

Bitboard Attacks::get_queen_attacks(const Square& s,
                                    const Bitboard& occupancy) const {
  return (this->get_bishop_attacks(s, occupancy) |
          this->get_rook_attacks(s, occupancy));
}

void Attacks::init_leaper_attacks() {
  for (uint8_t square_idx = 0; square_idx < NUM_OF_SQUARES_ON_CHESS_BOARD;
       square_idx++) {
    m_pawn_attack_tables[PIECE_COLOR::WHITE][square_idx] =
        mask_pawn_attacks(PIECE_COLOR::WHITE, Square(square_idx));
    m_pawn_attack_tables[PIECE_COLOR::BLACK][square_idx] =
        mask_pawn_attacks(PIECE_COLOR::BLACK, Square(square_idx));

    m_knight_attack_tables[square_idx] =
        mask_knight_attacks(Square(square_idx));

    m_king_attack_tables[square_idx] = mask_king_attacks(Square(square_idx));
  }

  m_is_attack_tables_initialized = true;
}
