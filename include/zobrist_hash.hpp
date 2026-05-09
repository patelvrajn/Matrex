#pragma once

#include <array>
#include <cstdint>

#include "globals.hpp"
#include "psuedo_random_number_generator.hpp"
#include "square.hpp"

struct Zobrist_Hash_Keys
{
    std::array<std::array<std::array<uint64_t, NUM_OF_SQUARES_ON_CHESS_BOARD>,
                          NUM_OF_UNIQUE_PIECES_PER_PLAYER>,
               NUM_OF_PLAYERS>
                                                              pieces;
    std::array<uint64_t, (NUM_OF_SQUARES_ON_CHESS_BOARD + 1)> en_passant;
    std::array<uint64_t, 16>                                  castling_rights;
    uint64_t                                                  black_to_move;
};

consteval Zobrist_Hash_Keys initialize_zobrist_hash_keys()
{
    Zobrist_Hash_Keys    keys;
    Psuedo_RNG<uint64_t> prng;

    for (uint8_t player = 0; player < NUM_OF_PLAYERS; player++)
    {
        for (uint8_t piece = 0; piece < NUM_OF_UNIQUE_PIECES_PER_PLAYER;
             piece++)
        {
            for (uint8_t square = 0; square < NUM_OF_SQUARES_ON_CHESS_BOARD;
                 square++)
            {
                keys.pieces[player][piece][square] = prng.generate_random();
            }
        }
    }

    for (uint8_t square = 0; square < (NUM_OF_SQUARES_ON_CHESS_BOARD + 1);
         square++)
    {
        keys.en_passant[square] = prng.generate_random();
    }

    for (uint8_t cr = 0; cr < 16; cr++)
    {
        keys.castling_rights[cr] = prng.generate_random();
    }

    keys.black_to_move = prng.generate_random();

    return keys;
}

class Zobrist_Hash
{
  public:

    constexpr Zobrist_Hash();

    constexpr uint64_t get_hash_value() const;

    constexpr void update_piece(const PIECE_COLOR color,
                                const PIECES      piece,
                                const Square      square);
    constexpr void update_en_passant_square(const Square s);
    constexpr void update_castling_rights(const uint8_t new_rights);
    constexpr void flip_side_to_move();

    constexpr inline auto operator<=>(const Zobrist_Hash& other) const;

  private:

    uint64_t m_hash_value;

    inline static constexpr Zobrist_Hash_Keys m_hash_keys =
        initialize_zobrist_hash_keys();
};

constexpr inline auto Zobrist_Hash::operator<=>(const Zobrist_Hash& other) const
{
    return (get_hash_value() <=> other.get_hash_value());
}

constexpr Zobrist_Hash::Zobrist_Hash() : m_hash_value(0) {}

constexpr uint64_t Zobrist_Hash::get_hash_value() const
{
    return m_hash_value;
};

constexpr void Zobrist_Hash::update_piece(const PIECE_COLOR color,
                                          const PIECES      piece,
                                          const Square      square)
{
    m_hash_value ^= m_hash_keys.pieces[color][piece][square.get_index()];
}

constexpr void Zobrist_Hash::update_en_passant_square(const Square s)
{
    m_hash_value ^= m_hash_keys.en_passant[s.get_index()];
}

constexpr void Zobrist_Hash::update_castling_rights(const uint8_t new_rights)
{
    m_hash_value ^= m_hash_keys.castling_rights[new_rights];
}

constexpr void Zobrist_Hash::flip_side_to_move()
{
    m_hash_value ^= m_hash_keys.black_to_move;
}
