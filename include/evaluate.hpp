#pragma once

#include <cmath>

#include "chess_board.hpp"
#include "globals.hpp"
#include "move_generator.hpp"
#include "non_linear_response.hpp"
#include "score.hpp"
#include "correction_history_table.hpp"
#include "evaluation_weights.hpp"
#include "evaluation_terms.hpp"

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
    inline T mobility_score(const Moves_Bitboard_Matrix& matrix) const;

    template <PIECE_COLOR moving_side>
    inline T piece_square_score() const;

  private:

    const Evaluation_Weights<T>& m_weights;
    const Chess_Board&           m_chess_board;
    const Moves_Bitboard_Matrix& m_moving_side_matrix;
    const Moves_Bitboard_Matrix& m_opposing_side_matrix;

    inline static const Multi_Array<Non_Linear_Response_Table,
                                    NUM_OF_UNIQUE_PIECES_PER_PLAYER - 1>
        m_material_nlr_tables = []
    {
        Multi_Array<Non_Linear_Response_Table,
                    NUM_OF_UNIQUE_PIECES_PER_PLAYER - 1>
            tables;

        for (size_t piece = 0; piece < tables.size; ++piece)
        {
            tables[piece] =
                Non_Linear_Response_Table(TUNED_MATERIAL_NLR_WEIGHTS[piece]);
        }

        return tables;
    }();

    inline static const Multi_Array<Non_Linear_Response_Table,
                                    NUM_OF_PLAYERS,
                                    NUM_OF_UNIQUE_PIECES_PER_PLAYER>
        m_piece_square_nlr_tables = []
    {
        Multi_Array<Non_Linear_Response_Table,
                    NUM_OF_PLAYERS,
                    NUM_OF_UNIQUE_PIECES_PER_PLAYER>
            tables;

        for (size_t player = 0; player < NUM_OF_PLAYERS; ++player)
        {
            for (size_t piece = 0; piece < NUM_OF_UNIQUE_PIECES_PER_PLAYER;
                 ++piece)
            {
                tables[player][piece] = Non_Linear_Response_Table(
                    TUNED_PIECE_SQUARE_NLR_WEIGHTS[player][piece]);
            }
        }

        return tables;
    }();

    inline static const Multi_Array<Non_Linear_Response_Table, NUM_OF_PLAYERS>
        m_piece_square_interactive_nlr_tables = []
    {
        Multi_Array<Non_Linear_Response_Table, NUM_OF_PLAYERS> tables;

        for (size_t player = 0; player < NUM_OF_PLAYERS; ++player)
        {
            tables[player] = Non_Linear_Response_Table(
                TUNED_INTERACTIVE_PIECE_SQUARE_NLR_WEIGHTS[player]);
        }

        return tables;
    }();

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
    // T mobility;
    T piece_square;

    if (moving_side == PIECE_COLOR::WHITE)
    {
        material = material_score<PIECE_COLOR::WHITE>()
                 - material_score<PIECE_COLOR::BLACK>();
        // mobility = mobility_score<PIECE_COLOR::WHITE>(m_moving_side_matrix)
        //          -
        //          mobility_score<PIECE_COLOR::BLACK>(m_opposing_side_matrix);
        piece_square = piece_square_score<PIECE_COLOR::WHITE>()
                     - piece_square_score<PIECE_COLOR::BLACK>();
    }
    else
    {
        material = material_score<PIECE_COLOR::BLACK>()
                 - material_score<PIECE_COLOR::WHITE>();
        // mobility = mobility_score<PIECE_COLOR::BLACK>(m_moving_side_matrix)
        //          -
        //          mobility_score<PIECE_COLOR::WHITE>(m_opposing_side_matrix);
        piece_square = piece_square_score<PIECE_COLOR::BLACK>()
                     - piece_square_score<PIECE_COLOR::WHITE>();
    }

    const T evaluation = material + piece_square;
    // const T evaluation = material + mobility + piece_square;

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
    T return_value = constant_conversion(0.0);

    for (uint8_t piece = PIECES::PAWN; piece <= PIECES::QUEEN; ++piece)
    {
        T material =
            (m_weights.material[piece]
             * m_chess_board.get_piece_occupancies(moving_side, (PIECES) piece)
                   .high_bit_count());

        T non_linear_material;

        if constexpr (std::is_same_v<T, Matrex_FP_Int>)
        {
            non_linear_material = m_material_nlr_tables[piece].lookup(material);
        }
        else
        {
            non_linear_material =
                Non_Linear_Response(m_weights.material_NLR_parameters[piece])
                    .value(material);
        }

        return_value += non_linear_material;
    }

    return return_value;
}

template <typename T>
template <PIECE_COLOR moving_side>
inline T Evaluator<T>::mobility_score(const Moves_Bitboard_Matrix& matrix) const
{
    T mobility = constant_conversion(0.0);

    for (uint8_t piece = PIECES::PAWN; piece <= PIECES::KING; ++piece)
    {
        const T piece_mobility =
            calculate_piece_mobility<moving_side>(matrix, (PIECES) piece);

        mobility +=
            Non_Linear_Response(m_weights.piece_mobility_NLR_parameters[piece])
                .value(piece_mobility);
    }

    return static_cast<T>(mobility);
}

template <typename T>
template <PIECE_COLOR moving_side>
inline T Evaluator<T>::piece_square_score() const
{
    // Accumulate the piece-square values from the piece-square tables for the
    // present state of the board for the moving side.
    Multi_Array<T, NUM_OF_UNIQUE_PIECES_PER_PLAYER> color_piece_values {};

    for (uint8_t piece = PIECES::PAWN; piece <= PIECES::KING; ++piece)
    {
        // Initialize the array value for the case of T = AD Value which
        // contains optionals.
        color_piece_values[piece] = constant_conversion(0.0);

        const Bitboard piece_occupancy =
            m_chess_board.get_piece_occupancies(moving_side, (PIECES) piece);

        for (const Square s : piece_occupancy)
        {
            color_piece_values[piece] +=
                m_weights
                    .piece_square_tables[moving_side][piece][s.get_index()];
        }
    }

    T nlr_this_king_value;
    T nlr_this_queen_value;
    T nlr_this_rook_value;
    T nlr_this_bishop_value;
    T nlr_this_knight_value;
    T nlr_this_pawn_value;
    T nlr_this_interaction_value;

    if constexpr (std::is_same_v<T, Matrex_FP_Int>)
    {
        // NLR values for this side's pieces.
        nlr_this_king_value =
            m_piece_square_nlr_tables[moving_side][PIECES::KING].lookup(
                color_piece_values[PIECES::KING]);
        nlr_this_queen_value =
            m_piece_square_nlr_tables[moving_side][PIECES::QUEEN].lookup(
                color_piece_values[PIECES::QUEEN]);
        nlr_this_rook_value =
            m_piece_square_nlr_tables[moving_side][PIECES::ROOK].lookup(
                color_piece_values[PIECES::ROOK]);
        nlr_this_bishop_value =
            m_piece_square_nlr_tables[moving_side][PIECES::BISHOP].lookup(
                color_piece_values[PIECES::BISHOP]);
        nlr_this_knight_value =
            m_piece_square_nlr_tables[moving_side][PIECES::KNIGHT].lookup(
                color_piece_values[PIECES::KNIGHT]);
        nlr_this_pawn_value =
            m_piece_square_nlr_tables[moving_side][PIECES::PAWN].lookup(
                color_piece_values[PIECES::PAWN]);
    }
    else
    {
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

        nlr_this_king_value =
            nlr_this_king.value(color_piece_values[PIECES::KING]);
        nlr_this_queen_value =
            nlr_this_queen.value(color_piece_values[PIECES::QUEEN]);
        nlr_this_rook_value =
            nlr_this_rook.value(color_piece_values[PIECES::ROOK]);
        nlr_this_bishop_value =
            nlr_this_bishop.value(color_piece_values[PIECES::BISHOP]);
        nlr_this_knight_value =
            nlr_this_knight.value(color_piece_values[PIECES::KNIGHT]);
        nlr_this_pawn_value =
            nlr_this_pawn.value(color_piece_values[PIECES::PAWN]);
    }

    // Explicit interactive term.
    const T nlr_this_interaction_term =
        nlr_this_king_value * nlr_this_queen_value * nlr_this_rook_value
        * nlr_this_bishop_value * nlr_this_knight_value * nlr_this_pawn_value;

    if constexpr (std::is_same_v<T, Matrex_FP_Int>)
    {
        nlr_this_interaction_value =
            m_piece_square_interactive_nlr_tables[moving_side].lookup(
                nlr_this_interaction_term);
    }
    else
    {
        const Non_Linear_Response<T> nlr_this_side(
            m_weights.interactive_piece_square_NLR_parameters[moving_side]);

        nlr_this_interaction_value =
            nlr_this_side.value(nlr_this_interaction_term);
    }

    return (nlr_this_interaction_value + nlr_this_king_value
            + nlr_this_queen_value + nlr_this_rook_value + nlr_this_bishop_value
            + nlr_this_knight_value + nlr_this_pawn_value);
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

    T piece_mobility = constant_conversion(0.0);
    for (const Moves_Bitboard& mb : matrix.get_iterable(side, piece))
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
