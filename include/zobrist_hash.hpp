#pragma once

#include <array>
#include <cstdint>

#include "globals.hpp"
#include "psuedo_random_number_generator.hpp"
#include "square.hpp"

using Zobrist_Hash_Storage_Type = uint64_t;

struct Zobrist_Hash_Keys
{
    std::array<std::array<std::array<Zobrist_Hash_Storage_Type,
                                     NUM_OF_SQUARES_ON_CHESS_BOARD>,
                          NUM_OF_UNIQUE_PIECES_PER_PLAYER>,
               NUM_OF_PLAYERS>
        pieces;
    std::array<Zobrist_Hash_Storage_Type, (NUM_OF_SQUARES_ON_CHESS_BOARD + 1)>
                                              en_passant;
    std::array<Zobrist_Hash_Storage_Type, 16> castling_rights;
    Zobrist_Hash_Storage_Type                 black_to_move;
};

struct Correction_History_Hashes
{
    Zobrist_Hash_Storage_Type pawns_key       = 0;
    Zobrist_Hash_Storage_Type diagonals_key   = 0;
    Zobrist_Hash_Storage_Type orthogonals_key = 0;
    Zobrist_Hash_Storage_Type knights_key     = 0;
    Zobrist_Hash_Storage_Type material_key    = 0;
};

constexpr Zobrist_Hash_Keys initialize_zobrist_hash_keys()
{
    Zobrist_Hash_Keys                     keys;
    Psuedo_RNG<Zobrist_Hash_Storage_Type> prng;

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

    explicit constexpr Zobrist_Hash(Zobrist_Hash_Storage_Type h);

    constexpr Zobrist_Hash_Storage_Type        get_hash_value() const;
    constexpr const Correction_History_Hashes& get_corr_hist_hashes() const;

    constexpr void update_piece(const PIECE_COLOR color,
                                const PIECES      piece,
                                const Square      square);
    constexpr void update_en_passant_square(const Square s);
    constexpr void update_castling_rights(const uint8_t new_rights);
    constexpr void flip_side_to_move();

    constexpr inline auto operator<=>(const Zobrist_Hash& other) const;
    constexpr bool        operator==(const Zobrist_Hash& other) const;

    constexpr inline Zobrist_Hash  operator^(const Zobrist_Hash& other) const;
    constexpr inline Zobrist_Hash& operator^=(const Zobrist_Hash& other);

  private:

    Zobrist_Hash_Storage_Type m_hash_value;

    Correction_History_Hashes m_corr_hist_hashes;

    inline static constexpr Zobrist_Hash_Keys m_hash_keys =
        initialize_zobrist_hash_keys();
};

constexpr inline auto Zobrist_Hash::operator<=>(const Zobrist_Hash& other) const
{
    return (get_hash_value() <=> other.get_hash_value());
}

constexpr bool Zobrist_Hash::operator==(const Zobrist_Hash& other) const
{
    return (get_hash_value() == other.get_hash_value());
}

constexpr inline Zobrist_Hash
Zobrist_Hash::operator^(const Zobrist_Hash& other) const
{
    return Zobrist_Hash(m_hash_value ^ other.m_hash_value);
}

constexpr inline Zobrist_Hash&
Zobrist_Hash::operator^=(const Zobrist_Hash& other)
{
    m_hash_value ^= other.m_hash_value;

    return (*this);
}

constexpr Zobrist_Hash::Zobrist_Hash() :
    m_hash_value(0), m_corr_hist_hashes(Correction_History_Hashes())
{
}

constexpr Zobrist_Hash::Zobrist_Hash(Zobrist_Hash_Storage_Type h) :
    m_hash_value(h)
{
}

constexpr Zobrist_Hash_Storage_Type Zobrist_Hash::get_hash_value() const
{
    return m_hash_value;
};

constexpr const Correction_History_Hashes&
Zobrist_Hash::get_corr_hist_hashes() const
{
    return m_corr_hist_hashes;
}

constexpr void Zobrist_Hash::update_piece(const PIECE_COLOR color,
                                          const PIECES      piece,
                                          const Square      square)
{
    if (piece == PIECES::PAWN)
    {
        m_corr_hist_hashes.pawns_key ^=
            m_hash_keys.pieces[color][piece][square.get_index()];
    }

    if ((piece == PIECES::BISHOP) || (piece == PIECES::QUEEN))
    {
        m_corr_hist_hashes.diagonals_key ^=
            m_hash_keys.pieces[color][piece][square.get_index()];
    }

    if ((piece == PIECES::ROOK) || (piece == PIECES::QUEEN))
    {
        m_corr_hist_hashes.orthogonals_key ^=
            m_hash_keys.pieces[color][piece][square.get_index()];
    }

    if (piece == PIECES::KNIGHT)
    {
        m_corr_hist_hashes.knights_key ^=
            m_hash_keys.pieces[color][piece][square.get_index()];
    }

    m_corr_hist_hashes.material_key ^=
        m_hash_keys.pieces[color][piece][square.get_index()];

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
