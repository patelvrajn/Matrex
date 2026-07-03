#pragma once

#include "chess_board.hpp"
#include "chess_move.hpp"
#include "move_generator.hpp"
#include "static_exchange_evaluation.hpp"
#include "history.hpp"

constexpr Move_Score MVV_LVA_ATTACKER_VALUES[] = {10, 20, 30, 40, 50, 60};

// Attacker: Pawn, Knight, Bishop, Rook, Queen, King
// Victims: Pawn, Knight, Bishop, Rook, Queen
// mvv_lva_array[attacker][victim]
typedef std::array<std::array<Move_Score, (PIECES::QUEEN + 1)>,
                   (PIECES::KING + 1)>
    mvv_lva_array;

template <std::size_t CONT_HIST_STACK_SIZE>
class Move_Ordering
{
  public:

    Move_Ordering(const Chess_Board& cb, const Chess_Move& hash_move);
    Move_Ordering(const Chess_Board& cb,
                  const Chess_Move&  hash_move,
                  const Continuation_History_Stack<CONT_HIST_STACK_SIZE>&
                      cont_hist_stack);

    Move_Generation_List&  get_sorted_moves();
    Moves_Bitboard_Matrix& get_moves_matrix();
    bool                   is_side_to_move_in_check() const;

    template <MOVE_GENERATION_TYPE move_gen_type>
    void generate_moves();

  private:

    const Chess_Board&                    m_chess_board;
    Move_Generation_List                  m_move_list;
    Moves_Bitboard_Matrix                 m_moves_matrix;
    Chess_Move                            m_hash_move;
    Static_Exchange_Evaluator<Move_Score> m_see;
    const Optional_Reference<
        const Continuation_History_Stack<CONT_HIST_STACK_SIZE>>
        m_cont_hist_stack;

    static mvv_lva_array generate_mvv_lva_array();
    void                 move_scorer();

    inline static mvv_lva_array m_mvv_lva_array =
        Move_Ordering<CONT_HIST_STACK_SIZE>::generate_mvv_lva_array();
};

template <std::size_t CONT_HIST_STACK_SIZE>
Move_Ordering<CONT_HIST_STACK_SIZE>::Move_Ordering(
    const Chess_Board& cb,
    const Chess_Move&  hash_move) :
    m_chess_board(cb), m_hash_move(hash_move), m_see(cb)
{
}

template <std::size_t CONT_HIST_STACK_SIZE>
Move_Ordering<CONT_HIST_STACK_SIZE>::Move_Ordering(
    const Chess_Board&                                      cb,
    const Chess_Move&                                       hash_move,
    const Continuation_History_Stack<CONT_HIST_STACK_SIZE>& cont_hist_stack) :
    m_chess_board(cb),
    m_hash_move(hash_move),
    m_see(cb),
    m_cont_hist_stack(cont_hist_stack)
{
}

template <std::size_t CONT_HIST_STACK_SIZE>
template <MOVE_GENERATION_TYPE move_gen_type>
void Move_Ordering<CONT_HIST_STACK_SIZE>::generate_moves()
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

template <std::size_t CONT_HIST_STACK_SIZE>
Move_Generation_List& Move_Ordering<CONT_HIST_STACK_SIZE>::get_sorted_moves()
{
    if (m_move_list.get_max_index() != -1)
    {
        move_scorer();
        m_move_list.sort();
    }
    return m_move_list;
}

template <std::size_t CONT_HIST_STACK_SIZE>
Moves_Bitboard_Matrix& Move_Ordering<CONT_HIST_STACK_SIZE>::get_moves_matrix()
{
    return m_moves_matrix;
}

template <std::size_t CONT_HIST_STACK_SIZE>
mvv_lva_array Move_Ordering<CONT_HIST_STACK_SIZE>::generate_mvv_lva_array()
{
    mvv_lva_array return_value;

    for (uint8_t attacker = PIECES::PAWN; attacker <= PIECES::KING; attacker++)
    {
        for (uint8_t victim = PIECES::PAWN; victim <= PIECES::QUEEN; victim++)
        {
            return_value[attacker][victim] =
                ((MVV_LVA_ATTACKER_VALUES[victim]
                  + NUM_OF_UNIQUE_PIECES_PER_PLAYER)
                 - (MVV_LVA_ATTACKER_VALUES[attacker] / 10));
        }
    }

    return return_value;
}

template <std::size_t CONT_HIST_STACK_SIZE>
void Move_Ordering<CONT_HIST_STACK_SIZE>::move_scorer()
{
    for (Chess_Move& move : m_move_list)
    {
        // For capture moves, apply MVV LVA score.
        if (move.is_capture)
        {
            move.score =
                m_mvv_lva_array[move.moving_piece][move.captured_piece];

            // move.score +=
            //     m_see.evaluate(move.destination_square, move.moving_piece,
            //     15);
        }
        // Enpassant is a capture but not labeled under is_capture for move
        // generator implementation reasons. The moving/attacking piece is the
        // same as the victim, both are pawns.
        if (move.is_en_passant)
        {
            move.score = m_mvv_lva_array[move.moving_piece][move.moving_piece];
        }

        // For ALL moves that induce a beta cutoff, apply continutation history
        // score.
        if (m_cont_hist_stack.has_ref())
        {
            const std::size_t ply = m_cont_hist_stack.get_ref().stack.size();

            const int64_t start = static_cast<int64_t>(ply) - 1;
            const int64_t end =
                (ply < CONTINUATION_HISTORY_LOOKBACK_DEPTH)
                    ? 0
                    : static_cast<int64_t>(ply)
                          - static_cast<int64_t>(
                              CONTINUATION_HISTORY_LOOKBACK_DEPTH);

            if ((start >= 0) && (end >= 0))
            {
                for (int64_t i = start; i >= end; i--)
                {
                    const auto& hist_table =
                        m_cont_hist_stack.get_ref()
                            .stack[static_cast<std::size_t>(i)];
                    move.score += hist_table.get_ref()[move];
                }
            }
        }

        // For the hash move, give it the maximum score to ensure it is sorted
        // to the front. If the hash move is not found, it won't be scored (e.g.
        // if move is Chess_Move()).
        if (move.is_same_move(m_hash_move))
        {
            move.score = std::numeric_limits<Move_Score>::max();
        }
    }
}

template <std::size_t CONT_HIST_STACK_SIZE>
bool Move_Ordering<CONT_HIST_STACK_SIZE>::is_side_to_move_in_check() const
{
    Move_Generator mg(m_chess_board);
    return mg.is_side_to_move_in_check();
}
