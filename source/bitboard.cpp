#include "bitboard.hpp"

#include <bit>
#include <iostream>

#include "globals.hpp"

bool Bitboard::m_are_masks_initialized = false;

std::array<std::array<uint64_t, NUM_OF_SQUARES_ON_CHESS_BOARD>,
           NUM_OF_SQUARES_ON_CHESS_BOARD>
    Bitboard::m_between_squares_masks{};

std::array<uint64_t, NUM_OF_SQUARES_ON_CHESS_BOARD> Bitboard::m_rank_masks{};
std::array<uint64_t, NUM_OF_SQUARES_ON_CHESS_BOARD> Bitboard::m_file_masks{};
std::array<uint64_t, NUM_OF_SQUARES_ON_CHESS_BOARD>
    Bitboard::m_diagonal_masks{};
std::array<uint64_t, NUM_OF_SQUARES_ON_CHESS_BOARD>
    Bitboard::m_antidiagonal_masks{};

Bitboard::Iterator::Iterator(uint64_t board) : m_board(board) {}

Square Bitboard::Iterator::operator*() const {
  return Square(__builtin_ctzll(m_board));
}

Bitboard::Iterator& Bitboard::Iterator::operator++() {
  m_board &= (m_board - 1);
  return *this;
}

bool Bitboard::Iterator::operator==(const Iterator& other) const {
  return (other.m_board == m_board);
}

bool Bitboard::Iterator::operator!=(const Iterator& other) const {
  return (other.m_board != m_board);
}

Bitboard::Bitboard() : m_board(0) {
  if (!m_are_masks_initialized) {
    init_geometry_masks();
    init_between_squares_masks();
  }
}

Bitboard::Iterator Bitboard::begin() const {
  return Bitboard::Iterator(m_board);
}

Bitboard::Iterator Bitboard::end() const { return Bitboard::Iterator(0ULL); }

void Bitboard::pretty_print() const {
  std::cout << "Bitboard: " << m_board << " (0x" << std::hex << m_board << ")"
            << std::endl;

  for (uint8_t rank = 0; rank < NUM_OF_RANKS_ON_CHESS_BOARD; rank++) {
    // Print the ranks on the left hand side of the board before the first file.
    std::cout << (NUM_OF_RANKS_ON_CHESS_BOARD - rank) << "   ";

    for (uint8_t file = 0; file < NUM_OF_FILES_ON_CHESS_BOARD; file++) {
      Square s(rank, file);

      if (get_square(s)) {
        std::cout << 1 << " ";
      } else {
        std::cout << 0 << " ";
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
}

uint64_t Bitboard::get_board() const { return m_board; }

void Bitboard::set_board(uint64_t board) { m_board = board; }

void Bitboard::unset_square(const Square& s) { m_board &= ~(s.get_mask()); }

void Bitboard::set_square(const Square& s) {
  m_board = (m_board | s.get_mask());
}

uint64_t Bitboard::get_square(const Square& s) const {
  return (m_board & s.get_mask());
}

// Method to count how many bits are set to 1 in the bitboard (population
// count).
uint8_t Bitboard::high_bit_count() const {
  // Counter to keep track of how many set bits we find.
  uint8_t high_bit_cnt = 0;

  // Make a temporary copy of the internal board so we don’t mutate the
  // original.
  uint64_t temp_board = m_board;

  // Loop until there are no more set bits left in temp_board.
  while (temp_board) {
    // Increment counter for each set bit we remove.
    high_bit_cnt++;

    // Clear the least significant set bit.
    // Trick: x & (x - 1) removes the lowest high bit in x.
    temp_board &= (temp_board - 1);
  }

  // Return the total number of set bits in the bitboard.
  return high_bit_cnt;
}

uint8_t Bitboard::low_bit_count() const {
  return (NUM_OF_SQUARES_ON_CHESS_BOARD - high_bit_count());
}

int8_t Bitboard::get_index_of_high_lsb() const {
  if (m_board == 0) {
    return -1;
  }

  return __builtin_ctzll(m_board);
}

int8_t Bitboard::get_index_of_high_msb() const {
  if (m_board == 0) {
    return -1;
  }

  return ((NUM_OF_SQUARES_ON_CHESS_BOARD - 1) - std::countl_zero(m_board));
}

uint64_t Bitboard::get_between_squares_mask(const Square& a,
                                            const Square& b) const {
  return m_between_squares_masks[a.get_index()][b.get_index()];
}

uint64_t Bitboard::generate_between_squares_mask(const Square& a,
                                                 const Square& b) const {
  uint64_t mask = 0;
  int8_t delta = 0;

  // There are no squares in between a and b if they are the same square.
  if (a == b) {
    return mask;
  }

  const int8_t distance = b.get_index() - a.get_index();

  if ((distance & 7) == 0) {  // Squares a and b are on the same file.

    delta = ((distance > 0) ? 8 : -8);  // Move along the file.

  } else if (b.get_rank() ==
             a.get_rank()) {  // Squares a and b are on the same rank.

    delta = ((distance > 0) ? 1 : -1);  // Move along the rank.

  } else if ((distance % 9) == 0) {  // Squares a and b are on a diagonal.

    delta = ((distance > 0) ? 9 : -9);  // Move along the diagonal.

  } else if ((distance % 7) == 0) {  // Squares a and b are on another diagonal.

    delta = ((distance > 0) ? 7 : -7);  // Move along the diagonal.

  } else {
    return mask;  // No diagonal or orthogonal path between the squares.
  }

  // Starting from square a travel until you get to square b while setting the
  // bits in the mask corresponding to squares between squres a and b.
  int square_index = a.get_index() + delta;
  while (square_index != b.get_index()) {
    mask |= Square(square_index).get_mask();
    square_index += delta;
  }

  return mask;
}

void Bitboard::init_between_squares_masks() {
  for (uint8_t outer_square_idx = 0;
       outer_square_idx < NUM_OF_SQUARES_ON_CHESS_BOARD; outer_square_idx++) {
    for (uint8_t inner_square_idx = 0;
         inner_square_idx < NUM_OF_SQUARES_ON_CHESS_BOARD; inner_square_idx++) {
      m_between_squares_masks[outer_square_idx][inner_square_idx] =
          generate_between_squares_mask(Square(outer_square_idx),
                                        Square(inner_square_idx));
    }
  }

  m_are_masks_initialized = true;
}

void Bitboard::init_geometry_masks() {
  for (uint8_t outer_square_index = 0;
       outer_square_index < NUM_OF_SQUARES_ON_CHESS_BOARD;
       outer_square_index++) {
    const Square outer_square(outer_square_index);
    const uint8_t r = outer_square.get_rank();
    const uint8_t f = outer_square.get_file();

    uint64_t rank = 0;
    uint64_t file = 0;
    uint64_t diag = 0;
    uint64_t antidiag = 0;

    for (uint8_t inner_square_index = 0;
         inner_square_index < NUM_OF_SQUARES_ON_CHESS_BOARD;
         inner_square_index++) {
      const Square inner_square(inner_square_index);
      const uint8_t rr = inner_square.get_rank();
      const uint8_t ff = inner_square.get_file();

      if (rr == r) rank |= inner_square.get_mask();
      if (ff == f) file |= inner_square.get_mask();
      if ((rr - ff) == (r - f)) diag |= inner_square.get_mask();
      if ((rr + ff) == (r + f)) antidiag |= inner_square.get_mask();
    }

    m_rank_masks[outer_square_index] = rank;
    m_file_masks[outer_square_index] = file;
    m_diagonal_masks[outer_square_index] = diag;
    m_antidiagonal_masks[outer_square_index] = antidiag;
  }
}

Bitboard Bitboard::get_rank_mask(const Square& s) {
  return Bitboard(m_rank_masks[s.get_index()]);
}

Bitboard Bitboard::get_file_mask(const Square& s) {
  return Bitboard(m_file_masks[s.get_index()]);
}

Bitboard Bitboard::get_diagonal_mask(const Square& s) {
  return Bitboard(m_diagonal_masks[s.get_index()]);
}

Bitboard Bitboard::get_antidiagonal_mask(const Square& s) {
  return Bitboard(m_antidiagonal_masks[s.get_index()]);
}

Bitboard Bitboard::get_infinite_ray(const Square& a, const Square& b) {
  if (a.get_rank() == b.get_rank()) {
    return get_rank_mask(a);
  }

  if (a.get_file() == b.get_file()) {
    return get_file_mask(a);
  }

  if (a.get_diagonal() == b.get_diagonal()) {
    return get_diagonal_mask(a);
  }

  if (a.get_antidiagonal() == b.get_antidiagonal()) {
    return get_antidiagonal_mask(a);
  }

  return Bitboard();
}
