#include "search.hpp"

#include <random>

#include "move_generator.hpp"

Chess_Move random_legal_move(const Chess_Board& board) {
  Move_Generator mg(board);
  Chess_Move_List moves;
  mg.generate_all_moves(moves);

  std::mt19937_64 rng(std::random_device{}());
  std::uniform_int_distribution<uint16_t> dist(0, (moves.get_max_index() - 1));

  const uint16_t move_index = dist(rng);

  uint16_t move_count = 0;

  for (const auto& move : moves) {
    if (move_count == move_index) {
      return move;
    }
    move_count++;
  }

  return Chess_Move();
}
