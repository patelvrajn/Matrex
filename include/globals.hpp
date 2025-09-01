#pragma once

#include <stdint.h>

#include <string>

constexpr uint8_t NUM_OF_PLAYERS = 2;

constexpr uint8_t NUM_OF_RANKS_ON_CHESS_BOARD = 8;
constexpr uint8_t NUM_OF_FILES_ON_CHESS_BOARD = 8;
constexpr uint8_t NUM_OF_SQUARES_ON_CHESS_BOARD =
    NUM_OF_RANKS_ON_CHESS_BOARD * NUM_OF_FILES_ON_CHESS_BOARD;

constexpr uint8_t NUM_OF_UNIQUE_PIECES_PER_PLAYER = 6;

enum PIECE_COLOR { WHITE, BLACK, NO_COLOR };

enum PIECES { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NO_PIECE };

constexpr std::string PIECE_STRINGS[] = {"PAWN",  "KNIGHT", "BISHOP",  "ROOK",
                                         "QUEEN", "KING",   "NO_PIECE"};

constexpr std::string
    UNICODE_PIECES[NUM_OF_PLAYERS * NUM_OF_UNIQUE_PIECES_PER_PLAYER] = {
        "♙", "♘", "♗", "♖", "♕", "♔", "♟︎", "♞", "♝", "♜", "♛", "♚"};
