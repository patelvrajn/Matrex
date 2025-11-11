#pragma once

#include "chess_board.hpp"
#include "chess_move.hpp"
#include "move_generator.hpp"

constexpr uint64_t MVV_LVA_ATTACKER_VALUES[] = {10, 20, 30, 40, 50, 60};

// Attacker: Pawn, Knight, Bishop, Rook, Queen, King
// Victims: Pawn, Knight, Bishop, Rook, Queen
// mvv_lva_array[attacker][victim]
typedef std::array<std::array<uint64_t, (PIECES::QUEEN + 1)>,
                   (PIECES::KING + 1)>
    mvv_lva_array;

class Move_Ordering {
 public:
  Move_Ordering(const Chess_Board& cb);
  template <MOVE_GENERATION_TYPE move_gen_type>
  void generate_moves();
  Chess_Move_List& get_sorted_moves();
  bool is_side_to_move_in_check() const;

 private:
  const Chess_Board& m_chess_board;
  Chess_Move_List m_move_list;
  static mvv_lva_array m_mvv_lva_array;

  static mvv_lva_array generate_mvv_lva_array();
  void mvv_lva_scorer();
};

template <MOVE_GENERATION_TYPE move_gen_type>
void Move_Ordering::generate_moves() {
  Move_Generator mg(m_chess_board);
  mg.generate_all_moves<move_gen_type>(m_move_list);
}
