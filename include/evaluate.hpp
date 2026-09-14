#pragma once

#include <algorithm>
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

    Evaluator(const Game_Phased_Eval_Weights<T>& weights,
              const Chess_Board&                 cb,
              const Moves_Bitboard_Matrix&       moving_side_matrix,
              const Moves_Bitboard_Matrix&       opposing_side_matrix);

    template <EGAME_PHASE phase>
    T phased_evaluate_template_typed() const;

    T evaluate_template_typed() const;

    template <std::size_t corr_hist_table_size>
    Score evaluate(const Correction_History_Tables<corr_hist_table_size>&
                       corr_hist_tables) const;

    template <PIECE_COLOR moving_side, EGAME_PHASE phase>
    inline T material_score() const;

    template <PIECE_COLOR moving_side, EGAME_PHASE phase>
    inline T mobility_score(const Moves_Bitboard_Matrix& matrix) const;

    template <PIECE_COLOR moving_side, EGAME_PHASE phase>
    inline T piece_square_score() const;

  private:

    const Game_Phased_Eval_Weights<T>& m_weights;
    const Chess_Board&                 m_chess_board;
    const Moves_Bitboard_Matrix&       m_moving_side_matrix;
    const Moves_Bitboard_Matrix&       m_opposing_side_matrix;

    inline static const Multi_Array<Non_Linear_Response_Table,
                                    NUM_OF_GAME_PHASES,
                                    NUM_OF_UNIQUE_PIECES_PER_PLAYER - 1>
        m_material_nlr_tables = []
    {
        Multi_Array<Non_Linear_Response_Table,
                    NUM_OF_GAME_PHASES,
                    NUM_OF_UNIQUE_PIECES_PER_PLAYER - 1>
            tables;

        for (std::size_t phase = 0; phase < NUM_OF_GAME_PHASES; ++phase)
        {
            const auto& phase_weights =
                (phase == MIDDLE_GAME) ? TUNED_MIDDLE_GAME_MATERIAL_NLR_WEIGHTS
                                       : TUNED_END_GAME_MATERIAL_NLR_WEIGHTS;
            for (std::size_t piece = 0;
                 piece < (NUM_OF_UNIQUE_PIECES_PER_PLAYER - 1);
                 ++piece)
            {
                tables[phase][piece] =
                    Non_Linear_Response_Table(phase_weights[piece]);
            }
        }

        return tables;
    }();

    inline static const Multi_Array<Non_Linear_Response_Table,
                                    NUM_OF_GAME_PHASES,
                                    NUM_OF_PLAYERS,
                                    NUM_OF_UNIQUE_PIECES_PER_PLAYER>
        m_piece_square_nlr_tables = []
    {
        Multi_Array<Non_Linear_Response_Table,
                    NUM_OF_GAME_PHASES,
                    NUM_OF_PLAYERS,
                    NUM_OF_UNIQUE_PIECES_PER_PLAYER>
            tables;

        for (std::size_t phase = 0; phase < NUM_OF_GAME_PHASES; ++phase)
        {
            const auto& phase_weights =
                (phase == MIDDLE_GAME)
                    ? TUNED_MIDDLE_GAME_PIECE_SQUARE_NLR_WEIGHTS
                    : TUNED_END_GAME_PIECE_SQUARE_NLR_WEIGHTS;
            for (std::size_t player = 0; player < NUM_OF_PLAYERS; ++player)
            {
                for (std::size_t piece = 0;
                     piece < NUM_OF_UNIQUE_PIECES_PER_PLAYER;
                     ++piece)
                {
                    tables[phase][player][piece] =
                        Non_Linear_Response_Table(phase_weights[player][piece]);
                }
            }
        }

        return tables;
    }();

    inline static const Multi_Array<Non_Linear_Response_Table,
                                    NUM_OF_GAME_PHASES,
                                    NUM_OF_PLAYERS>
        m_piece_square_interactive_nlr_tables = []
    {
        Multi_Array<Non_Linear_Response_Table,
                    NUM_OF_GAME_PHASES,
                    NUM_OF_PLAYERS>
            tables;

        for (std::size_t phase = 0; phase < NUM_OF_GAME_PHASES; ++phase)
        {
            const auto& phase_weights =
                (phase == MIDDLE_GAME)
                    ? TUNED_MIDDLE_GAME_INTERACTIVE_PIECE_SQUARE_NLR_WEIGHTS
                    : TUNED_END_GAME_INTERACTIVE_PIECE_SQUARE_NLR_WEIGHTS;
            for (std::size_t player = 0; player < NUM_OF_PLAYERS; ++player)
            {
                tables[phase][player] =
                    Non_Linear_Response_Table(phase_weights[player]);
            }
        }

        return tables;
    }();

    inline static const Multi_Array<Non_Linear_Response_Table,
                                    NUM_OF_GAME_PHASES,
                                    NUM_OF_UNIQUE_PIECES_PER_PLAYER>
        m_mobility_nlr_tables = []
    {
        Multi_Array<Non_Linear_Response_Table,
                    NUM_OF_GAME_PHASES,
                    NUM_OF_UNIQUE_PIECES_PER_PLAYER>
            tables;

        for (std::size_t phase = 0; phase < NUM_OF_GAME_PHASES; ++phase)
        {
            const auto& phase_weights =
                (phase == MIDDLE_GAME)
                    ? TUNED_MIDDLE_GAME_PIECE_MOBILITY_NLR_WEIGHTS
                    : TUNED_END_GAME_PIECE_MOBILITY_NLR_WEIGHTS;
            for (std::size_t piece = 0; piece < NUM_OF_UNIQUE_PIECES_PER_PLAYER;
                 ++piece)
            {
                tables[phase][piece] =
                    Non_Linear_Response_Table(phase_weights[piece]);
            }
        }

        return tables;
    }();

    // Helpers
    template <PIECE_COLOR side, EGAME_PHASE phase>
    inline T calculate_piece_mobility(const Moves_Bitboard_Matrix& matrix,
                                      const PIECES                 piece) const;

    template <EGAME_PHASE phase>
    T constant_conversion(const double value) const;
};

template <typename T>
Evaluator<T>::Evaluator(const Game_Phased_Eval_Weights<T>& weights,
                        const Chess_Board&                 cb,
                        const Moves_Bitboard_Matrix&       moving_side_matrix,
                        const Moves_Bitboard_Matrix& opposing_side_matrix) :
    m_weights(weights),
    m_chess_board(cb),
    m_moving_side_matrix(moving_side_matrix),
    m_opposing_side_matrix(opposing_side_matrix)
{
}

template <typename T>
template <EGAME_PHASE phase>
T Evaluator<T>::phased_evaluate_template_typed() const
{
    PIECE_COLOR moving_side = m_chess_board.get_side_to_move();

    T material;
    T mobility;
    T piece_square;

    if (moving_side == PIECE_COLOR::WHITE)
    {
        material = material_score<PIECE_COLOR::WHITE, phase>()
                 - material_score<PIECE_COLOR::BLACK, phase>();
        mobility =
            mobility_score<PIECE_COLOR::WHITE, phase>(m_moving_side_matrix)
            - mobility_score<PIECE_COLOR::BLACK, phase>(m_opposing_side_matrix);
        piece_square = piece_square_score<PIECE_COLOR::WHITE, phase>()
                     - piece_square_score<PIECE_COLOR::BLACK, phase>();
    }
    else
    {
        material = material_score<PIECE_COLOR::BLACK, phase>()
                 - material_score<PIECE_COLOR::WHITE, phase>();
        mobility =
            mobility_score<PIECE_COLOR::BLACK, phase>(m_moving_side_matrix)
            - mobility_score<PIECE_COLOR::WHITE, phase>(m_opposing_side_matrix);
        piece_square = piece_square_score<PIECE_COLOR::BLACK, phase>()
                     - piece_square_score<PIECE_COLOR::WHITE, phase>();
    }

    const T evaluation = material + mobility + piece_square;

    return evaluation;
}

template <typename T>
T Evaluator<T>::evaluate_template_typed() const
{
    constexpr uint8_t MAXIMUM_PHASE_VALUE = 64;

    const uint64_t queen_phase_value =
        9
        * m_chess_board.get_piece_occupancies(PIECES::QUEEN)
              .high_bit_count(); // 9 * 2 = 18
    const uint64_t rook_phase_value =
        5
        * m_chess_board.get_piece_occupancies(PIECES::ROOK)
              .high_bit_count(); // 5 * 4 = 20
    const uint64_t bishop_phase_value =
        7 * m_chess_board.get_piece_occupancies(PIECES::BISHOP).high_bit_count()
        / 2; // (7 * 4) / 2 = 14
    const uint64_t knight_phase_value =
        3
        * m_chess_board.get_piece_occupancies(PIECES::KNIGHT)
              .high_bit_count(); // 3 * 4 = 12

    const uint8_t middle_game_phase = static_cast<uint8_t>(
        std::min((queen_phase_value + rook_phase_value + bishop_phase_value
                  + knight_phase_value),
                 static_cast<uint64_t>(MAXIMUM_PHASE_VALUE)));
    const uint8_t end_game_phase = MAXIMUM_PHASE_VALUE - middle_game_phase;

    const T middle_game_evaluation =
        phased_evaluate_template_typed<MIDDLE_GAME>();
    const T end_game_evaluation = phased_evaluate_template_typed<END_GAME>();

    const T tapered_evaluation = ((middle_game_evaluation * middle_game_phase)
                                  + (end_game_evaluation * end_game_phase))
                               / MAXIMUM_PHASE_VALUE;

    return tapered_evaluation;
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
template <PIECE_COLOR moving_side, EGAME_PHASE phase>
inline T Evaluator<T>::material_score() const
{
    T return_value = constant_conversion<phase>(0.0);

    for (uint8_t piece = PIECES::PAWN; piece <= PIECES::QUEEN; ++piece)
    {
        T material =
            (m_weights[phase].material[piece]
             * m_chess_board.get_piece_occupancies(moving_side, (PIECES) piece)
                   .high_bit_count());

        T non_linear_material;

        if constexpr (std::is_same_v<T, Matrex_FP_Int>)
        {
            non_linear_material =
                m_material_nlr_tables[phase][piece].lookup(material);
        }
        else
        {
            non_linear_material =
                Non_Linear_Response(
                    m_weights[phase].material_NLR_parameters[piece])
                    .value(material);
        }

        return_value += non_linear_material;
    }

    return return_value;
}

template <typename T>
template <PIECE_COLOR moving_side, EGAME_PHASE phase>
inline T Evaluator<T>::mobility_score(const Moves_Bitboard_Matrix& matrix) const
{
    T mobility = constant_conversion<phase>(0.0);

    for (uint8_t piece = PIECES::PAWN; piece <= PIECES::KING; ++piece)
    {
        const T piece_mobility =
            calculate_piece_mobility<moving_side, phase>(matrix,
                                                         (PIECES) piece);

        T non_linear_mobility;

        if constexpr (std::is_same_v<T, Matrex_FP_Int>)
        {
            non_linear_mobility =
                m_mobility_nlr_tables[phase][piece].lookup(piece_mobility);
        }
        else
        {
            non_linear_mobility =
                Non_Linear_Response(
                    m_weights[phase].piece_mobility_NLR_parameters[piece])
                    .value(piece_mobility);
        }

        mobility += non_linear_mobility;
    }

    return static_cast<T>(mobility);
}

template <typename T>
template <PIECE_COLOR moving_side, EGAME_PHASE phase>
inline T Evaluator<T>::piece_square_score() const
{
    // Accumulate the piece-square values from the piece-square tables for the
    // present state of the board for the moving side.
    Multi_Array<T, NUM_OF_UNIQUE_PIECES_PER_PLAYER> color_piece_values {};

    for (uint8_t piece = PIECES::PAWN; piece <= PIECES::KING; ++piece)
    {
        // Initialize the array value for the case of T = AD Value which
        // contains optionals.
        color_piece_values[piece] = constant_conversion<phase>(0.0);

        const Bitboard piece_occupancy =
            m_chess_board.get_piece_occupancies(moving_side, (PIECES) piece);

        for (const Square s : piece_occupancy)
        {
            color_piece_values[piece] +=
                m_weights[phase]
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
            m_piece_square_nlr_tables[phase][moving_side][PIECES::KING].lookup(
                color_piece_values[PIECES::KING]);
        nlr_this_queen_value =
            m_piece_square_nlr_tables[phase][moving_side][PIECES::QUEEN].lookup(
                color_piece_values[PIECES::QUEEN]);
        nlr_this_rook_value =
            m_piece_square_nlr_tables[phase][moving_side][PIECES::ROOK].lookup(
                color_piece_values[PIECES::ROOK]);
        nlr_this_bishop_value =
            m_piece_square_nlr_tables[phase][moving_side][PIECES::BISHOP]
                .lookup(color_piece_values[PIECES::BISHOP]);
        nlr_this_knight_value =
            m_piece_square_nlr_tables[phase][moving_side][PIECES::KNIGHT]
                .lookup(color_piece_values[PIECES::KNIGHT]);
        nlr_this_pawn_value =
            m_piece_square_nlr_tables[phase][moving_side][PIECES::PAWN].lookup(
                color_piece_values[PIECES::PAWN]);
    }
    else
    {
        // NLR objects for this side's pieces.
        const Non_Linear_Response<T> nlr_this_king(
            m_weights[phase]
                .piece_square_NLR_parameters[moving_side][PIECES::KING]);
        const Non_Linear_Response<T> nlr_this_queen(
            m_weights[phase]
                .piece_square_NLR_parameters[moving_side][PIECES::QUEEN]);
        const Non_Linear_Response<T> nlr_this_rook(
            m_weights[phase]
                .piece_square_NLR_parameters[moving_side][PIECES::ROOK]);
        const Non_Linear_Response<T> nlr_this_bishop(
            m_weights[phase]
                .piece_square_NLR_parameters[moving_side][PIECES::BISHOP]);
        const Non_Linear_Response<T> nlr_this_knight(
            m_weights[phase]
                .piece_square_NLR_parameters[moving_side][PIECES::KNIGHT]);
        const Non_Linear_Response<T> nlr_this_pawn(
            m_weights[phase]
                .piece_square_NLR_parameters[moving_side][PIECES::PAWN]);

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
            m_piece_square_interactive_nlr_tables[phase][moving_side].lookup(
                nlr_this_interaction_term);
    }
    else
    {
        const Non_Linear_Response<T> nlr_this_side(
            m_weights[phase]
                .interactive_piece_square_NLR_parameters[moving_side]);

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
template <PIECE_COLOR side, EGAME_PHASE phase>
inline T
Evaluator<T>::calculate_piece_mobility(const Moves_Bitboard_Matrix& matrix,
                                       const PIECES                 piece) const
{
    Attacks a;

    T piece_mobility = constant_conversion<phase>(0.0);
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

        const T diagonal_mobility = m_weights[phase].diagonal_mobility
                                  * diagonal_movements.high_bit_count();

        const T orthogonal_mobility = orthogonal_movements.high_bit_count()
                                    * m_weights[phase].orthogonal_mobility;

        const T backward_mobility =
            backward_movements.high_bit_count()
            * m_weights[phase].backwards_movement_mobility;

        const T multi_movement_mobility =
            ((diagonal_movements.high_bit_count() > 0)
             && (orthogonal_movements.high_bit_count() > 0))
            * m_weights[phase].multi_movement_mobility;

        const T knight_movements_mobility =
            mb.bitboard.high_bit_count() * (piece == PIECES::KNIGHT)
            * m_weights[phase].knight_movement_mobility;

        piece_mobility +=
            (diagonal_mobility + orthogonal_mobility + backward_mobility
             + multi_movement_mobility + knight_movements_mobility);
    }

    return piece_mobility;
}

template <typename T>
template <EGAME_PHASE phase>
T Evaluator<T>::constant_conversion(const double value) const
{
    if constexpr (std::is_same_v<T, AD_Value>)
    {
        return AD_Value::constant(m_weights[phase][0].tape, value);
    }
    else
    {
        return explicit_fp_double_conversion<T>(value);
    }
}
