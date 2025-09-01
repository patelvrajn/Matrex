#include "attacks.hpp"

#include "leaper_attacks.hpp"

bool Attacks::m_is_attack_tables_initialized = false;

std::array<std::array<Bitboard, NUM_OF_SQUARES_ON_CHESS_BOARD>, NUM_OF_PLAYERS>
    Attacks::m_pawn_attack_tables{};
std::array<Bitboard, NUM_OF_SQUARES_ON_CHESS_BOARD>
    Attacks::m_knight_attack_tables{};
std::array<Bitboard, NUM_OF_SQUARES_ON_CHESS_BOARD>
    Attacks::m_king_attack_tables{};
std::array<std::array<Bitboard, NUM_OF_BISHOP_RAY_DIRECTIONS>,
           NUM_OF_SQUARES_ON_CHESS_BOARD>
    Attacks::m_bishop_attack_rays_table{};
std::array<std::array<Bitboard, NUM_OF_ROOK_RAY_DIRECTIONS>,
           NUM_OF_SQUARES_ON_CHESS_BOARD>
    Attacks::m_rook_attack_rays_table{};

Attacks::Attacks() {
  if (!m_is_attack_tables_initialized) {
    init_slider_rays();
    init_leaper_attacks();
  }
}

Bishop_Magic_Bitboards& Attacks::m_bishop_attack_tables() {
  static Bishop_Magic_Bitboards* bishop_attack_tables =
      new Bishop_Magic_Bitboards();
  return *bishop_attack_tables;
}

Rook_Magic_Bitboards& Attacks::m_rook_attack_tables() {
  static Rook_Magic_Bitboards* rook_attack_tables = new Rook_Magic_Bitboards();
  return *rook_attack_tables;
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
                                     const Bitboard& occupancy) {
  return m_bishop_attack_tables().get_attacks(s, occupancy);
}

Bitboard Attacks::get_rook_attacks(const Square& s, const Bitboard& occupancy) {
  return m_rook_attack_tables().get_attacks(s, occupancy);
}

Bitboard Attacks::get_bishop_rays(const Square& s, uint8_t direction) const {
  return m_bishop_attack_rays_table[s.get_index()][direction];
}

Bitboard Attacks::get_rook_rays(const Square& s, uint8_t direction) const {
  return m_rook_attack_rays_table[s.get_index()][direction];
}

Bitboard Attacks::get_queen_attacks(const Square& s,
                                    const Bitboard& occupancy) {
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

Bitboard Attacks::generate_slider_rays_on_square(Square s,
                                                 int8_t rank_direction,
                                                 int8_t file_direction) {
  Bitboard ray;

  int8_t f = s.get_file() + file_direction;
  int8_t r = s.get_rank() + rank_direction;

  while (f >= 0 && f < NUM_OF_FILES_ON_CHESS_BOARD && r >= 0 &&
         r < NUM_OF_RANKS_ON_CHESS_BOARD) {
    ray.set_square(Square(r, f));
    f += file_direction;
    r += rank_direction;
  }

  return ray;
}

void Attacks::init_slider_rays() {
  for (uint8_t square_idx = 0; square_idx < NUM_OF_SQUARES_ON_CHESS_BOARD;
       square_idx++) {
    m_bishop_attack_rays_table[square_idx][0] =
        generate_slider_rays_on_square(Square(square_idx), 1, 1);
    m_bishop_attack_rays_table[square_idx][1] =
        generate_slider_rays_on_square(Square(square_idx), -1, -1);
    m_bishop_attack_rays_table[square_idx][2] =
        generate_slider_rays_on_square(Square(square_idx), -1, 1);
    m_bishop_attack_rays_table[square_idx][3] =
        generate_slider_rays_on_square(Square(square_idx), 1, -1);

    m_rook_attack_rays_table[square_idx][0] =
        generate_slider_rays_on_square(Square(square_idx), 1, 0);
    m_rook_attack_rays_table[square_idx][1] =
        generate_slider_rays_on_square(Square(square_idx), -1, 0);
    m_rook_attack_rays_table[square_idx][2] =
        generate_slider_rays_on_square(Square(square_idx), 0, 1);
    m_rook_attack_rays_table[square_idx][3] =
        generate_slider_rays_on_square(Square(square_idx), 0, -1);
  }
}
