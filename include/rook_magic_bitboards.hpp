#pragma once

#include <array>

#include "globals.hpp"
#include "magic_hash_table.hpp"
#include "occupancy.hpp"

// Generate rook attack mask for a square (without board edges)
constexpr Bitboard mask_rook_attacks(const Square s)
{
    Bitboard attacks;

    const uint8_t rook_rank = s.get_rank();
    const uint8_t rook_file = s.get_file();

    for (int8_t r = rook_rank + 1; r <= 6; r++)
    {
        attacks.set_square(Square(r, rook_file));
    }
    for (int8_t r = rook_rank - 1; r >= 1; r--)
    {
        attacks.set_square(Square(r, rook_file));
    }
    for (int8_t f = rook_file + 1; f <= 6; f++)
    {
        attacks.set_square(Square(rook_rank, f));
    }
    for (int8_t f = rook_file - 1; f >= 1; f--)
    {
        attacks.set_square(Square(rook_rank, f));
    }

    return attacks;
}

// Given blockers, generate rook attacks from a square with board edges.
constexpr Bitboard calculate_rook_attacks(const Square   s,
                                          const Bitboard blockers)
{
    Bitboard attacks;

    const uint8_t rook_rank = s.get_rank();
    const uint8_t rook_file = s.get_file();

    for (int8_t r = rook_rank + 1; r <= 7; r++)
    {
        attacks.set_square(Square(r, rook_file));
        if (blockers.get_board() & Square(r, rook_file).get_mask())
        { // If a blocker exists on the square, stop the ray.
            break;
        }
    }
    for (int8_t r = rook_rank - 1; r >= 0; r--)
    {
        attacks.set_square(Square(r, rook_file));
        if (blockers.get_board() & Square(r, rook_file).get_mask()) { break; }
    }
    for (int8_t f = rook_file + 1; f <= 7; f++)
    {
        attacks.set_square(Square(rook_rank, f));
        if (blockers.get_board() & Square(rook_rank, f).get_mask()) { break; }
    }
    for (int8_t f = rook_file - 1; f >= 0; f--)
    {
        attacks.set_square(Square(rook_rank, f));
        if (blockers.get_board() & Square(rook_rank, f).get_mask()) { break; }
    }

    return attacks;
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

template <auto square_idx>
constexpr auto create_rook_attacks_array()
{
    // Create a Square object for the current index (0–63)
    // This represents the actual chessboard square
    constexpr Square s(square_idx);

    // Generate the rook mask for this square.
    // The mask contains all potential sliding directions (diagonals)
    // but WITHOUT including edge squares.
    constexpr Bitboard mask = mask_rook_attacks(s);

    // Count how many bits are set in the mask.
    // This tells us how many squares the rook could theoretically
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
        constexpr uint64_t hash = occupancy.get_board() * rook_magics[square_idx];

        // Extract index bits: shift right to keep only the top
        // `num_of_high_bits_in_mask` bits. This is our hash index into the
        // attack table.
        constexpr uint64_t magic_index =
            hash >> ((sizeof(hash) << 3) - num_of_high_bits_in_mask);

        // Compute rook attack bitboard for this square given the specific
        // occupancy. This simulates the rook moving along diagonals and
        // being blocked by pieces where the occupancy bitboard has 1s.
        output[magic_index] = calculate_rook_attacks(s, occupancy);
    });

    return output;

    })();

    return std::pair {attacks, info};
}

consteval auto init_rook_attack_hash_tables()
{
    return index_sequence_unpacker<64>(
        []<std::size_t... square_index>()
        {
            return Compile_Time_Jagged_Array<
                Bitboard,
                decltype(create_rook_attacks_array<square_index>().first)...>(
                create_rook_attacks_array<square_index>().first...);
        });
}

consteval auto init_rook_attack_hash_tables_infos()
{
    return index_sequence_unpacker<64>(
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

    inline static constexpr auto m_attack_hash_tables =
        Magic_Hash_Jagged_Table<rook_magics,
                                init_rook_attack_hash_tables,
                                init_rook_attack_hash_tables_infos>();

    Magics_Array init_magics();
};

constexpr Rook_Magic_Bitboards::Rook_Magic_Bitboards() {}

constexpr Bitboard
Rook_Magic_Bitboards::get_attacks(const Square   s,
                                  const Bitboard occupancy) const
{
    return m_attack_hash_tables.get_attacks(s, occupancy);
}
