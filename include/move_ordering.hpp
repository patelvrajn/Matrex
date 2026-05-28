#pragma once

#include "chess_board.hpp"
#include "chess_move.hpp"
#include "move_generator.hpp"
#include "static_exchange_evaluation.hpp"

constexpr Move_Score MVV_LVA_ATTACKER_VALUES[] = {10, 20, 30, 40, 50, 60};

// Attacker: Pawn, Knight, Bishop, Rook, Queen, King
// Victims: Pawn, Knight, Bishop, Rook, Queen
// mvv_lva_array[attacker][victim]
typedef std::array<std::array<Move_Score, (PIECES::QUEEN + 1)>,
                   (PIECES::KING + 1)>
    mvv_lva_array;

class Move_Ordering
{
  public:

    Move_Ordering(const Chess_Board& c, const Chess_Move& hash_move);
    Move_Generation_List&  get_sorted_moves();
    Moves_Bitboard_Matrix& get_moves_matrix();
    bool                   is_side_to_move_in_check() const;

    template <MOVE_GENERATION_TYPE move_gen_type>
    void generate_moves();

  private:

    const Chess_Board&                    m_chess_board;
    Move_Generation_List                  m_move_list;
    Moves_Bitboard_Matrix                 m_moves_matrix;
    static mvv_lva_array                  m_mvv_lva_array;
    Chess_Move                            m_hash_move;
    Static_Exchange_Evaluator<Move_Score> m_see;

    static mvv_lva_array generate_mvv_lva_array();
    void                 move_scorer();
};

template <MOVE_GENERATION_TYPE move_gen_type>
void Move_Ordering::generate_moves()
{
    if constexpr (move_gen_type == MOVE_GENERATION_TYPE::ALL)
    {
        Move_Generator mg(m_chess_board);
        mg.generate_all_moves<move_gen_type>(m_move_list, m_moves_matrix);
    }
    else
    {
        Move_Generation_List  not_used_moves_list;
        Moves_Bitboard_Matrix not_used_moves_matrix;

        Move_Generator typed_mg(m_chess_board);
        typed_mg.generate_all_moves<move_gen_type>(m_move_list,
                                                   not_used_moves_matrix);

        Move_Generator all_mg(m_chess_board);
        all_mg.generate_all_moves<MOVE_GENERATION_TYPE::ALL>(
            not_used_moves_list,
            m_moves_matrix);
    }
}
