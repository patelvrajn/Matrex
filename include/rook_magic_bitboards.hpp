#pragma once

#include <array>

#include "bitboard.hpp"
#include "globals.hpp"
#include "magic_hash_table.hpp"
#include "occupancy.hpp"
#include "psuedo_random_number_generator.hpp"

// Generate rook attack mask for a square (without board edges i.e. ranks 0 and
// 7, files 0 and 7)
constexpr Bitboard mask_rook_attacks(const Square s)
{
    constexpr int8_t FIRST_RANK_NOT_ON_EDGE = 1;
    constexpr int8_t LAST_RANK_NOT_ON_EDGE  = 6;

    constexpr int8_t FIRST_FILE_NOT_ON_EDGE = 1;
    constexpr int8_t LAST_FILE_NOT_ON_EDGE  = 6;

    Bitboard attacks;

    const uint8_t rook_rank = s.get_rank();
    const uint8_t rook_file = s.get_file();

    for (int8_t r = rook_rank + 1; r <= LAST_RANK_NOT_ON_EDGE; r++)
    {
        attacks.set_square(Square(r, rook_file));
    }
    for (int8_t r = rook_rank - 1; r >= FIRST_RANK_NOT_ON_EDGE; r--)
    {
        attacks.set_square(Square(r, rook_file));
    }
    for (int8_t f = rook_file + 1; f <= LAST_FILE_NOT_ON_EDGE; f++)
    {
        attacks.set_square(Square(rook_rank, f));
    }
    for (int8_t f = rook_file - 1; f >= FIRST_FILE_NOT_ON_EDGE; f--)
    {
        attacks.set_square(Square(rook_rank, f));
    }

    return attacks;
}

// Given blockers, generate rook attacks from a square with board edges.
constexpr Bitboard calculate_rook_attacks(const Square   s,
                                          const Bitboard blockers)
{
    constexpr int8_t FIRST_RANK = 0;
    constexpr int8_t LAST_RANK  = 7;

    constexpr int8_t FIRST_FILE = 0;
    constexpr int8_t LAST_FILE  = 7;

    Bitboard attacks;

    const uint8_t rook_rank = s.get_rank();
    const uint8_t rook_file = s.get_file();

    for (int8_t r = rook_rank + 1; r <= LAST_RANK; r++)
    {
        attacks.set_square(Square(r, rook_file));

        // If a blocker exists on the square, stop the ray.
        if (blockers.get_square(Square(r, rook_file))) { break; }
    }
    for (int8_t r = rook_rank - 1; r >= FIRST_RANK; r--)
    {
        attacks.set_square(Square(r, rook_file));
        if (blockers.get_square(Square(r, rook_file))) { break; }
    }
    for (int8_t f = rook_file + 1; f <= LAST_FILE; f++)
    {
        attacks.set_square(Square(rook_rank, f));
        if (blockers.get_square(Square(rook_rank, f))) { break; }
    }
    for (int8_t f = rook_file - 1; f >= FIRST_FILE; f--)
    {
        attacks.set_square(Square(rook_rank, f));
        if (blockers.get_square(Square(rook_rank, f))) { break; }
    }

    return attacks;
}

// =============================================================================
// Rook Magic Number Generator
//
// Used to initialize rook magic numbers for all 64 squares of the chessboard.
// This function attempts to find "magic numbers" (numbers that convert an
// occupancy bitboard to a hash used to index the attack bitboards table) for
// each square that allow efficient indexing into precomputed rook attack
// bitboards.
// =============================================================================
constexpr Magics_Array init_rook_magics()
{
    Magics_Array output;

    constexpr uint64_t   PRNG_SEED = 95647789;
    Psuedo_RNG<uint64_t> prng(PRNG_SEED);

    // For each square, we will attempt to find a suitable magic number.
    for (uint8_t square_idx = 0; square_idx < NUM_OF_SQUARES_ON_CHESS_BOARD;
         square_idx++)
    {
        const Square s(square_idx);

        const Bitboard mask = mask_rook_attacks(s);

        // Count how many bits are set in the mask which corresponds to how many
        // squares can act as blockers.
        const uint8_t num_of_high_bits_in_mask = mask.high_bit_count();

        // The number of possible blocker configurations is 2^(#bits in mask).
        // This defines the size of our occupancy/attack arrays.
        const uint64_t attacks_array_size = (1ULL << num_of_high_bits_in_mask);

        // Allocate arrays to hold:
        // → occupancies: all possible blocker configurations
        // → attacks: rook attack sets for each blocker configuration
        Bitboard* occupancies = new Bitboard[attacks_array_size];
        Bitboard* attacks     = new Bitboard[attacks_array_size];

        // Generate all possible blocker boards and their corresponding rook
        // attacks for this square.
        for (uint64_t idx = 0; idx < attacks_array_size; idx++)
        {
            occupancies[idx] =
                set_occupancy(idx, num_of_high_bits_in_mask, mask);
            attacks[idx] = calculate_rook_attacks(s, occupancies[idx]);
        }

        // Attempt to find a suitable magic number for this square. A magic
        // number is valid only if maps all possible occupancy configurations to
        // a respective attack bitboard with no collisions.
        while (true)
        {
            // Allocate an array to check for hash collisions.
            Bitboard* used = new Bitboard[attacks_array_size];

            // Generate a random sparse (fewer bits set) 64-bit candidate magic
            // number. Sparseness empirically tends to work better as magic
            // multipliers.
            const uint64_t magic = prng.generate_sparse_random();

            // Boolean set if there is a collision.
            bool fail = false;

            // Try mapping every occupancy configuration through this magic
            for (uint64_t idx = 0; idx < attacks_array_size; idx++)
            {
                // Multiply occupancy by the magic number to generate a hash.
                const uint64_t hash = occupancies[idx].get_board() * magic;

                // Extract bits for indexing the attack table. The magic index
                // will always be at most log2(attacks array size) hence, the
                // bit shift logic.
                const uint64_t magic_index =
                    hash >> ((sizeof(hash) << 3) - num_of_high_bits_in_mask);

                // If this slot is not used, assign it an attack bitboard.
                if (used[magic_index] == EMPTY_BITBOARD)
                {
                    used[magic_index] = attacks[idx];
                }
                // If slot already contains a different attack set, this is a
                // collision and we have not found a proper magic number.
                else if (used[magic_index] != attacks[idx])
                {
                    fail = true;
                    break;
                }
            }

            delete[] used;

            // If no collisions occurred, save the magic number for this square.
            if (!fail)
            {
                output[square_idx] = magic;
                break; // Move on to next square
            }
        }

        delete[] occupancies;
        delete[] attacks;
    }

    return output;
}

constexpr Magics_Array rook_magics = {
    612489824201375777ULL,   18015635461111872ULL,   13871122036941684744ULL,
    72093638208785664ULL,    1585302287834808324ULL, 72059947680530688ULL,
    648518921872277508ULL,   36029076737097984ULL,   140738562130016ULL,
    10376363911348752384ULL, 36873291759230984ULL,   8358821680256258048ULL,
    9385642738890571904ULL,  4644354303984128ULL,    2306406274880768004ULL,
    1829588472697728ULL,     2415077300944900ULL,    9227893504083050496ULL,
    288249068386271488ULL,   76561743555334276ULL,   1315192378637694976ULL,
    437131738410388104ULL,   576605888115245320ULL,  1731776993512410125ULL,
    2322170705379376ULL,     2599245489136226304ULL, 148618862867251328ULL,
    4611694816676286464ULL,  9223377538708965376ULL, 37155248833562112ULL,
    13835902484507918340ULL, 4612816617028617217ULL, 162130426259833902ULL,
    4900207224028151842ULL,  77124764275057728ULL,   2306300612226066690ULL,
    281646792197296ULL,      9225634833940620288ULL, 9223389638771605768ULL,
    140815879897344ULL,      44255351439368ULL,      5485454852589109248ULL,
    281612420055088ULL,      576742370356297737ULL,  6062132071109820432ULL,
    144678206815993865ULL,   72727265374633992ULL,   4611695506014470156ULL,
    72572070945280ULL,       4773833197735657792ULL, 17594870431872ULL,
    2306713891144532224ULL,  54126760572747904ULL,   2814785469305344ULL,
    4774660596912103680ULL,  576461029882728960ULL,  144397212812582973ULL,
    3458905320582037505ULL,  35188676116497ULL,      2640252875359397893ULL,
    10414600042319877ULL,    282041912927745ULL,     81628945871552644ULL,
    564136505254018ULL};

// =============================================================================
// Create Rook Attacks Array
//
// Given the magic numbers, it iterates through each possible blocker
// configuration for a rook on the square specified and outputs the following:
//
// → An array of attack bitboards for the rook (one per blocker configuration)
// → An info struct that has the mask used to generate the occupancies and the
//   the number of bits set to 1 in the mask.
// =============================================================================
template <auto square_idx>
constexpr auto create_rook_attacks_array()
{
    constexpr Square s(square_idx);

    constexpr Bitboard mask = mask_rook_attacks(s);

    constexpr uint8_t num_of_high_bits_in_mask = mask.high_bit_count();

    constexpr Magic_Attacks_Info_Storage info {.mask = mask,
                                               .num_of_idx_bits =
                                                   num_of_high_bits_in_mask};

    constexpr uint64_t attacks_array_size = (1ULL << num_of_high_bits_in_mask);

    constexpr Multi_Array<Bitboard, attacks_array_size> attacks = ([&]() {

    Multi_Array<Bitboard, attacks_array_size> output{};

    constexpr uint64_t ATTACKS_INDEX_MINIMUM = 0;
    constexpr uint64_t ATTACK_INDEX_INCREMENT = 1;

    constexpr_for<ATTACKS_INDEX_MINIMUM, attacks_array_size, ATTACK_INDEX_INCREMENT>([&](auto idx)
    {
        constexpr Bitboard occupancy =
            set_occupancy(idx, num_of_high_bits_in_mask, mask);

        constexpr uint64_t hash = occupancy.get_board() * rook_magics[square_idx];

        constexpr uint64_t magic_index =
            hash >> ((sizeof(hash) << 3) - num_of_high_bits_in_mask);

        output[magic_index] = calculate_rook_attacks(s, occupancy);
    });

    return output;

    })();

    return std::pair {attacks, info};
}

constexpr auto init_rook_attack_hash_tables()
{
    return index_sequence_unpacker<NUM_OF_SQUARES_ON_CHESS_BOARD>(
        []<std::size_t... square_index>()
        {
            return Compile_Time_Jagged_Array<
                Bitboard,
                decltype(create_rook_attacks_array<square_index>().first)...>(
                create_rook_attacks_array<square_index>().first...);
        });
}

constexpr auto init_rook_attack_hash_tables_infos()
{
    return index_sequence_unpacker<NUM_OF_SQUARES_ON_CHESS_BOARD>(
        []<std::size_t... square_index>()
        {
            return Magic_Attacks_Info_Array {
                create_rook_attacks_array<square_index>().second...};
        });
}

class Rook_Magic_Bitboards
{
  public:

    constexpr Rook_Magic_Bitboards();

    constexpr Bitboard get_attacks(const Square   s,
                                   const Bitboard occupancy) const;

  private:

    static constexpr auto m_attack_hash_tables =
        Magic_Hash_Jagged_Table<rook_magics,
                                init_rook_attack_hash_tables,
                                init_rook_attack_hash_tables_infos>();
};

constexpr Rook_Magic_Bitboards::Rook_Magic_Bitboards() {}

constexpr Bitboard
Rook_Magic_Bitboards::get_attacks(const Square   s,
                                  const Bitboard occupancy) const
{
    return m_attack_hash_tables.get_attacks(s, occupancy);
}
