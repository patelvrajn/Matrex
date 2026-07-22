#pragma once

#include <array>

#include "globals.hpp"
#include "magic_hash_table.hpp"
#include "occupancy.hpp"
#include "psuedo_random_number_generator.hpp"

// Generate bishop attack mask for a square (without board edges i.e. ranks 0
// and 7, files 0 and 7)
constexpr Bitboard mask_bishop_attacks(const Square s)
{
    constexpr int8_t FIRST_RANK_NOT_ON_EDGE = 1;
    constexpr int8_t LAST_RANK_NOT_ON_EDGE  = 6;

    constexpr int8_t FIRST_FILE_NOT_ON_EDGE = 1;
    constexpr int8_t LAST_FILE_NOT_ON_EDGE  = 6;

    Bitboard attacks;

    const uint8_t bishop_rank = s.get_rank();
    const uint8_t bishop_file = s.get_file();

    for (int8_t r = bishop_rank + 1, f = bishop_file + 1;
         r <= LAST_RANK_NOT_ON_EDGE && f <= LAST_FILE_NOT_ON_EDGE;
         ++r, ++f)
    {
        attacks.set_square(Square(r, f));
    }

    for (int8_t r = bishop_rank + 1, f = bishop_file - 1;
         r <= LAST_RANK_NOT_ON_EDGE && f >= FIRST_FILE_NOT_ON_EDGE;
         ++r, --f)
    {
        attacks.set_square(Square(r, f));
    }

    for (int8_t r = bishop_rank - 1, f = bishop_file + 1;
         r >= FIRST_RANK_NOT_ON_EDGE && f <= LAST_FILE_NOT_ON_EDGE;
         --r, ++f)
    {
        attacks.set_square(Square(r, f));
    }

    for (int8_t r = bishop_rank - 1, f = bishop_file - 1;
         r >= FIRST_RANK_NOT_ON_EDGE && f >= FIRST_FILE_NOT_ON_EDGE;
         --r, --f)
    {
        attacks.set_square(Square(r, f));
    }

    return attacks;
}

// Given blockers, generate bishop attacks from a square with board edges.
constexpr Bitboard calculate_bishop_attacks(const Square   s,
                                            const Bitboard blockers)
{
    constexpr int8_t FIRST_RANK = 0;
    constexpr int8_t LAST_RANK  = 7;

    constexpr int8_t FIRST_FILE = 0;
    constexpr int8_t LAST_FILE  = 7;

    Bitboard attacks;

    const uint8_t bishop_rank = s.get_rank();
    const uint8_t bishop_file = s.get_file();

    for (int8_t r = bishop_rank + 1, f = bishop_file + 1;
         r <= LAST_RANK && f <= LAST_FILE;
         ++r, ++f)
    {
        attacks.set_square(Square(r, f));

        // If a blocker exists on the square, stop the ray.
        if (blockers.get_square(Square(r, f))) { break; }
    }

    for (int8_t r = bishop_rank + 1, f = bishop_file - 1;
         r <= LAST_RANK && f >= FIRST_FILE;
         ++r, --f)
    {
        attacks.set_square(Square(r, f));
        if (blockers.get_square(Square(r, f))) { break; }
    }

    for (int8_t r = bishop_rank - 1, f = bishop_file + 1;
         r >= FIRST_RANK && f <= LAST_FILE;
         --r, ++f)
    {
        attacks.set_square(Square(r, f));
        if (blockers.get_square(Square(r, f))) { break; }
    }

    for (int8_t r = bishop_rank - 1, f = bishop_file - 1;
         r >= FIRST_RANK && f >= FIRST_FILE;
         --r, --f)
    {
        attacks.set_square(Square(r, f));
        if (blockers.get_square(Square(r, f))) { break; }
    }

    return attacks;
}

// =============================================================================
// Bishop Magic Number Generator
//
// Used to initialize bishop magic numbers for all 64 squares of the chessboard.
// This function attempts to find "magic numbers" (numbers that convert an
// occupancy bitboard to a hash used to index the attack bitboards table) for
// each square that allow efficient indexing into precomputed bishop attack
// bitboards.
// =============================================================================
constexpr Magics_Array init_bishop_magics()
{
    Magics_Array output;

    constexpr uint64_t   PRNG_SEED = 177348;
    Psuedo_RNG<uint64_t> prng(PRNG_SEED);

    // For each square, we will attempt to find a suitable magic number.
    for (uint8_t square_idx = 0; square_idx < NUM_OF_SQUARES_ON_CHESS_BOARD;
         ++square_idx)
    {
        const Square s(square_idx);

        const Bitboard mask = mask_bishop_attacks(s);

        // Count how many bits are set in the mask which corresponds to how many
        // squares can act as blockers.
        const uint8_t num_of_high_bits_in_mask = mask.high_bit_count();

        // The number of possible blocker configurations is 2^(#bits in mask).
        // This defines the size of our occupancy/attack arrays.
        const uint64_t attacks_array_size = (1ULL << num_of_high_bits_in_mask);

        // Allocate arrays to hold:
        // → occupancies: all possible blocker configurations
        // → attacks: bishop attack sets for each blocker configuration
        Bitboard* occupancies = new Bitboard[attacks_array_size];
        Bitboard* attacks     = new Bitboard[attacks_array_size];

        // Generate all possible blocker boards and their corresponding bishop
        // attacks for this square.
        for (uint64_t idx = 0; idx < attacks_array_size; ++idx)
        {
            occupancies[idx] =
                set_occupancy(idx, num_of_high_bits_in_mask, mask);
            attacks[idx] = calculate_bishop_attacks(s, occupancies[idx]);
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
            for (uint64_t idx = 0; idx < attacks_array_size; ++idx)
            {
                // Multiply occupancy by the magic number to generate a hash.
                const uint64_t hash = occupancies[idx].get_board() * magic;

                // Extract bits for indexing the attack table. The magic index
                // will always be at most log2(attacks array size) hence, the
                // bit shift logic.
                const uint64_t magic_index =
                    hash >> (SIZE_OF_IN_BITS(hash) - num_of_high_bits_in_mask);

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

constexpr Magics_Array bishop_magics = {
    1134730381265408ULL,    20284086875130114ULL,   434606161820387089ULL,
    4612883387133264427ULL, 1189249644271509520ULL, 5932974407180288ULL,
    2378186819975520388ULL, 144154779093049344ULL,  1153502184218853905ULL,
    144689305078530112ULL,  72083991242615810ULL,   13585582861066256ULL,
    9020463265497344ULL,    2307814438264114240ULL, 2594233922922750976ULL,
    4686558947489874947ULL, 289375277271746062ULL,  2252006245998784ULL,
    37161328359768072ULL,   3377734118041616ULL,    5188992055191798784ULL,
    306526258244813154ULL,  288512418131346176ULL,  36063983773495296ULL,
    289374968330981377ULL,  9296010173569106432ULL, 9800963087145635856ULL,
    145249884077826112ULL,  5911819009153581059ULL, 577024320832819204ULL,
    290069859709223424ULL,  299342057701576ULL,     2850071579273728ULL,
    5084348059747216ULL,    4616823280350265376ULL, 1532721357127808ULL,
    9297701239099756800ULL, 1134702442852353ULL,    282720519062017ULL,
    2252420436526409ULL,    6936405478197429248ULL, 9372836658270969872ULL,
    4721461948284437506ULL, 2488531402056835328ULL, 2305913380407378177ULL,
    9307051503162229248ULL, 4612266569190353024ULL, 577116062315971075ULL,
    2328924850791923712ULL, 71485705289728ULL,      9227911387857625088ULL,
    6918092001764704868ULL, 576460822130327552ULL,  1203028483136882816ULL,
    9277432850348310528ULL, 2269484375098384ULL,    20100850128900ULL,
    188995864580ULL,        9104027699120129ULL,    259521302927345668ULL,
    117111182772536832ULL,  1224981572579758336ULL, 324549459340427776ULL,
    289360742829916288ULL};

// =============================================================================
// Create Bishop Attacks Array
//
// Given the magic numbers, it iterates through each possible blocker
// configuration for a bishop on the square specified and outputs the following:
//
// → An array of attack bitboards for the bishop (one per blocker configuration)
// → An info struct that has the mask used to generate the occupancies and the
//   the number of bits set to 1 in the mask.
// =============================================================================
template <auto square_idx>
constexpr auto create_bishop_attacks_array()
{
    constexpr Square s(square_idx);

    constexpr Bitboard mask = mask_bishop_attacks(s);

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

        constexpr uint64_t hash = occupancy.get_board() * bishop_magics[square_idx];

        constexpr uint64_t magic_index =
            hash >> (SIZE_OF_IN_BITS(hash) - num_of_high_bits_in_mask);

        output[magic_index] = calculate_bishop_attacks(s, occupancy);
    });

    return output;

    })();

    return std::pair {attacks, info};
}

constexpr auto init_bishop_attack_hash_tables()
{
    return index_sequence_unpacker<NUM_OF_SQUARES_ON_CHESS_BOARD>(
        []<std::size_t... square_index>()
        {
            return Compile_Time_Jagged_Array<
                Bitboard,
                decltype(create_bishop_attacks_array<square_index>().first)...>(
                create_bishop_attacks_array<square_index>().first...);
        });
}

constexpr auto init_bishop_attack_hash_tables_infos()
{
    return index_sequence_unpacker<NUM_OF_SQUARES_ON_CHESS_BOARD>(
        []<std::size_t... square_index>()
        {
            return Magic_Attacks_Info_Array {
                create_bishop_attacks_array<square_index>().second...};
        });
}

class Bishop_Magic_Bitboards
{
  public:

    constexpr Bishop_Magic_Bitboards();

    constexpr Bitboard get_attacks(const Square   s,
                                   const Bitboard occupancy) const;

  private:

    static constexpr auto m_attack_hash_tables =
        Magic_Hash_Jagged_Table<bishop_magics,
                                init_bishop_attack_hash_tables,
                                init_bishop_attack_hash_tables_infos>();
};

constexpr Bishop_Magic_Bitboards::Bishop_Magic_Bitboards() {}

constexpr Bitboard
Bishop_Magic_Bitboards::get_attacks(const Square   s,
                                    const Bitboard occupancy) const
{
    return m_attack_hash_tables.get_attacks(s, occupancy);
}
