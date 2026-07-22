#pragma once

#include "bitboard.hpp"
#include "globals.hpp"

constexpr Bitboard NOT_A_FILE(18374403900871474942ULL);
constexpr Bitboard NOT_H_FILE(9187201950435737471ULL);
constexpr Bitboard NOT_AB_FILES(18229723555195321596ULL);
constexpr Bitboard NOT_GH_FILES(4557430888798830399ULL);

// Function that generates all possible pawn attack moves (not forward pushes!)
// for a pawn of a given color on a given square.
constexpr Bitboard mask_pawn_attacks(const PIECE_COLOR c, const Square s)
{
    Bitboard attacks;
    Bitboard pawn_bb;

    // Set the pawn's location.
    pawn_bb.set_square(s);

    if (c == PIECE_COLOR::WHITE)
    {
        // For white pawns, attacks are diagonally forward-left and
        // forward-right.
        const Bitboard RIGHT_ATTACK =
            (pawn_bb >> (NUM_OF_FILES_ON_CHESS_BOARD - 1));
        const Bitboard LEFT_ATTACK =
            (pawn_bb >> (NUM_OF_FILES_ON_CHESS_BOARD + 1));

        // Prevent wrap-around of the right attack.
        if ((RIGHT_ATTACK & NOT_A_FILE).get_board())
        {
            attacks |= RIGHT_ATTACK;
        }

        // Prevent wrap-around of the left attack.
        if ((LEFT_ATTACK & NOT_H_FILE).get_board()) { attacks |= LEFT_ATTACK; }
    }
    // Black pawn attacks
    else
    {
        // For black pawns, their diagonals go "down-left" and "down-right."
        // That corresponds to left shifts instead of right shifts.
        const Bitboard LEFT_ATTACK =
            (pawn_bb << (NUM_OF_FILES_ON_CHESS_BOARD - 1));
        const Bitboard RIGHT_ATTACK =
            (pawn_bb << (NUM_OF_FILES_ON_CHESS_BOARD + 1));

        if ((LEFT_ATTACK & NOT_H_FILE).get_board()) { attacks |= LEFT_ATTACK; }

        if ((RIGHT_ATTACK & NOT_A_FILE).get_board())
        {
            attacks |= RIGHT_ATTACK;
        }
    }

    return attacks;
};

// Function that generates all possible knight attack moves for a knight on a
// given.
constexpr Bitboard mask_knight_attacks(const Square s)
{
    Bitboard attacks;
    Bitboard knight_bb;

    // Set the knight's location.
    knight_bb.set_square(s);

    // NNW: two steps north and one step west.
    const Bitboard NNW_ATTACK = (knight_bb >> 17);

    // Prevent wrap around.
    if ((NNW_ATTACK & NOT_H_FILE).get_board()) { attacks |= NNW_ATTACK; }

    // NNE: two steps north and one step east.
    const Bitboard NNE_ATTACK = (knight_bb >> 15);

    // Prevent wrap around.
    if ((NNE_ATTACK & NOT_A_FILE).get_board()) { attacks |= NNE_ATTACK; }

    // NWW: one step north and two steps west.
    const Bitboard NWW_ATTACK = (knight_bb >> 10);

    // Prevent wrap around.
    if ((NWW_ATTACK & NOT_GH_FILES).get_board()) { attacks |= NWW_ATTACK; }

    // NEE: one step north and two steps east.
    const Bitboard NEE_ATTACK = (knight_bb >> 6);

    // Prevent wrap around.
    if ((NEE_ATTACK & NOT_AB_FILES).get_board()) { attacks |= NEE_ATTACK; }

    // SSE: two steps south and one step east.
    const Bitboard SSE_ATTACK = (knight_bb << 17);

    // Prevent wrap around.
    if ((SSE_ATTACK & NOT_A_FILE).get_board()) { attacks |= SSE_ATTACK; }

    // SSW: two steps south and one step west
    const Bitboard SSW_ATTACK = (knight_bb << 15);

    // Prevent wrap around.
    if ((SSW_ATTACK & NOT_H_FILE).get_board()) { attacks |= SSW_ATTACK; }

    // SEE: one step south and two steps east
    const Bitboard SEE_ATTACK = (knight_bb << 10);

    // Prevent wrap around.
    if ((SEE_ATTACK & NOT_AB_FILES).get_board()) { attacks |= SEE_ATTACK; }

    // SWW: one step south and two steps west
    const Bitboard SWW_ATTACK = (knight_bb << 6);

    // Prevent wrap around.
    if ((SWW_ATTACK & NOT_GH_FILES).get_board()) { attacks |= SWW_ATTACK; }

    return attacks;
}

// Function that generates all possible king attack moves for a king on a given
// square, using bitboards.
constexpr Bitboard mask_king_attacks(const Square s)
{
    Bitboard attacks;
    Bitboard king_bb;

    // Set the king's location.
    king_bb.set_square(s);

    // N: one step north
    const Bitboard N_ATTACK  = (king_bb >> 8);
    attacks                 |= N_ATTACK;

    // NW: one step north and one step west.
    const Bitboard NW_ATTACK = (king_bb >> 9);

    // Prevent wrap around.
    if ((NW_ATTACK & NOT_H_FILE).get_board()) { attacks |= NW_ATTACK; }

    // NE: one step north and one step east.
    const Bitboard NE_ATTACK = (king_bb >> 7);

    // Prevent wrap around.
    if ((NE_ATTACK & NOT_A_FILE).get_board()) { attacks |= NE_ATTACK; }

    // W: one step west
    const Bitboard W_ATTACK = (king_bb >> 1);

    // Prevent wrap around.
    if ((W_ATTACK & NOT_H_FILE).get_board()) { attacks |= W_ATTACK; }

    // S: one step south
    const Bitboard S_ATTACK  = (king_bb << 8);
    attacks                 |= S_ATTACK;

    // SE: one step south and one step east.
    const Bitboard SE_ATTACK = (king_bb << 9);

    // Prevent wrap around.
    if ((SE_ATTACK & NOT_A_FILE).get_board()) { attacks |= SE_ATTACK; }

    // SW: one step south and one step west.
    const Bitboard SW_ATTACK = (king_bb << 7);
    if ((SW_ATTACK & NOT_H_FILE).get_board()) { attacks |= SW_ATTACK; }

    // E: one step east
    const Bitboard E_ATTACK = (king_bb << 1);

    // Prevent wrap around.
    if ((E_ATTACK & NOT_A_FILE).get_board()) { attacks |= E_ATTACK; }

    return attacks;
}
