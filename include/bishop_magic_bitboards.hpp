#pragma once

#include <array>

#include "globals.hpp"
#include "magic_hash_table.hpp"
#include "occupancy.hpp"
#include "psuedo_random_number_generator.hpp"

// Generate bishop attack mask for a square (without board edges)
constexpr Bitboard mask_bishop_attacks(const Square s)
{
    Bitboard attacks;

    const uint8_t bishop_rank = s.get_rank();
    const uint8_t bishop_file = s.get_file();

    for (int8_t r = bishop_rank + 1, f = bishop_file + 1; r <= 6 && f <= 6;
         r++, f++)
    {
        attacks.set_square(Square(r, f));
    }

    for (int8_t r = bishop_rank + 1, f = bishop_file - 1; r <= 6 && f >= 1;
         r++, f--)
    {
        attacks.set_square(Square(r, f));
    }

    for (int8_t r = bishop_rank - 1, f = bishop_file + 1; r >= 1 && f <= 6;
         r--, f++)
    {
        attacks.set_square(Square(r, f));
    }

    for (int8_t r = bishop_rank - 1, f = bishop_file - 1; r >= 1 && f >= 1;
         r--, f--)
    {
        attacks.set_square(Square(r, f));
    }

    return attacks;
}

// Given blockers, generate bishop attacks from a square with board edges.
constexpr Bitboard calculate_bishop_attacks(const Square   s,
                                            const Bitboard blockers)
{
    Bitboard attacks;

    const uint8_t bishop_rank = s.get_rank();
    const uint8_t bishop_file = s.get_file();

    for (int8_t r = bishop_rank + 1, f = bishop_file + 1; r <= 7 && f <= 7;
         r++, f++)
    {
        attacks.set_square(Square(r, f));
        if (blockers.get_board() & Square(r, f).get_mask())
        { // If a blocker exists on the square, stop the ray.
            break;
        }
    }

    for (int8_t r = bishop_rank + 1, f = bishop_file - 1; r <= 7 && f >= 0;
         r++, f--)
    {
        attacks.set_square(Square(r, f));
        if (blockers.get_board() & Square(r, f).get_mask()) { break; }
    }

    for (int8_t r = bishop_rank - 1, f = bishop_file + 1; r >= 0 && f <= 7;
         r--, f++)
    {
        attacks.set_square(Square(r, f));
        if (blockers.get_board() & Square(r, f).get_mask()) { break; }
    }

    for (int8_t r = bishop_rank - 1, f = bishop_file - 1; r >= 0 && f >= 0;
         r--, f--)
    {
        attacks.set_square(Square(r, f));
        if (blockers.get_board() & Square(r, f).get_mask()) { break; }
    }

    return attacks;
}

// Initialize bishop magic numbers for all 64 squares of the chessboard
// This function attempts to find "magic numbers" for each square
// that allow efficient indexing into precomputed bishop attack tables.
// Magic bitboards are a chess programming optimization that replaces
// slow ray-tracing with fast hash table lookups.
constexpr Magics_Array init_bishop_magics()
{
    Magics_Array output;

    constexpr uint64_t   PRNG_SEED = 177348;
    Psuedo_RNG<uint64_t> prng(PRNG_SEED);

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
            uint64_t magic = prng.generate_sparse_random();

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

template <auto square_idx>
constexpr auto create_bishop_attacks_array()
{
    // Create a Square object for the current index (0–63)
    // This represents the actual chessboard square
    constexpr Square s(square_idx);

    // Generate the bishop mask for this square.
    // The mask contains all potential sliding directions (diagonals)
    // but WITHOUT including edge squares.
    constexpr Bitboard mask = mask_bishop_attacks(s);

    // Count how many bits are set in the mask.
    // This tells us how many squares the bishop could theoretically
    // interact with when generating all possible occupancies.
    constexpr uint8_t num_of_high_bits_in_mask = mask.high_bit_count();

    constexpr Magic_Attacks_Info_Storage info {.mask = mask,
                                               .num_of_idx_bits =
                                                   num_of_high_bits_in_mask};

    // Compute the total number of occupancy variations for this mask.
    // Since each relevant square in the mask can either be empty (0) or
    // filled (1), there are 2^(num_of_high_bits_in_mask) possible blocker
    // configurations.
    constexpr uint64_t attacks_array_size = (1ULL << num_of_high_bits_in_mask);

    constexpr std::array<Bitboard, attacks_array_size> attacks = ([&]() -> std::array<Bitboard, attacks_array_size> {

    std::array<Bitboard, attacks_array_size> output{};

    constexpr uint64_t ATTACKS_INDEX_MINIMUM = 0;
    constexpr uint64_t ATTACK_INDEX_INCREMENT = 1;

    // Iterate over every possible occupancy configuration for the mask
    constexpr_for<ATTACKS_INDEX_MINIMUM, attacks_array_size, ATTACK_INDEX_INCREMENT>([&](auto idx)
    {
        // Generate an occupancy bitboard from the index.
        constexpr Bitboard occupancy =
            set_occupancy(idx, num_of_high_bits_in_mask, mask);

        // Multiply occupancy by magic to generate hash
        constexpr uint64_t hash = occupancy.get_board() * bishop_magics[square_idx];

        // Extract index bits: shift right to keep only the top
        // `num_of_high_bits_in_mask` bits. This is our hash index into the
        // attack table.
        constexpr uint64_t magic_index =
            hash >> ((sizeof(hash) << 3) - num_of_high_bits_in_mask);

        // Compute bishop attack bitboard for this square given the specific
        // occupancy. This simulates the bishop moving along diagonals and
        // being blocked by pieces where the occupancy bitboard has 1s.
        output[magic_index] = calculate_bishop_attacks(s, occupancy);
    });

    return output;

    })();

    return std::pair {attacks, info};
}

constexpr auto init_bishop_attack_hash_tables()
{
    return index_sequence_unpacker<64>(
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
    return index_sequence_unpacker<64>(
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

    inline static constexpr auto m_attack_hash_tables =
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
