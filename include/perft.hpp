#pragma once

#include "chess_board.hpp"

uint64_t perft(Chess_Board& board, uint64_t depth);

uint64_t divide_perft(Chess_Board& board, uint64_t depth);
