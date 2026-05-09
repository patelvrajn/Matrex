#include "bitboard.hpp"

#include <bit>
#include <iostream>

#include "globals.hpp"

Bitboard::Iterator::Iterator(uint64_t board) : m_board(board) {}

Square Bitboard::Iterator::operator*() const
{
    return Square(__builtin_ctzll(m_board));
}

Bitboard::Iterator& Bitboard::Iterator::operator++()
{
    m_board &= (m_board - 1);
    return *this;
}

bool Bitboard::Iterator::operator==(const Iterator& other) const
{
    return (other.m_board == m_board);
}

bool Bitboard::Iterator::operator!=(const Iterator& other) const
{
    return (other.m_board != m_board);
}

Bitboard::Iterator Bitboard::begin() const
{
    return Bitboard::Iterator(m_board);
}

Bitboard::Iterator Bitboard::end() const { return Bitboard::Iterator(0ULL); }

void Bitboard::pretty_print() const
{
    std::cout << "Bitboard: " << m_board << " (0x" << std::hex << m_board << ")"
              << std::endl;

    for (uint8_t rank = 0; rank < NUM_OF_RANKS_ON_CHESS_BOARD; rank++)
    {
        // Print the ranks on the left hand side of the board before the first
        // file.
        std::cout << (NUM_OF_RANKS_ON_CHESS_BOARD - rank) << "   ";

        for (uint8_t file = 0; file < NUM_OF_FILES_ON_CHESS_BOARD; file++)
        {
            Square s(rank, file);

            if (get_square(s)) { std::cout << 1 << " "; }
            else { std::cout << 0 << " "; }
        }

        std::cout << std::endl;
    }

    std::cout << std::endl;
    std::cout << "    ";

    for (uint8_t file = 0; file < NUM_OF_FILES_ON_CHESS_BOARD; file++)
    {
        char file_char = 'A' + file;
        std::cout << file_char << " ";
    }

    std::cout << std::endl;
}
