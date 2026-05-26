#include <ranges>

#include "evaluate.hpp"

class Static_Exchange_Evaluator
{
  public:

    Static_Exchange_Evaluator::Static_Exchange_Evaluator(
        const Evaluation_Weights<Matrex_FP_Int>& weights,
        const Chess_Board&                       position,
        const Square                             target_square);

    template <typename Return_Integral_Type>
    Return_Integral_Type evaluate();

  private:

    Square        m_target_square;
    Matrex_FP_Int m_initial_score;

    PIECE_COLOR m_this_side;

    Bitboard m_all_occupancies;
    multi_array<Bitboard, NUM_OF_PLAYERS, NUM_OF_UNIQUE_PIECES_PER_PLAYER>
        m_piece_bitboards;
    multi_array<Matrex_FP_Int, (NUM_OF_UNIQUE_PIECES_PER_PLAYER - 1)>
        m_material_weights;

    std::unique_ptr<std::vector<Placed_Piece>>
    what_pieces_attack_this_square(const Square      s,
                                   const PIECE_COLOR this_side) const;

    std::unique_ptr<std::vector<Placed_Piece>>
    get_all_interleaved_attackers(const Square      s,
                                  const PIECE_COLOR this_side) const;

}

Static_Exchange_Evaluator::Static_Exchange_Evaluator(const Evaluation_Weights<Matrex_FP_Int>& weights, const Chess_Board& position, const Square target_square) :
    : m_target_square(target_square),
      m_initial_score(weights.material[position.what_piece_is_on_square(target_square).second]),
      m_this_side(position.get_side_to_move()),
      m_all_occupancies(position.get_both_color_occupancies()),
      m_piece_bitboards[PIECE_COLOR::WHITE][PIECES::PAWN](position.get_piece_occupancies(PIECE_COLOR::WHITE, PIECES::PAWN)),
      m_piece_bitboards[PIECE_COLOR::WHITE][PIECES::KNIGHT](position.get_piece_occupancies(PIECE_COLOR::WHITE, PIECES::KNIGHT)),
      m_piece_bitboards[PIECE_COLOR::WHITE][PIECES::BISHOP](position.get_piece_occupancies(PIECE_COLOR::WHITE, PIECES::BISHOP)),
      m_piece_bitboards[PIECE_COLOR::WHITE][PIECES::ROOK](position.get_piece_occupancies(PIECE_COLOR::WHITE, PIECES::ROOK)),
      m_piece_bitboards[PIECE_COLOR::WHITE][PIECES::QUEEN](position.get_piece_occupancies(PIECE_COLOR::WHITE, PIECES::QUEEN)),
      m_piece_bitboards[PIECE_COLOR::WHITE][PIECES::KING](position.get_piece_occupancies(PIECE_COLOR::WHITE, PIECES::KING)),
      m_piece_bitboards[PIECE_COLOR::BLACK][PIECES::PAWN](position.get_piece_occupancies(PIECE_COLOR::BLACK, PIECES::PAWN)),
      m_piece_bitboards[PIECE_COLOR::BLACK][PIECES::KNIGHT](position.get_piece_occupancies(PIECE_COLOR::BLACK, PIECES::KNIGHT)),
      m_piece_bitboards[PIECE_COLOR::BLACK][PIECES::BISHOP](position.get_piece_occupancies(PIECE_COLOR::BLACK, PIECES::BISHOP)),
      m_piece_bitboards[PIECE_COLOR::BLACK][PIECES::ROOK](position.get_piece_occupancies(PIECE_COLOR::BLACK, PIECES::ROOK)),
      m_piece_bitboards[PIECE_COLOR::BLACK][PIECES::QUEEN](position.get_piece_occupancies(PIECE_COLOR::BLACK, PIECES::QUEEN)),
      m_piece_bitboards[PIECE_COLOR::BLACK][PIECES::KING](position.get_piece_occupancies(PIECE_COLOR::BLACK, PIECES::KING)),
      m_material_weights(weights.material)

{
}

std::unique_ptr<std::vector<Placed_Piece>>
Static_Exchange_Evaluator::what_pieces_attack_this_square(
    const Square      s,
    const PIECE_COLOR this_side) const
{
    std::unique_ptr<std::vector<Placed_Piece>> attackers =
        std::make_unique<std::vector<Placed_Piece>>(NUM_OF_PIECES_PER_PLAYER);

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
        attackers->push_back(
            {.color = this_side, .piece = PIECES::KING, .square = attacker});
    }

    for (const Square attacker : this_attacking_queens)
    {
        attackers->push_back(
            {.color = this_side, .piece = PIECES::QUEEN, .square = attacker});
    }

    for (const Square attacker : this_attacking_rooks)
    {
        attackers->push_back(
            {.color = this_side, .piece = PIECES::ROOK, .square = attacker});
    }

    for (const Square attacker : this_attacking_bishops)
    {
        attackers->push_back(
            {.color = this_side, .piece = PIECES::BISHOP, .square = attacker});
    }

    for (const Square attacker : this_attacking_knights)
    {
        attackers->push_back(
            {.color = this_side, .piece = PIECES::KNIGHT, .square = attacker});
    }

    for (const Square attacker : this_attacking_pawns)
    {
        attackers->push_back(
            {.color = this_side, .piece = PIECES::PAWN, .square = attacker});
    }

    return attackers;
}

std::unique_ptr<std::vector<Placed_Piece>>
Static_Exchange_Evaluator::get_all_interleaved_attackers(
    const Square      s,
    const PIECE_COLOR this_side) const
{
    auto interleaved_attackers = std::make_unique<std::vector<Placed_Piece>>();

    auto this_side_attackers  = what_pieces_attack_this_square(s, this_side);
    auto other_side_attackers = what_pieces_attack_this_square(s, ~this_side);

    for (const auto& [this_side_attacker, other_side_attacker] :
         std::views::zip(*this_side_attackers, *other_side_attackers))
    {
        interleaved_attackers->push_back(this_side_attacker);
        interleaved_attackers->push_back(other_side_attacker);
    }

    // The above logic will always pick out the maximum number of pairs of
    // attackers from both sides, but there may be one or more extra attackers
    // from one side that need to be added to the end of the interleaved list.
    // Since the pairs are always (this side (first), other side (second)), the
    // last pair will always end with other side so we only need to consider
    // this side's attackers for the extra attackers. We only push the next one
    // because that is what is needed to prove at least an equal trade of
    // pieces.
    if (this_side_attackers->size() > other_side_attackers->size())
    {
        interleaved_attackers->push_back(
            this_side_attackers[other_side_attackers->size()]);
    }

    return interleaved_attackers;
}

template <typename Return_Integral_Type>
Return_Integral_Type Static_Exchange_Evaluator::evaluate()
{
    // Score of the overall exchange sequence.
    Matrex_FP_Int score = 0;

    // All attackers to this square from both sides in interleaved order and in
    // order of least to greatest material value (e.g. white, black, white, ...
    // ).
    auto attackers =
        get_all_interleaved_attackers(m_target_square, m_this_side);

    if (attackers.size() == 0) { return score; }
    else
    {
        // There is a follow-up attacker so the value of the material currently
        // on the target square is the initial score of the exchange sequence.
        score = m_initial_score;
    }

    while (true)
    {
        // If there are no more attackers hidden or unhidden, then we are done.
        if (attackers.size() == 0) { break; }

        // Loop over all attackers and clear their squares (to simulate
        // capturing) and add/subtract the material value of the piece to the
        // score depending on which side the piece is on.
        while (attackers.size() > 0)
        {
            // Check the size is non-zero after popping an attacker, so it is
            // known that a follow-up attacker can capture the current attacker.
            bool is_there_more_attackers = ((attackers.size() - 1) > 0);

            const Placed_Piece& attacker = attackers.back();

            if (is_there_more_attackers)
            {
                // If the current attacker on this side will be captured,
                // subtract off it's value.
                if (attacker.color == m_this_side)
                {
                    score -= m_material_weights[attacker.piece];
                }

                // If the current attacker on the other side will be captured,
                // add its value.
                if (attacker.color == (~m_this_side))
                {
                    score += m_material_weights[attacker.piece];
                }

                // Clear the attacker from it's square which will unhide any
                // attackers that were behind it.
                m_all_occupancies.unset_square(attacker.square);
                m_piece_bitboards[attacker.color][attacker.piece].unset_square(
                    attacker.square);
            }

            attackers.pop_back();
        }

        // Generate next set of attackers that may have been hidden behind other
        // attackers.
        attackers = get_all_interleaved_attackers(m_target_square, m_this_side);
    }

    return static_cast<Return_Integral_Type>(score.get_integer());
}
