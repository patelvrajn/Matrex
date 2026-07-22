#pragma once

#include <cmath>

#include "chess_board.hpp"
#include "globals.hpp"
#include "move_generator.hpp"
#include "non_linear_response.hpp"
#include "score.hpp"
#include "correction_history_table.hpp"
#include "evaluation_weights.hpp"

template <typename T>
class Evaluator
{
  public:

    Evaluator(const Evaluation_Weights<T>& weights,
              const Chess_Board&           cb,
              const Moves_Bitboard_Matrix& moving_side_matrix,
              const Moves_Bitboard_Matrix& opposing_side_matrix);

    T evaluate_template_typed() const;

    template <std::size_t corr_hist_table_size>
    Score evaluate(const Correction_History_Tables<corr_hist_table_size>&
                       corr_hist_tables) const;

    template <PIECE_COLOR moving_side>
    inline T material_score() const;

    template <PIECE_COLOR moving_side>
    inline T mobility_score() const;

    template <PIECE_COLOR moving_side>
    inline T piece_square_score() const;

  private:

    const Evaluation_Weights<T>& m_weights;
    const Chess_Board&           m_chess_board;
    const Moves_Bitboard_Matrix& m_moving_side_matrix;
    const Moves_Bitboard_Matrix& m_opposing_side_matrix;

    // Helpers
    template <PIECE_COLOR side>
    inline T calculate_piece_mobility(const Moves_Bitboard_Matrix& matrix,
                                      const PIECES                 piece) const;

    T constant_conversion(const double value) const;
};

template <typename T>
Evaluator<T>::Evaluator(const Evaluation_Weights<T>& weights,
                        const Chess_Board&           cb,
                        const Moves_Bitboard_Matrix& moving_side_matrix,
                        const Moves_Bitboard_Matrix& opposing_side_matrix) :
    m_weights(weights),
    m_chess_board(cb),
    m_moving_side_matrix(moving_side_matrix),
    m_opposing_side_matrix(opposing_side_matrix)
{
}

template <typename T>
T Evaluator<T>::evaluate_template_typed() const
{
    PIECE_COLOR moving_side = m_chess_board.get_side_to_move();

    T material;
    T mobility;
    T piece_square;

    if (moving_side == PIECE_COLOR::WHITE)
    {
        material     = material_score<PIECE_COLOR::WHITE>();
        mobility     = mobility_score<PIECE_COLOR::WHITE>();
        piece_square = piece_square_score<PIECE_COLOR::WHITE>();
    }
    else
    {
        material     = material_score<PIECE_COLOR::BLACK>();
        mobility     = mobility_score<PIECE_COLOR::BLACK>();
        piece_square = piece_square_score<PIECE_COLOR::BLACK>();
    }

    const T evaluation = material + mobility + piece_square;

    return evaluation;
}

template <typename T>
template <std::size_t corr_hist_table_size>
Score Evaluator<T>::evaluate(
    const Correction_History_Tables<corr_hist_table_size>& corr_hist_tables)
    const
{
    const Score corrected_evaluation =
        Score(evaluate_template_typed())
        + corr_hist_tables.get_correction(m_chess_board);
    T clamped_evaluation =
        Matrex_FP_Int(std::clamp(corrected_evaluation.to_int(),
                                 FP_EVALUATION_MIN,
                                 FP_EVALUATION_MAX));
    const Score return_value = Score(clamped_evaluation);
    return return_value;
}

template <typename T>
template <PIECE_COLOR moving_side>
inline T Evaluator<T>::material_score() const
{
    constexpr PIECE_COLOR opposing_side = ~moving_side;

    T return_value = constant_conversion(0.0);

    for (uint8_t piece = PIECES::PAWN; piece <= PIECES::QUEEN; ++piece)
    {
        T material_difference = constant_conversion(0.0);

        material_difference +=
            (m_weights.material[piece]
             * m_chess_board.get_piece_occupancies(moving_side, (PIECES) piece)
                   .high_bit_count());

        material_difference -=
            (m_weights.material[piece]
             * m_chess_board
                   .get_piece_occupancies(opposing_side, (PIECES) piece)
                   .high_bit_count());

        T non_linear_material =
            Non_Linear_Response(m_weights.material_NLR_parameters[piece])
                .value(material_difference);

        return_value += non_linear_material;
    }

    return return_value;
}

template <typename T>
template <PIECE_COLOR moving_side>
inline T Evaluator<T>::mobility_score() const
{
    constexpr PIECE_COLOR opposing_side = ~moving_side;

    T mobility = constant_conversion(0.0);

    for (uint8_t piece = PIECES::PAWN; piece <= PIECES::KING; ++piece)
    {
        const T moving_side_piece_mobility =
            calculate_piece_mobility<moving_side>(m_moving_side_matrix,
                                                  (PIECES) piece);
        const T opposing_side_piece_mobility =
            calculate_piece_mobility<opposing_side>(m_opposing_side_matrix,
                                                    (PIECES) piece);

        const T piece_mobility_difference =
            moving_side_piece_mobility - opposing_side_piece_mobility;

        mobility +=
            Non_Linear_Response(m_weights.piece_mobility_NLR_parameters[piece])
                .value(piece_mobility_difference);
    }

    return static_cast<T>(mobility);
}

template <typename T>
template <PIECE_COLOR moving_side>
inline T Evaluator<T>::piece_square_score() const
{
    constexpr PIECE_COLOR opposing_side = ~moving_side;

    // Accumulate the piece-square values from the piece-square tables for the
    // present state of the board. The accumulations are per piece per side.
    Multi_Array<T, NUM_OF_PLAYERS, NUM_OF_UNIQUE_PIECES_PER_PLAYER>
        color_piece_values {};
    for (uint8_t color = PIECE_COLOR::WHITE; color <= PIECE_COLOR::BLACK;
         ++color)
    {
        for (uint8_t piece = PIECES::PAWN; piece <= PIECES::KING; ++piece)
        {
            // Initialize the array value for the case of T = AD Value which
            // contains optionals.
            color_piece_values[color][piece] = constant_conversion(0.0);

            for (uint8_t square_idx = 0;
                 square_idx < NUM_OF_SQUARES_ON_CHESS_BOARD;
                 ++square_idx)
            {
                color_piece_values[color][piece] +=
                    m_weights.piece_square_tables[color][piece][square_idx]
                    * (m_chess_board
                           .get_piece_occupancies((PIECE_COLOR) color,
                                                  (PIECES) piece)
                           .get_square(Square(square_idx))
                       > 0);
            }
        }
    }

    // NLR objects for this side's pieces.
    const Non_Linear_Response<T> nlr_this_king(
        m_weights.piece_square_NLR_parameters[moving_side][PIECES::KING]);
    const Non_Linear_Response<T> nlr_this_queen(
        m_weights.piece_square_NLR_parameters[moving_side][PIECES::QUEEN]);
    const Non_Linear_Response<T> nlr_this_rook(
        m_weights.piece_square_NLR_parameters[moving_side][PIECES::ROOK]);
    const Non_Linear_Response<T> nlr_this_bishop(
        m_weights.piece_square_NLR_parameters[moving_side][PIECES::BISHOP]);
    const Non_Linear_Response<T> nlr_this_knight(
        m_weights.piece_square_NLR_parameters[moving_side][PIECES::KNIGHT]);
    const Non_Linear_Response<T> nlr_this_pawn(
        m_weights.piece_square_NLR_parameters[moving_side][PIECES::PAWN]);

    // NLR values for this side's pieces and their interaction.
    const T nlr_this_king_value =
        nlr_this_king.value(color_piece_values[moving_side][PIECES::KING]);
    const T nlr_this_queen_value =
        nlr_this_queen.value(color_piece_values[moving_side][PIECES::QUEEN]);
    const T nlr_this_rook_value =
        nlr_this_rook.value(color_piece_values[moving_side][PIECES::ROOK]);
    const T nlr_this_bishop_value =
        nlr_this_bishop.value(color_piece_values[moving_side][PIECES::BISHOP]);
    const T nlr_this_knight_value =
        nlr_this_knight.value(color_piece_values[moving_side][PIECES::KNIGHT]);
    const T nlr_this_pawn_value =
        nlr_this_pawn.value(color_piece_values[moving_side][PIECES::PAWN]);
    const T nlr_this_interaction_value =
        nlr_this_king_value * nlr_this_queen_value * nlr_this_rook_value
        * nlr_this_bishop_value * nlr_this_knight_value * nlr_this_pawn_value;

    // NLR objects for opposing side's pieces.
    const Non_Linear_Response<T> nlr_opposing_king(
        m_weights.piece_square_NLR_parameters[opposing_side][PIECES::KING]);
    const Non_Linear_Response<T> nlr_opposing_queen(
        m_weights.piece_square_NLR_parameters[opposing_side][PIECES::QUEEN]);
    const Non_Linear_Response<T> nlr_opposing_rook(
        m_weights.piece_square_NLR_parameters[opposing_side][PIECES::ROOK]);
    const Non_Linear_Response<T> nlr_opposing_bishop(
        m_weights.piece_square_NLR_parameters[opposing_side][PIECES::BISHOP]);
    const Non_Linear_Response<T> nlr_opposing_knight(
        m_weights.piece_square_NLR_parameters[opposing_side][PIECES::KNIGHT]);
    const Non_Linear_Response<T> nlr_opposing_pawn(
        m_weights.piece_square_NLR_parameters[opposing_side][PIECES::PAWN]);

    // NLR values for opposing side's pieces and their interaction.
    const T nlr_opposing_king_value = nlr_opposing_king.value(
        color_piece_values[opposing_side][PIECES::KING]);
    const T nlr_opposing_queen_value = nlr_opposing_queen.value(
        color_piece_values[opposing_side][PIECES::QUEEN]);
    const T nlr_opposing_rook_value = nlr_opposing_rook.value(
        color_piece_values[opposing_side][PIECES::ROOK]);
    const T nlr_opposing_bishop_value = nlr_opposing_bishop.value(
        color_piece_values[opposing_side][PIECES::BISHOP]);
    const T nlr_opposing_knight_value = nlr_opposing_knight.value(
        color_piece_values[opposing_side][PIECES::KNIGHT]);
    const T nlr_opposing_pawn_value = nlr_opposing_pawn.value(
        color_piece_values[opposing_side][PIECES::PAWN]);
    const T nlr_opposing_interaction_value =
        nlr_opposing_king_value * nlr_opposing_queen_value
        * nlr_opposing_rook_value * nlr_opposing_bishop_value
        * nlr_opposing_knight_value * nlr_opposing_pawn_value;

    // Explicit interactive term NLR objects per side.
    const Non_Linear_Response<T> nlr_this_side(
        m_weights.interactive_piece_square_NLR_parameters[moving_side]);
    const Non_Linear_Response<T> nlr_opposing_side(
        m_weights.interactive_piece_square_NLR_parameters[opposing_side]);

    // Explicit interactive term NLR values per side.
    const T nlr_this_value = nlr_this_side.value(nlr_this_interaction_value);
    const T nlr_opposing_value =
        nlr_opposing_side.value(nlr_opposing_interaction_value);

    return (nlr_this_value + nlr_this_king_value + nlr_this_queen_value
            + nlr_this_rook_value + nlr_this_bishop_value
            + nlr_this_knight_value + nlr_this_pawn_value)
         - (nlr_opposing_value + nlr_opposing_king_value
            + nlr_opposing_queen_value + nlr_opposing_rook_value
            + nlr_opposing_bishop_value + nlr_opposing_knight_value
            + nlr_opposing_pawn_value);
}

/*******************************************************************************
 *
 * HELPER FUNCTIONS FOR EVALUATOR
 *
 *******************************************************************************/

template <typename T>
template <PIECE_COLOR side>
inline T
Evaluator<T>::calculate_piece_mobility(const Moves_Bitboard_Matrix& matrix,
                                       const PIECES                 piece) const
{
    Attacks a;

    std::vector<Moves_Bitboard> moves_bitboards;
    matrix.get_piece_moves_bitboards(side, piece, moves_bitboards);
    T piece_mobility = constant_conversion(0.0);
    for (Moves_Bitboard& mb : moves_bitboards)
    {
        const Bitboard diagonal_movements =
            mb.bitboard
            & (a.get_bishop_attacks(
                mb.square,
                m_chess_board.get_both_color_occupancies()));

        const Bitboard orthogonal_movements =
            mb.bitboard
            & (a.get_rook_attacks(mb.square,
                                  m_chess_board.get_both_color_occupancies()));

        const Bitboard backward_movements =
            mb.bitboard & Bitboard::get_backward_squares_mask(mb.square, side);

        const T diagonal_mobility =
            m_weights.diagonal_mobility * diagonal_movements.high_bit_count();

        const T orthogonal_mobility = orthogonal_movements.high_bit_count()
                                    * m_weights.orthogonal_mobility;

        const T backward_mobility = backward_movements.high_bit_count()
                                  * m_weights.backwards_movement_mobility;

        const T multi_movement_mobility =
            ((diagonal_movements.high_bit_count() > 0)
             && (orthogonal_movements.high_bit_count() > 0))
            * m_weights.multi_movement_mobility;

        const T knight_movements_mobility = mb.bitboard.high_bit_count()
                                          * (piece == PIECES::KNIGHT)
                                          * m_weights.knight_movement_mobility;

        piece_mobility +=
            (diagonal_mobility + orthogonal_mobility + backward_mobility
             + multi_movement_mobility + knight_movements_mobility);
    }

    return piece_mobility;
}

template <typename T>
T Evaluator<T>::constant_conversion(const double value) const
{
    if constexpr (std::is_same_v<T, AD_Value>)
    {
        return AD_Value::constant(m_weights[0].tape, value);
    }
    else
    {
        return explicit_fp_double_conversion<T>(value);
    }
}
