#pragma once

#include <stdint.h>

#include <string>

constexpr std::string_view ENGINE_NAME = "Matrex";
constexpr std::string_view ENGINE_VERSION = "0.0.1";

constexpr uint8_t NUM_OF_PLAYERS = 2;

constexpr uint8_t NUM_OF_RANKS_ON_CHESS_BOARD = 8;
constexpr uint8_t NUM_OF_FILES_ON_CHESS_BOARD = 8;
constexpr uint8_t NUM_OF_SQUARES_ON_CHESS_BOARD =
    NUM_OF_RANKS_ON_CHESS_BOARD * NUM_OF_FILES_ON_CHESS_BOARD;

constexpr uint8_t NUM_OF_PIECES_PER_PLAYER = 16;
constexpr uint8_t NUM_OF_UNIQUE_PIECES_PER_PLAYER = 6;

enum PIECE_COLOR { WHITE, BLACK, NO_COLOR };

enum PIECES { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NO_PIECE };

constexpr std::string PIECE_STRINGS[] = {"PAWN",  "KNIGHT", "BISHOP",  "ROOK",
                                         "QUEEN", "KING",   "NO_PIECE"};

constexpr std::string
    UNICODE_PIECES[NUM_OF_PLAYERS * NUM_OF_UNIQUE_PIECES_PER_PLAYER] = {
        "♙", "♘", "♗", "♖", "♕", "♔", "♟︎", "♞", "♝", "♜", "♛", "♚"};

constexpr std::string_view START_POSITION_FEN =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
