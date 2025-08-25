#pragma once

#include "bitboard.hpp"

Bitboard get_queen_attacks(const Square& s, const Bitboard& occupancy);
