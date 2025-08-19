#pragma once

#include <stdint.h>

constexpr uint8_t NUM_OF_RANKS_ON_CHESS_BOARD = 8;
constexpr uint8_t NUM_OF_FILES_ON_CHESS_BOARD = 8;
constexpr uint8_t NUM_OF_SQUARES_ON_CHESS_BOARD =
    NUM_OF_RANKS_ON_CHESS_BOARD * NUM_OF_FILES_ON_CHESS_BOARD;

enum PIECE_COLOR { WHITE, BLACK };
