#pragma once

#include "bitboard.hpp"

Bitboard set_occupancy(uint64_t index, uint8_t num_of_high_bits_in_mask,
                       const Bitboard& mask);
