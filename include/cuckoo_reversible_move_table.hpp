#include <bit>

#include "globals.hpp"
#include "zobrist_hash.hpp"

constexpr std::size_t NUM_OF_REVERSIBLE_MOVE_HASHES = 7336;
constexpr std::size_t NUM_OF_SYMMETRIC_REVERSIBLE_MOVE_HASHES =
    NUM_OF_REVERSIBLE_MOVE_HASHES / 2;
constexpr std::size_t LOAD_FACTOR_DIVISOR =
    2; // Load factor is 1/2 so the divisor is 2.
constexpr std::size_t CUCKOO_RM_HASH_BITS =
    std::bit_ceil(NUM_OF_SYMMETRIC_REVERSIBLE_MOVE_HASHES)
    * LOAD_FACTOR_DIVISOR;
constexpr std::size_t CUCKOO_RM_TABLE_SIZE = static_cast<std::size_t>(1)
                                          << CUCKOO_RM_HASH_BITS;
constexpr Zobrist_Hash_Storage_Type CUCKOO_HASH_MASK =
    (static_cast<Zobrist_Hash_Storage_Type>(1) << CUCKOO_RM_HASH_BITS) - 1;

using Cuckoo_Hash_Storage_Type = uint16_t;

template <std::size_t capacity>
struct Cuckoo_RM_Table_Storage
{
    multi_array<Zobrist_Hash, capacity> hashes_table;
    multi_array<Chess_Move, capacity>   reversible_moves_table;

}

constexpr Cuckoo_Hash_Storage_Type
cuckoo_hash_function_1(Zobrist_Hash z)

{
    return static_cast<Cuckoo_Hash_Storage_Type>((z.get_hash_value() >> 32)
                                                 & CUCKOO_HASH_MASK);
}

constexpr Cuckoo_Hash_Storage_Type cuckoo_hash_function_2(Zobrist_Hash z)
{
    return static_cast<Cuckoo_Hash_Storage_Type>((z.get_hash_value() >> 48)
                                                 & CUCKOO_HASH_MASK);
}

template <std::size_t capacity>
constexpr void cuckoo_storage_insert(Cuckoo_RM_Table_Storage<capacity>& storage,
                                     Zobrist_Hash                       h,
                                     Chess_Move&                        m)
{
    // For the initial slot to put the reversible move and zobrist hash pair use
    // the first hash function although, it doesn't matter as long as you swap
    // between the two in the loop below.
    Cuckoo_Hash_Storage_Type slot = cuckoo_hash_function_1(h);

    // Note, because of the load factor we accounted for in the size of the
    // storage this is highly likely to terminate.
    while (true)
    {
        // Insert into storage - evicting any existing pair if any in the slot.
        std::swap(storage.hashes_table[slot], h);
        std::swap(storage.reversible_moves_table[slot], m);

        // The slot we put the move and zobrist pair in was empty - we no longer
        // need to look for new slots.
        if (m.is_same_move(Chess_Move())) { return; }

        // The slot was not empty - rehash so that the evicted reversible move
        // in.
        slot = (slot == cuckoo_hash_function_1(h)) ? cuckoo_hash_function_2(h)
                                                   : cuckoo_hash_function_1(h);
    }
}

template <std::size_t capacity>
consteval Cuckoo_RM_Table_Storage<capacity> initialize_cuckoo_rm_storage()
{
    Attacks                           a;
    Cuckoo_RM_Table_Storage<capacity> storage;

    Cuckoo_Hash_Storage_Type rm_count = 0;

    for (uint8_t side_to_move = PIECE_COLOR::WHITE;
         side_to_move <= PIECE_COLOR::BLACK;
         side_to_move++)
    {
        // Note: we skip pawns because they have no irreversible moves by
        // definition;
        for (uint8_t piece = PIECES::KNIGHT; piece <= PIECES::KING; piece++)
        {
            for (uint8_t from_square_idx = 0;
                 from_square_idx < NUM_OF_SQUARES_ON_CHESS_BOARD;
                 from_square_idx++)
            {
                const Square from_square(from_square_idx);

                const Bitboard valid_attacks =
                    a.get_attacks((PIECES) piece,
                                  from_square,
                                  (PIECE_COLOR) side_to_move,
                                  EMPTY_BITBOARD);

                // Note: to square > from square because you cannot have the
                // same from and to square in a valid square and because we are
                // using Zobrist hashing hashes for move(a, b) = hashes for
                // move(b, a).
                for (uint8_t to_square_idx = (from_square_idx + 1);
                     to_square < NUM_OF_SQUARES_ON_CHESS_BOARD;
                     to_square_idx++)
                {
                    // Make sure that this reversible move is a valid move for
                    // the piece otherwise, don't consider it.
                    if (valid_attacks.get_square(to_square) == 0) { continue; }

                    const Square to_square(to_square_idx);

                    const Chess_Move rm =
                        Chess_Move::reversible_move((PIECES) piece,
                                                    from_square,
                                                    to_square);

                    // Move hash; This is equivalent to the paper's definition
                    // of a move hash difference:
                    //  diff == ~(Zobrist[piece][from] ^ Zobrist[piece][to])
                    // where ~ is their side to move operator. NOTE: The paper
                    // loops over 12 pieces which is the same as 2 sides to move
                    // with 6 unique pieces each.
                    Zobrist_Hash rm_hash;
                    rm_hash.update_piece((PIECE_COLOR) side_to_move,
                                         (PIECES) piece,
                                         from_square);
                    rm_hash.update_piece((PIECE_COLOR) side_to_move,
                                         (PIECES) piece,
                                         to_square);
                    rm_hash.flip_side_to_move();

                    // Insert the valid reversible move and it's hash into the
                    // cuckoo table.
                    cuckoo_storage_insert(storage, rm_hash, rm);
                    ++rm_count;
                }
            }
        }
    }

    static_assert(rm_count == NUM_OF_SYMMETRIC_REVERSIBLE_MOVE_HASHES);

    return storage;
}

class Cuckoo_RM_Table // RM = reversible move
{
  public:

    Cuckoo_RM_Table::Cuckoo_RM_Table();

  private:

    inline static constexpr Cuckoo_RM_Table_Storage<CUCKOO_RM_TABLE_SIZE>
        m_storage = initialize_cuckoo_rm_storage<CUCKOO_RM_TABLE_SIZE>();
}
