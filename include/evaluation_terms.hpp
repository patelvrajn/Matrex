#include "evaluate.hpp"
#include "globals.hpp"

constexpr multi_array<uint16_t, (NUM_OF_UNIQUE_PIECES_PER_PLAYER - 1)>
    material_weights = {100, 300, 350, 500, 900};

constexpr uint16_t diagonal_mobility_weight = 10;
constexpr uint16_t orthogonal_mobility_weight = 10;
constexpr uint16_t knight_movement_mobility_weight = 15;
constexpr uint16_t multi_movement_mobility_weight = 5;
constexpr uint16_t backwards_movement_mobility_weight = 2;

const Evaluation_Weights TUNED_EVALUATION_WEIGHTS =
    Evaluation_Weights(  // Cannot be constexpr due to Reference_Array.
        material_weights, diagonal_mobility_weight, orthogonal_mobility_weight,
        knight_movement_mobility_weight, multi_movement_mobility_weight,
        backwards_movement_mobility_weight);
