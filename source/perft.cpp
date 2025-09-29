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

uint64_t divide_perft(Chess_Board& board, uint64_t depth) {
  Move_Generator mg(board);
  Chess_Move_List moves;
  mg.generate_all_moves(moves);

  uint64_t nodes = 0;

  for (const auto& move : moves) {
    Undo_Chess_Move undo = board.make_move(move);

    uint64_t child_nodes = perft(board, (depth - 1));
    nodes += child_nodes;

    std::string promotion_string = "";

    switch (move.promoted_piece) {
      case (PIECES::KNIGHT):
        promotion_string = "n";
        break;
      case (PIECES::BISHOP):
        promotion_string = "b";
        break;
      case (PIECES::ROOK):
        promotion_string = "r";
        break;
      case (PIECES::QUEEN):
        promotion_string = "q";
        break;
      default:
        promotion_string = "";
    }

    std::cout << SQUARE_STRINGS[move.source_square]
              << SQUARE_STRINGS[move.destination_square] << promotion_string
              << " - " << child_nodes << std::endl;

    board.undo_move(undo);
  }

  std::cout << "Total nodes: " << nodes << std::endl;

  return nodes;
}