#pragma once

#include <ranges>

#include "evaluate.hpp"

constexpr std::size_t SEE_ATTACKERS_ARRAY_SIZE = NUM_OF_PIECES_PER_PLAYER;

using SEE_Attackers_Array =
    Partially_Filled_Array<Placed_Piece, SEE_ATTACKERS_ARRAY_SIZE>;
using SEE_Interleaved_Attackers_Array =
    Partially_Filled_Array<Placed_Piece, (2 * SEE_ATTACKERS_ARRAY_SIZE)>;

template <typename Integral_Type>
class Static_Exchange_Evaluator
{
  public:

    Static_Exchange_Evaluator(const Chess_Board& position);

    Integral_Type evaluate(Square        target_square,
                           PIECES        moving_piece,
                           Integral_Type score_scaler);

  private:

    const Chess_Board& m_position;

    PIECE_COLOR m_this_side;

    Bitboard m_all_occupancies;
    multi_array<Bitboard, NUM_OF_PLAYERS, NUM_OF_UNIQUE_PIECES_PER_PLAYER>
        m_piece_bitboards;

    // These are general material weights, we don't use the evaluation weights
    // because they require computations too slow for SEE purposes.
    static constexpr multi_array<Integral_Type,
                                 (NUM_OF_UNIQUE_PIECES_PER_PLAYER - 1)>
        m_material_weights = {10, 30, 35, 50, 90};

    static constexpr multi_array<Integral_Type, NUM_OF_UNIQUE_PIECES_PER_PLAYER>
        m_moving_piece_penalties = {1, 3, 4, 5, 9, 15};

    SEE_Attackers_Array
    what_pieces_attack_this_square(const Square      s,
                                   const PIECE_COLOR this_side) const;

    SEE_Interleaved_Attackers_Array
    get_all_interleaved_attackers(const Square      s,
                                  const PIECE_COLOR this_side,
                                  bool&             only_kings) const;

    void reset_occupancy_bitboards();

    inline Placed_Piece find_hidden_attacker(const Placed_Piece& previous_attacker, Square target_square, const PIECE_COLOR side) const;

    inline void insert_hidden_attacker(SEE_Interleaved_Attackers_Array& attackers, const Placed_Piece& hidden_attacker) const;
};

template <typename Integral_Type>
Static_Exchange_Evaluator<Integral_Type>::Static_Exchange_Evaluator(
    const Chess_Board& position) :
    m_position(position),
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

    const Bitboard pawn_attacks   = a.get_pawn_attacks(s, ~this_side);
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

    // Expected count of the number of interleaved attackers.
    const std::size_t expected_pairs = std::min(this_side_attackers.size(), other_side_attackers.size());
    const std::size_t expected_count = 2 * expected_pairs;

    // If there are more this side attackers we can append at most one of the
    // extra attackers - we append the smallest cost attacker that is beyond the
    // number of expected pairs (minimum greatest cost).
    const bool this_side_attacker_advantage =
        this_side_attackers.size() > other_side_attackers.size();
    const std::size_t append_index = (this_side_attackers.size() - 1) - expected_pairs;
    if (this_side_attacker_advantage)
    {
        only_kings =
            only_kings && (this_side_attackers[append_index].piece == PIECES::KING);
        interleaved_attackers.append(this_side_attackers[append_index]);
    }

    auto attackers = std::array{std::ref(this_side_attackers), std::ref(other_side_attackers)};
    auto indices = std::array{this_side_attackers.get_max_index(), other_side_attackers.get_max_index()};
    PIECE_COLOR side = this_side;
    std::size_t count = 0;
    while (true)
    {
        // No side can have zero attackers.
        if ((this_side_attackers.get_max_index() < 0) || (other_side_attackers.get_max_index() < 0))
        {
            break;
        }

        // Get the cheapest attacker for the current side - we start the indices
        // from the back because the attackers for each side are arranged from
        // most expensive to least expensive.
        Placed_Piece cheapest_attacker;
        if (side == this_side)
        {
            cheapest_attacker = attackers[0].get()[indices[0]];
        }
        if (side == (~this_side))
        {
            cheapest_attacker = attackers[1].get()[indices[1]];
        }

        // Update boolean that tracks if only kings are left as attackers.
        only_kings = only_kings && (cheapest_attacker.piece == PIECES::KING);

        // Place the least expensive attacker for the current side into the 
        // interleaved attackers array.
        const std::size_t interleaved_index = ((expected_count - 1) - count) + (this_side_attacker_advantage ? 1 : 0);
        interleaved_attackers[interleaved_index] = cheapest_attacker;

        // Count until we get the expected number of attackers into the 
        // interleaved array.
        ++count;
        if (count == expected_count)
        {
            break;
        }

        // Maintain indices for each side. 
        if (side == this_side)
        {
            --indices[0];
        }
        if (side == (~this_side))
        {
            --indices[1];
        }

        // Alternate sides.
        side = ~side;
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
void Static_Exchange_Evaluator<Integral_Type>::reset_occupancy_bitboards()
{
    m_all_occupancies = m_position.get_both_color_occupancies();
    m_piece_bitboards = m_position.get_piece_occupancies();
}

template <typename Integral_Type>
inline Placed_Piece Static_Exchange_Evaluator<Integral_Type>::find_hidden_attacker(const Placed_Piece& previous_attacker, Square target_square, const PIECE_COLOR side) const
{
    // Attacker ray - The ray going in the direction of the previous attacker 
    // from the target square until it reaches the edge of the board.
    const auto& attacker_ray = Attacks::get_directional_ray(target_square, previous_attacker.square);

    // All occupancies along the attacker ray.
    Bitboard occupancies = attacker_ray.ray & m_all_occupancies;

    // Eliminate occupanices that cannot be attackers - the attackers are not 
    // the previous attacker or the piece on the target square being evaluated 
    // for capture.
    occupancies.unset_square(previous_attacker.square);
    occupancies.unset_square(target_square);

    // Get the first occupied square after the previous attacker on the ray.
    constexpr bool START_FROM_START_SQUARE = true;
    constexpr uint8_t FIRST_OCCUPIED_INDEX = 0;
    const Square first_occupied_square = attacker_ray.travel_occupied_ray<START_FROM_START_SQUARE>(FIRST_OCCUPIED_INDEX, occupancies);

    // A bishop attacks the target square.
    if (m_piece_bitboards[side][PIECES::BISHOP].get_square(first_occupied_square))
    {
        return {.color = side, .piece = PIECES::BISHOP, .square = first_occupied_square};
    }

    // A rook attacks the target square.
    if (m_piece_bitboards[side][PIECES::ROOK].get_square(first_occupied_square))
    {
        return {.color = side, .piece = PIECES::ROOK, .square = first_occupied_square};
    }

    // A queen attacks the target square.
    if (m_piece_bitboards[side][PIECES::QUEEN].get_square(first_occupied_square))
    {
        return {.color = side, .piece = PIECES::QUEEN, .square = first_occupied_square};
    }

    // No sliding pieces attack the square therefore, there is no hidden
    // attackers on the ray.
    return Placed_Piece();
}

template <typename Integral_Type>
inline void Static_Exchange_Evaluator<Integral_Type>::insert_hidden_attacker(SEE_Interleaved_Attackers_Array& attackers, const Placed_Piece& hidden_attacker) const
{
    // There are no current attackers, just return and let the algorithm 
    // generate new attackers.
    if (attackers.size() == 0)
    {
        return;
    }

    // Find the position in the array where the hidden attacker should be 
    // inserted based on only material value.
    auto material_position = std::lower_bound (
        attackers.begin(),
        attackers.end(),
        hidden_attacker,
        [](const Placed_Piece& a, const Placed_Piece& b)
        {
            return (m_material_weights[a.piece] >= m_material_weights[b.piece]);
        }
    );

    // If the hidden attacker is higher in material value than all attackers in
    // the array, return.
    if (material_position == attackers.end())
    {
        return;
    }

    // Based on the material value we are in the correct locality of the array 
    // but we need to insert the hidden attacker where its opposite in color to
    // the adjacent elements. Since, the attackers array is in descending 
    // interleaved order we can just go an index back from the current index if
    // the color isn't the same. 
    std::size_t insert_index = static_cast<std::size_t>(std::distance(attackers.begin(), material_position));
    if ((attackers[insert_index].color != hidden_attacker.color) && (insert_index != 0))
    {
        insert_index--;
    }

    // If the hidden attacker is not the same color as the last attacker and it 
    // belongs at that position or before, return.
    if ((insert_index == 0) && (attackers[insert_index].color != hidden_attacker.color))
    {
        return;
    }

    // Insert the hidden attacker into the array via swapping.
    auto value = hidden_attacker;
    for (int8_t i = insert_index; i >= 0; i -= NUM_OF_PLAYERS)
    {
        std::swap(value, attackers[i]);
    }
}

template <typename Integral_Type>
Integral_Type
Static_Exchange_Evaluator<Integral_Type>::evaluate(Square        target_square,
                                                   PIECES        moving_piece,
                                                   Integral_Type score_scaler)
{
    bool only_kings = false;

    // All attackers to this square from both sides in interleaved order and in
    // order of least to greatest material value (e.g. white, black, white, ...
    // ).
    SEE_Interleaved_Attackers_Array attackers = get_all_interleaved_attackers(target_square, m_this_side, only_kings);

    // Score of the overall exchange sequence. Start with a penalty if the move
    // does not capture with the least valuable piece first.
    Integral_Type score = (m_moving_piece_penalties[attackers.back().piece]
                           - m_moving_piece_penalties[moving_piece]);

    bool is_first_iteration = true;
    Placed_Piece previous_attacker;
    while (true)
    {
        // If there are no more attackers hidden or unhidden, then we are done.
        if (attackers.size() == 0) { break; }

        if (is_first_iteration)
        {
            previous_attacker = {
                .color = ~m_this_side,
                .piece =
                    m_position.what_piece_is_on_square(target_square).second,
                .square = target_square};

            // Clear the target square of it's original piece.
            m_all_occupancies.unset_square(previous_attacker.square);
            m_piece_bitboards[previous_attacker.color][previous_attacker.piece]
                .unset_square(previous_attacker.square);

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
                // attackers besides kings - don't execute any other logic. The
                // king will not attack if there are other attackers so generate
                // attackers for the opposing side of the previous attacker.
                attackers =
                    get_all_interleaved_attackers(target_square,
                                                  ~previous_attacker.color,
                                                  only_kings);

                // If there is not only kings but only this king and less than
                // or equal to 1 attackers total, then we can return because the
                // king cannot capture a guarded piece. 
                if ((!only_kings) && (attackers.size() <= 2))
                {
                    reset_occupancy_bitboards();
                    return (score * score_scaler);
                }
      
                if (!only_kings) { break; }

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
                reset_occupancy_bitboards();
                return (score * score_scaler);
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

            // Find if a hidden attacker was revealed for either side.
            Placed_Piece hidden_attacker = find_hidden_attacker(previous_attacker, target_square, ~previous_attacker.color);
            if (hidden_attacker == Placed_Piece())
            {
                hidden_attacker = find_hidden_attacker(previous_attacker, target_square, previous_attacker.color);
            }

            // If we found a hidden attacker insert it into the attackers array.
            if (hidden_attacker != Placed_Piece())
            {
                insert_hidden_attacker(attackers, hidden_attacker);
            }
        }

        // Generate next set of attackers that may have been hidden behind other
        // attackers.
        attackers = get_all_interleaved_attackers(target_square,
                                                  ~previous_attacker.color,
                                                  only_kings);
    }

    reset_occupancy_bitboards();
    return (score * score_scaler);
}
