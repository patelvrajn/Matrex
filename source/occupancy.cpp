#include "occupancy.hpp"

// Build a specific occupancy bitboard for a given "index" (subset selector)
// over the set bits contained in `mask`.
// NOTE: This function is typically used when enumerating all blocker
// configurations for magic bitboards.
//
// Parameters:
//   - index: selects which subset of the set bits in `mask` to turn on
//            (expected range: 0 .. (1 << popcount(mask)) - 1).
//   - num_of_high_bits_in_mask: number of set bits in `mask` that we plan to
//   iterate.
//       * This SHOULD equal popcount(mask). If it doesn't, the loop may do the
//       wrong thing.
//   - mask: the bitboard whose set bits (squares) define the "relevant"
//   squares.
//
// Returns:
//   A Bitboard `occupancy` with a subset of `mask`'s squares turned on,
//   corresponding to the bit pattern in `index`.
//
// Example:
// attack_mask = 0b00101100  // squares {2,3,5}
// bits_in_mask = 3
// index = 5  // binary 101

// Loop will:
// count = 0 → square 2 → index bit 0 = 1 → set square 2 in occupancy.
// count = 1 → square 3 → index bit 1 = 0 → leave it empty.
// count = 2 → square 5 → index bit 2 = 1 → set square 5.

// Result: occupancy = 0b00100100 (occupied at squares 2 and 5)
Bitboard set_occupancy(uint64_t index, uint8_t num_of_high_bits_in_mask,
                       const Bitboard& mask) {
  Bitboard occupancy;
  Bitboard temp_mask = mask;

  for (uint8_t idx = 0; idx < num_of_high_bits_in_mask; idx++) {
    Square s(temp_mask.get_index_of_high_lsb());

    // Remove that bit from the temp mask so next iteration advances to the next
    // square.
    temp_mask.unset_square(s);

    if (index & (1ULL << idx)) {
      occupancy.set_square(s);
    }
  }

  return occupancy;
}
