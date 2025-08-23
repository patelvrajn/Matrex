#include "rook_magic_bitboards.hpp"

#include <random>

#include "occupancy.hpp"

bool Rook_Magic_Bitboards::m_is_attack_tables_initialized = false;
bool Rook_Magic_Bitboards::m_is_magics_initialized = false;

std::array<Magic_Hash_Table, NUM_OF_SQUARES_ON_CHESS_BOARD>
    Rook_Magic_Bitboards::m_attack_hash_tables{};
std::array<uint64_t, NUM_OF_SQUARES_ON_CHESS_BOARD>
    Rook_Magic_Bitboards::m_magics = {
        612489824201375777ULL,   18015635461111872ULL,
        13871122036941684744ULL, 72093638208785664ULL,
        1585302287834808324ULL,  72059947680530688ULL,
        648518921872277508ULL,   36029076737097984ULL,
        140738562130016ULL,      10376363911348752384ULL,
        36873291759230984ULL,    8358821680256258048ULL,
        9385642738890571904ULL,  4644354303984128ULL,
        2306406274880768004ULL,  1829588472697728ULL,
        2415077300944900ULL,     9227893504083050496ULL,
        288249068386271488ULL,   76561743555334276ULL,
        1315192378637694976ULL,  437131738410388104ULL,
        576605888115245320ULL,   1731776993512410125ULL,
        2322170705379376ULL,     2599245489136226304ULL,
        148618862867251328ULL,   4611694816676286464ULL,
        9223377538708965376ULL,  37155248833562112ULL,
        13835902484507918340ULL, 4612816617028617217ULL,
        162130426259833902ULL,   4900207224028151842ULL,
        77124764275057728ULL,    2306300612226066690ULL,
        281646792197296ULL,      9225634833940620288ULL,
        9223389638771605768ULL,  140815879897344ULL,
        44255351439368ULL,       5485454852589109248ULL,
        281612420055088ULL,      576742370356297737ULL,
        6062132071109820432ULL,  144678206815993865ULL,
        72727265374633992ULL,    4611695506014470156ULL,
        72572070945280ULL,       4773833197735657792ULL,
        17594870431872ULL,       2306713891144532224ULL,
        54126760572747904ULL,    2814785469305344ULL,
        4774660596912103680ULL,  576461029882728960ULL,
        144397212812582973ULL,   3458905320582037505ULL,
        35188676116497ULL,       2640252875359397893ULL,
        10414600042319877ULL,    282041912927745ULL,
        81628945871552644ULL,    564136505254018ULL};

Rook_Magic_Bitboards::Rook_Magic_Bitboards() {
  // if (!m_is_magics_initialized) {
  //   init_magics();
  // }

  if (!m_is_attack_tables_initialized) {
    init_attack_hash_tables();
  }
}

Bitboard Rook_Magic_Bitboards::get_attacks(const Square& s,
                                           Bitboard occupancy) const {
  return m_attack_hash_tables[s.get_index()].get_attacks(occupancy);
}

// ============================================================================
// Initializes the rook magic bitboard attack hash tables.
// This function builds the lookup tables that allow rooks to "magically"
// compute their moves extremely fast using bitwise tricks and precomputed data.
// ============================================================================
void Rook_Magic_Bitboards::init_attack_hash_tables() {
  // Loop over all 64 squares of the chessboard
  // Each square needs its own magic hash table because rook moves
  // depend heavily on the square they are standing on
  for (uint8_t square_idx = 0; square_idx < NUM_OF_SQUARES_ON_CHESS_BOARD;
       square_idx++) {
    // Create a Square object for the current index (0–63)
    // This represents the actual chessboard square
    const Square s(square_idx);

    // Generate the rook mask for this square.
    // The mask contains all potential sliding directions (diagonals)
    // but WITHOUT including edge squares.
    const Bitboard mask = mask_rook_attacks(s);

    // Count how many bits are set in the mask.
    // This tells us how many squares the rook could theoretically interact
    // with when generating all possible occupancies.
    const uint8_t num_of_high_bits_in_mask = mask.high_bit_count();

    // Compute the total number of occupancy variations for this mask.
    // Since each relevant square in the mask can either be empty (0) or filled
    // (1), there are 2^(num_of_high_bits_in_mask) possible blocker
    // configurations.
    const uint64_t attacks_array_size = (1ULL << num_of_high_bits_in_mask);

    // Dynamically allocate an array to store all possible rook attacks
    // for each blocker configuration.
    // For each occupancy of the masked squares, we will precompute rook
    // moves.
    // Note: This saves memory compared to a fixed size array [64][4096] which
    // assumes 4096 different configurations per square.
    Bitboard* attacks = new Bitboard[attacks_array_size];

    // Iterate over every possible occupancy configuration for the mask
    for (uint64_t idx = 0; idx < attacks_array_size; idx++) {
      // Generate an occupancy bitboard from the index.
      Bitboard occupancy = set_occupancy(idx, num_of_high_bits_in_mask, mask);

      // Multiply occupancy by magic to generate hash
      uint64_t hash = occupancy.get_board() * m_magics[square_idx];

      // Extract index bits: shift right to keep only the top
      // `num_of_high_bits_in_mask` bits. This is our hash index into the
      // attack table.
      uint64_t magic_index =
          hash >> ((sizeof(hash) << 3) - num_of_high_bits_in_mask);

      // Compute rook attack bitboard for this square given the specific
      // occupancy. This simulates the rook moving along diagonals and being
      // blocked by pieces where the occupancy bitboard has 1s.
      attacks[magic_index] = calculate_rook_attacks(s, occupancy);
    }

    // Build a Magic Hash Table for this square.
    Magic_Hash_Table table(m_magics[square_idx], num_of_high_bits_in_mask, mask,
                           attacks);

    // Save the constructed hash table into the rook’s global attack tables,
    // indexed by square. After this step, looking up rook moves for this
    // square during gameplay will be O(1).
    m_attack_hash_tables[square_idx] = table;
  }

  m_is_attack_tables_initialized = true;
}

// Initialize rook magic numbers for all 64 squares of the chessboard
// This function attempts to find "magic numbers" for each square
// that allow efficient indexing into precomputed rook attack tables.
// Magic bitboards are a chess programming optimization that replaces
// slow ray-tracing with fast hash table lookups.
void Rook_Magic_Bitboards::init_magics() {
  // Random number generator (Mersenne Twister 64-bit), seeded with fixed value.
  // We use a fixed seed to ensure reproducibility of results.
  std::mt19937_64 rng(1755979527);
  std::uniform_int_distribution<uint64_t> dist;

  // Loop over all squares of the chessboard (0..63).
  // For each square, we will attempt to find a suitable magic number.
  for (uint8_t square_idx = 0; square_idx < NUM_OF_SQUARES_ON_CHESS_BOARD;
       square_idx++) {
    // Wrap raw index in a Square object.
    const Square s(square_idx);

    // Generate the *mask* of relevant squares for rook moves from `s`.
    // The mask excludes edges (since they never block further sliding moves).
    const Bitboard mask = mask_rook_attacks(s);

    // Count how many bits are set in the mask.
    // This corresponds to how many squares can act as blockers.
    const uint8_t num_of_high_bits_in_mask = mask.high_bit_count();

    // The number of possible blocker configurations is 2^(#bits in mask).
    // This defines the size of our occupancy/attack arrays.
    const uint64_t attacks_array_size = (1ULL << num_of_high_bits_in_mask);

    // Allocate arrays to hold:
    // - `occupancies`: all possible blocker configurations
    // - `attacks`: rook attack sets for each blocker configuration
    Bitboard* occupancies = new Bitboard[attacks_array_size];
    Bitboard* attacks = new Bitboard[attacks_array_size];

    // Generate all possible blocker boards and their corresponding attacks
    // for this rook square.
    for (uint64_t idx = 0; idx < attacks_array_size; idx++) {
      // Generate occupancy bitboard for given subset of mask bits
      occupancies[idx] = set_occupancy(idx, num_of_high_bits_in_mask, mask);
      // Compute rook attack set for this occupancy
      attacks[idx] = calculate_rook_attacks(s, occupancies[idx]);
    }

    // Attempt to find a suitable magic number for this square.
    // A magic number is valid if it perfectly maps all blocker configurations
    // into unique indices with no collisions.
    while (true) {
      // Allocate an array `used` to check for hash collisions during magic
      // testing.
      Bitboard* used = new Bitboard[attacks_array_size];

      // Generate a random 64-bit candidate magic number.
      // Using bitwise AND of multiple random draws increases chance
      // of producing a "sparse" number (fewer bits set),
      // which empirically tends to work better as magic multipliers.
      uint64_t magic = dist(rng) & dist(rng) & dist(rng);

      bool fail =
          false;  // Will flip true if this magic number causes collisions

      // Try mapping every occupancy configuration through this magic
      for (uint64_t idx = 0; idx < attacks_array_size; idx++) {
        // Multiply occupancy by magic to generate hash
        uint64_t hash = occupancies[idx].get_board() * magic;

        // Extract index bits: shift right to keep only the top
        // `num_of_high_bits_in_mask` bits. This is our hash index into the
        // attack table.
        uint64_t magic_index =
            hash >> ((sizeof(hash) << 3) - num_of_high_bits_in_mask);

        // If this slot in the used[] table is empty, assign it.
        if (used[magic_index].get_board() == 0ULL) {
          used[magic_index] = attacks[idx];
        }
        // If slot already contains a different attack set, collision → magic
        // fails.
        else if (used[magic_index] != attacks[idx]) {
          fail = true;
          break;
        }
      }

      // Clear the used array each iteration.
      delete[] used;

      // If no collisions occurred, magic is valid → save it for this square.
      if (!fail) {
        m_magics[square_idx] = magic;
        break;  // Move on to next square
      }
    }

    delete[] occupancies;
    delete[] attacks;
  }

  m_is_magics_initialized = true;
}

// Generate rook attack mask for a square (without board edges)
Bitboard Rook_Magic_Bitboards::mask_rook_attacks(const Square& s) const {
  Bitboard attacks;

  const uint8_t rook_rank = s.get_rank();
  const uint8_t rook_file = s.get_file();

  for (int8_t r = rook_rank + 1; r <= 6; r++) {
    attacks.set_square(Square(r, rook_file));
  }
  for (int8_t r = rook_rank - 1; r >= 1; r--) {
    attacks.set_square(Square(r, rook_file));
  }
  for (int8_t f = rook_file + 1; f <= 6; f++) {
    attacks.set_square(Square(rook_rank, f));
  }
  for (int8_t f = rook_file - 1; f >= 1; f--) {
    attacks.set_square(Square(rook_rank, f));
  }

  return attacks;
}

// Given blockers, generate rook attacks from a square with board edges.
Bitboard Rook_Magic_Bitboards::calculate_rook_attacks(
    const Square& s, const Bitboard& blockers) const {
  Bitboard attacks;

  const uint8_t rook_rank = s.get_rank();
  const uint8_t rook_file = s.get_file();

  for (int8_t r = rook_rank + 1; r <= 7; r++) {
    attacks.set_square(Square(r, rook_file));
    if (blockers.get_board() &
        Square(r, rook_file)
            .get_mask()) {  // If a blocker exists on the square, stop the ray.
      break;
    }
  }
  for (int8_t r = rook_rank - 1; r >= 0; r--) {
    attacks.set_square(Square(r, rook_file));
    if (blockers.get_board() & Square(r, rook_file).get_mask()) {
      break;
    }
  }
  for (int8_t f = rook_file + 1; f <= 7; f++) {
    attacks.set_square(Square(rook_rank, f));
    if (blockers.get_board() & Square(rook_rank, f).get_mask()) {
      break;
    }
  }
  for (int8_t f = rook_file - 1; f >= 0; f--) {
    attacks.set_square(Square(rook_rank, f));
    if (blockers.get_board() & Square(rook_rank, f).get_mask()) {
      break;
    }
  }

  return attacks;
}
