#include <ranges>

#include "evaluate.hpp"

using SEE_Attackers_Array =
    Partially_Filled_Array<Placed_Piece, NUM_OF_PIECES_PER_PLAYER>;
using SEE_Interleaved_Attackers_Array =
    Partially_Filled_Array<Placed_Piece, (2 * NUM_OF_PIECES_PER_PLAYER)>;

template <typename Integral_Type>
class Static_Exchange_Evaluator
{
  public:

    Static_Exchange_Evaluator(const Chess_Board& position,
                              const Square       target_square);

    Integral_Type evaluate();

  private:

    Square m_target_square;
    PIECES m_initial_piece;

    PIECE_COLOR m_this_side;

    Bitboard m_all_occupancies;
    multi_array<Bitboard, NUM_OF_PLAYERS, NUM_OF_UNIQUE_PIECES_PER_PLAYER>
        m_piece_bitboards;

    // These are general material weights, we don't use the evaluation weights
    // because they require computations too slow for SEE purposes.
    static constexpr multi_array<Integral_Type,
                                 (NUM_OF_UNIQUE_PIECES_PER_PLAYER - 1)>
        m_material_weights = {10, 30, 35, 50, 90};

    SEE_Attackers_Array
    what_pieces_attack_this_square(const Square      s,
                                   const PIECE_COLOR this_side) const;

    SEE_Interleaved_Attackers_Array
    get_all_interleaved_attackers(const Square      s,
                                  const PIECE_COLOR this_side,
                                  bool&             only_kings) const;
};

template <typename Integral_Type>
Static_Exchange_Evaluator<Integral_Type>::Static_Exchange_Evaluator(
    const Chess_Board& position,
    const Square       target_square) :
    m_target_square(target_square),
    m_initial_piece(position.what_piece_is_on_square(target_square).second),
    m_this_side(position.get_side_to_move()),
    m_all_occupancies(position.get_both_color_occupancies()),
    m_piece_bitboards(position.get_piece_occupancies())
{
}

template <typename Integral_Type>
SEE_Attackers_Array
Static_Exchange_Evaluator<Integral_Type>::what_pieces_attack_this_square(
    const Square      s,
    const PIECE_COLOR this_side) const
{
    SEE_Attackers_Array attackers;

    const Attacks a;

    const Bitboard pawn_attacks   = a.get_pawn_attacks(s, this_side);
    const Bitboard bishop_attacks = a.get_bishop_attacks(s, m_all_occupancies);
    const Bitboard knight_attacks = a.get_knight_attacks(s);
    const Bitboard rook_attacks   = a.get_rook_attacks(s, m_all_occupancies);
    const Bitboard king_attacks   = a.get_king_attacks(s);

    const Bitboard this_attacking_pawns =
        m_piece_bitboards[this_side][PIECES::PAWN] & pawn_attacks;
    const Bitboard this_attacking_knights =
        m_piece_bitboards[this_side][PIECES::KNIGHT] & knight_attacks;
    const Bitboard this_attacking_bishops =
        m_piece_bitboards[this_side][PIECES::BISHOP] & bishop_attacks;
    const Bitboard this_attacking_rooks =
        m_piece_bitboards[this_side][PIECES::ROOK] & rook_attacks;
    const Bitboard this_attacking_queens =
        m_piece_bitboards[this_side][PIECES::QUEEN]
        & (bishop_attacks | rook_attacks);
    const Bitboard this_attacking_kings =
        m_piece_bitboards[this_side][PIECES::KING] & king_attacks;

    // Note: These pushes are in order from least to greatest material value
    // which is vital to the SEE logic. The least to the back of the array
    // because for performance reasons, vectors should be popped from the back
    // when possible.

    for (const Square attacker : this_attacking_kings)
    {
        attackers.append(
            {.color = this_side, .piece = PIECES::KING, .square = attacker});
    }

    for (const Square attacker : this_attacking_queens)
    {
        attackers.append(
            {.color = this_side, .piece = PIECES::QUEEN, .square = attacker});
    }

    for (const Square attacker : this_attacking_rooks)
    {
        attackers.append(
            {.color = this_side, .piece = PIECES::ROOK, .square = attacker});
    }

    for (const Square attacker : this_attacking_bishops)
    {
        attackers.append(
            {.color = this_side, .piece = PIECES::BISHOP, .square = attacker});
    }

    for (const Square attacker : this_attacking_knights)
    {
        attackers.append(
            {.color = this_side, .piece = PIECES::KNIGHT, .square = attacker});
    }

    for (const Square attacker : this_attacking_pawns)
    {
        attackers.append(
            {.color = this_side, .piece = PIECES::PAWN, .square = attacker});
    }

    return attackers;
}

template <typename Integral_Type>
SEE_Interleaved_Attackers_Array
Static_Exchange_Evaluator<Integral_Type>::get_all_interleaved_attackers(
    const Square      s,
    const PIECE_COLOR this_side,
    bool&             only_kings) const
{
    SEE_Interleaved_Attackers_Array interleaved_attackers;

    auto this_side_attackers  = what_pieces_attack_this_square(s, this_side);
    auto other_side_attackers = what_pieces_attack_this_square(s, ~this_side);

    // The only kings boolean starts as true if either side as a size of 1 or
    // both sides have a size of 1 (this is equivalent to an OR).
    only_kings =
        ((this_side_attackers.size() | other_side_attackers.size()) == 1);

    // The below for loop logic will always pick out the maximum number of pairs
    // of attackers from both sides, but there may be one or more extra
    // attackers from one side that need to be added to the start of the
    // interleaved list. Since the pairs are always (other side (first), first
    // side (second)), the first pair will always start with other side so we
    // only need to consider this side's attackers for the extra attackers. We
    // only push one because that is what is needed to prove at least an equal
    // trade of pieces and we must alternate sides. It is vital that this be
    // appended to the start because an attacker of this side must be the the
    // last element in the list (this side moves first).
    bool this_side_attacker_advantage =
        this_side_attackers.size() > other_side_attackers.size();
    if (this_side_attacker_advantage)
    {
        only_kings =
            only_kings && (this_side_attackers.front().piece == PIECES::KING);
        interleaved_attackers.append(this_side_attackers.front());
    }

    for (const auto& [other_side_attacker, this_side_attacker] :
         std::views::zip(
             other_side_attackers,
             (this_side_attackers
              | std::views::drop(this_side_attacker_advantage ? 1 : 0))))
    {
        only_kings = only_kings
                  && ((this_side_attacker.piece == PIECES::KING)
                      && (other_side_attacker.piece == PIECES::KING));
        interleaved_attackers.append(other_side_attacker);
        interleaved_attackers.append(this_side_attacker);
    }

    // Note: If the other side has more attackers it is accounted for because
    // this side must be first to move and if this side has no more attackers,
    // the capture sequence ends.

    if (interleaved_attackers.size() > 0)
    {
        MATREX_ASSERT((interleaved_attackers.back().color == this_side),
                      "The current side to move is not the back of the "
                      "interleaved attackers.");
    }

    return interleaved_attackers;
}

template <typename Integral_Type>
Integral_Type Static_Exchange_Evaluator<Integral_Type>::evaluate()
{
    // Score of the overall exchange sequence.
    Integral_Type score = 0;

    bool only_kings = false;

    // All attackers to this square from both sides in interleaved order and in
    // order of least to greatest material value (e.g. white, black, white, ...
    // ).
    auto attackers =
        get_all_interleaved_attackers(m_target_square, m_this_side, only_kings);

    if (attackers.size() == 0) { return score; }

    bool is_first_iteration = true;
    while (true)
    {
        // If there are no more attackers hidden or unhidden, then we are done.
        if (attackers.size() == 0) { break; }

        Placed_Piece previous_attacker;
        if (is_first_iteration)
        {
            previous_attacker  = {.color  = ~m_this_side,
                                  .piece  = m_initial_piece,
                                  .square = m_target_square};
            is_first_iteration = false;
        }

        // Loop over all attackers and clear their squares (to simulate
        // capturing) and add/subtract the material value of the piece to the
        // score depending on which side the piece is on.
        while (attackers.size() > 0)
        {
            const Placed_Piece attacker = attackers.back();

            if (attacker.piece == PIECES::KING)
            {
                // Pre-compute the next set of attackers - if there are any
                // attackers besides kings - don't execute any other logic.
                attackers = get_all_interleaved_attackers(m_target_square,
                                                          m_this_side,
                                                          only_kings);
                if (!only_kings)
                {
                    goto prev_attacker_value_post_hidden_attackers;
                }

                // If there is only an enemy king left attacking the target
                // square, then account for the value of the previous attacker
                // because the king can capture it.
                if ((attackers.size() == 1)
                    && (attackers.back().color == (~previous_attacker.color)))
                {
                    if (previous_attacker.color == m_this_side)
                    {
                        score -= m_material_weights[previous_attacker.piece];
                    }
                    else
                    {
                        score += m_material_weights[previous_attacker.piece];
                    }
                }

                // After accounting for the last attacker's value and ensuring
                // the king was the lone attacker - we are done just return the
                // current score.
                return score;
            }

            // Account for the value of the previous attacker.
            if (previous_attacker.color == m_this_side)
            {
                score -= m_material_weights[previous_attacker.piece];
            }

            if (previous_attacker.color == (~m_this_side))
            {
                score += m_material_weights[previous_attacker.piece];
            }

            // Clear the attacker from it's square which will unhide any
            // attackers that were behind it.
            m_all_occupancies.unset_square(attacker.square);
            m_piece_bitboards[attacker.color][attacker.piece].unset_square(
                attacker.square);

            previous_attacker = attacker;
            attackers.pop();
        }

        // Generate next set of attackers that may have been hidden behind other
        // attackers.
        attackers = get_all_interleaved_attackers(m_target_square,
                                                  ~previous_attacker.color,
                                                  only_kings);

    // We have hidden attackers so we need to account for the value of the
    // last attacker (before previous attacker resets) because it will be
    // captured by the hidden attackers.
    prev_attacker_value_post_hidden_attackers:
        if (attackers.size() > 0)
        {
            if (previous_attacker.color == m_this_side)
            {
                score -= m_material_weights[previous_attacker.piece];
            }

            if (previous_attacker.color == (~m_this_side))
            {
                score += m_material_weights[previous_attacker.piece];
            }
        }
    }

    return score;
}
