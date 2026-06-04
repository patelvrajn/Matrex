#include "bitboard.hpp"

#include <bit>
#include <iostream>

#include "globals.hpp"

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
