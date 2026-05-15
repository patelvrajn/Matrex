#include "bishop_magic_bitboards.hpp"

#include <iostream>
#include <random>

// Initialize bishop magic numbers for all 64 squares of the chessboard
// This function attempts to find "magic numbers" for each square
// that allow efficient indexing into precomputed bishop attack tables.
// Magic bitboards are a chess programming optimization that replaces
// slow ray-tracing with fast hash table lookups.
void Bishop_Magic_Bitboards::init_magics()
{
    // Random number generator (Mersenne Twister 64-bit), seeded with fixed
    // value. We use a fixed seed to ensure reproducibility of results.
    std::mt19937_64                         rng(3);
    std::uniform_int_distribution<uint64_t> dist;

    // Loop over all squares of the chessboard (0..63).
    // For each square, we will attempt to find a suitable magic number.
    for (uint8_t square_idx = 0; square_idx < NUM_OF_SQUARES_ON_CHESS_BOARD;
         square_idx++)
    {
        // Wrap raw index in a Square object.
        const Square s(square_idx);

        // Generate the *mask* of relevant squares for bishop moves from `s`.
        // The mask excludes edges (since they never block further sliding
        // moves).
        const Bitboard mask = mask_bishop_attacks(s);

        // Count how many bits are set in the mask.
        // This corresponds to how many squares can act as blockers.
        const uint8_t num_of_high_bits_in_mask = mask.high_bit_count();

        // The number of possible blocker configurations is 2^(#bits in mask).
        // This defines the size of our occupancy/attack arrays.
        const uint64_t attacks_array_size = (1ULL << num_of_high_bits_in_mask);

        // Allocate arrays to hold:
        // - `occupancies`: all possible blocker configurations
        // - `attacks`: bishop attack sets for each blocker configuration
        Bitboard* occupancies = new Bitboard[attacks_array_size];
        Bitboard* attacks     = new Bitboard[attacks_array_size];

        // Generate all possible blocker boards and their corresponding attacks
        // for this bishop square.
        for (uint64_t idx = 0; idx < attacks_array_size; idx++)
        {
            // Generate occupancy bitboard for given subset of mask bits
            occupancies[idx] =
                set_occupancy(idx, num_of_high_bits_in_mask, mask);
            // Compute bishop attack set for this occupancy
            attacks[idx] = calculate_bishop_attacks(s, occupancies[idx]);
        }

        // Attempt to find a suitable magic number for this square.
        // A magic number is valid if it perfectly maps all blocker
        // configurations into unique indices with no collisions.
        while (true)
        {
            // Allocate an array `used` to check for hash collisions during
            // magic testing.
            Bitboard* used = new Bitboard[attacks_array_size];

            // Generate a random 64-bit candidate magic number.
            // Using bitwise AND of multiple random draws increases chance
            // of producing a "sparse" number (fewer bits set),
            // which empirically tends to work better as magic multipliers.
            uint64_t magic = dist(rng) & dist(rng) & dist(rng);

            bool fail =
                false; // Will flip true if this magic number causes collisions

            // Try mapping every occupancy configuration through this magic
            for (uint64_t idx = 0; idx < attacks_array_size; idx++)
            {
                // Multiply occupancy by magic to generate hash
                uint64_t hash = occupancies[idx].get_board() * magic;

                // Extract index bits: shift right to keep only the top
                // `num_of_high_bits_in_mask` bits. This is our hash index into
                // the attack table.
                uint64_t magic_index =
                    hash >> ((sizeof(hash) << 3) - num_of_high_bits_in_mask);

                // If this slot in the used[] table is empty, assign it.
                if (used[magic_index].get_board() == 0ULL)
                {
                    used[magic_index] = attacks[idx];
                }
                // If slot already contains a different attack set, collision →
                // magic fails.
                else if (used[magic_index] != attacks[idx])
                {
                    fail = true;
                    break;
                }
            }

            // Clear the used array each iteration.
            delete[] used;

            // If no collisions occurred, magic is valid → save it for this
            // square.
            if (!fail)
            {
                m_magics[square_idx] = magic;
                break; // Move on to next square
            }
        }

        delete[] occupancies;
        delete[] attacks;
    }
}
