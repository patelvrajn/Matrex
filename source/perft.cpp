#include "perft.hpp"

#include <cassert>
#include <vector>

#include "move_generator.hpp"

uint64_t perft(Chess_Board& board, uint64_t depth) {
  Move_Generator mg(board);
  Chess_Move_List moves;
  uint64_t nodes = 0;

  if (depth == 0) {
    return 1ULL;
  }

  mg.generate_all_moves(moves);
  for (const auto& move : moves) {
    Undo_Chess_Move undo = board.make_move(move);

    nodes += perft(board, (depth - 1));

    board.undo_move(undo);
  }

  return nodes;
}

uint64_t divide_perft(Chess_Board& board, uint64_t depth, bool is_frc) {
  Move_Generator mg(board);
  Chess_Move_List moves;
  mg.generate_all_moves(moves);

  uint64_t nodes = 0;

  for (const auto& move : moves) {
    Undo_Chess_Move undo = board.make_move(move);

    uint64_t child_nodes = perft(board, (depth - 1));
    nodes += child_nodes;

    std::cout << move.to_coordinate_notation(is_frc) << " - " << child_nodes
              << std::endl;

    board.undo_move(undo);
  }

  std::cout << "Total nodes: " << nodes << std::endl;

  return nodes;
}