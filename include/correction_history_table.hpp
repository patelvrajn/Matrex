#pragma once

#include "chess_board.hpp"
#include "fixed_point.hpp"
#include "score.hpp"

constexpr double CORR_HIST_FRACTIONAL_UPDATE = 0.15;
constexpr double CORR_HIST_EMA_DEPTH_FACTOR = (1.0 / 8.0);

constexpr Matrex_FP_Int CORR_HIST_MIN_CORRECTION = Matrex_FP_Int::from_integer(-256);
constexpr Matrex_FP_Int CORR_HIST_MAX_CORRECTION = Matrex_FP_Int::from_integer(256);

constexpr Matrex_FP_Int CORR_HIST_PAWNS_WEIGHT = Matrex_FP_Int::from_double(0.35);
constexpr Matrex_FP_Int CORR_HIST_DIAGONALS_WEIGHT = Matrex_FP_Int::from_double(0.15);
constexpr Matrex_FP_Int CORR_HIST_ORTHOGONALS_WEIGHT = Matrex_FP_Int::from_double(0.15);
constexpr Matrex_FP_Int CORR_HIST_KNIGHTS_WEIGHT = Matrex_FP_Int::from_double(0.15);
constexpr Matrex_FP_Int CORR_HIST_MATERIALS_WEIGHT = Matrex_FP_Int::from_double(0.2);

template <std::size_t size>
struct Correction_History_Tables_Per_Side
{
    using Table_Type = multi_array<Score, size>;

    Table_Type pawns_table;
    Table_Type diagonals_table;
    Table_Type orthogonals_table;
    Table_Type knights_table;
    Table_Type material_table;
};

template <std::size_t size>
using Correction_History_Tables_Per_Side_Pair = multi_array<Correction_History_Tables_Per_Side<size>, NUM_OF_PLAYERS>;

struct Correction_History_Indices
{
    std::size_t pawns_index;
    std::size_t diagonals_index;
    std::size_t orthogonals_index;
    std::size_t knights_index;
    std::size_t material_index;
};

template <std::size_t size>
class Correction_History_Tables
{
    public:

        Correction_History_Tables();

        void clear();

        void update(const Chess_Board& position, uint32_t depth_squared, Score search_score, Score static_evaluation);

        Score get_correction(const Chess_Board& position) const;

    private:

        Correction_History_Tables_Per_Side_Pair<size> m_tables;

        std::size_t get_lemire_index(Zobrist_Hash_Storage_Type hash) const;

        Correction_History_Indices get_lemire_indices(const Zobrist_Hash& hash) const;
};

template <std::size_t size>
Correction_History_Tables<size>::Correction_History_Tables()
{
    clear();
}

template <std::size_t size>
void Correction_History_Tables<size>::clear()
{
    m_tables[PIECE_COLOR::WHITE].pawns_table.fill(Score(0));
    m_tables[PIECE_COLOR::WHITE].diagonals_table.fill(Score(0));
    m_tables[PIECE_COLOR::WHITE].orthogonals_table.fill(Score(0));
    m_tables[PIECE_COLOR::WHITE].knights_table.fill(Score(0));
    m_tables[PIECE_COLOR::WHITE].material_table.fill(Score(0));

    m_tables[PIECE_COLOR::BLACK].pawns_table.fill(Score(0));
    m_tables[PIECE_COLOR::BLACK].diagonals_table.fill(Score(0));
    m_tables[PIECE_COLOR::BLACK].orthogonals_table.fill(Score(0));
    m_tables[PIECE_COLOR::BLACK].knights_table.fill(Score(0));
    m_tables[PIECE_COLOR::BLACK].material_table.fill(Score(0));
}

template <std::size_t size>
void Correction_History_Tables<size>::update(const Chess_Board& position, uint32_t depth, Score search_score, Score static_evaluation)
{
    // Calculate correction for the difference between the search score and the
    // static evaluation scaled by depth.
    const Matrex_FP_Int difference = (search_score - static_evaluation).to_fixed_point();
    const double depth_multiplier = static_cast<double>(depth) * CORR_HIST_EMA_DEPTH_FACTOR;
    const Matrex_FP_Int correction = Matrex_FP_Int::adjustable_clamp((difference * depth_multiplier), CORR_HIST_MIN_CORRECTION, CORR_HIST_MAX_CORRECTION);

    const PIECE_COLOR side_to_move = position.get_side_to_move();

    const Correction_History_Indices indices = get_lemire_indices(position.get_zobrist_hash());

    // Grab the correction history entries for this position.
    Score& pawns_entry = m_tables[side_to_move].pawns_table[indices.pawns_index];
    Score& diagonals_entry = m_tables[side_to_move].diagonals_table[indices.diagonals_index];
    Score& orthogonals_entry = m_tables[side_to_move].orthogonals_table[indices.orthogonals_index];
    Score& knights_entry = m_tables[side_to_move].knights_table[indices.knights_index];
    Score& material_entry = m_tables[side_to_move].material_table[indices.material_index];

    // Establish the lambda that will return the new correction based on an 
    // exponential moving average formula.
    const auto update_entry = [&](Score& entry) 
    {
        entry += Score(correction * CORR_HIST_FRACTIONAL_UPDATE);
    };

    // Update correction history entries.
    update_entry(pawns_entry);
    update_entry(diagonals_entry);
    update_entry(orthogonals_entry);
    update_entry(knights_entry);
    update_entry(material_entry);
}

template <std::size_t size>
Score Correction_History_Tables<size>::get_correction(const Chess_Board& position) const
{
    const PIECE_COLOR side_to_move = position.get_side_to_move();

    const Correction_History_Indices indices = get_lemire_indices(position.get_zobrist_hash());

    // Grab the correction history entries for this position.
    const auto pawns_entry = m_tables[side_to_move].pawns_table[indices.pawns_index].to_fixed_point();
    const auto diagonals_entry = m_tables[side_to_move].diagonals_table[indices.diagonals_index].to_fixed_point();
    const auto orthogonals_entry = m_tables[side_to_move].orthogonals_table[indices.orthogonals_index].to_fixed_point();
    const auto knights_entry = m_tables[side_to_move].knights_table[indices.knights_index].to_fixed_point();
    const auto material_entry = m_tables[side_to_move].material_table[indices.material_index].to_fixed_point();

    const auto correction = (pawns_entry * CORR_HIST_PAWNS_WEIGHT) + (diagonals_entry * CORR_HIST_DIAGONALS_WEIGHT)
    + (orthogonals_entry * CORR_HIST_ORTHOGONALS_WEIGHT) + (knights_entry * CORR_HIST_KNIGHTS_WEIGHT)
    + (material_entry * CORR_HIST_MATERIALS_WEIGHT);

    return Score(correction);
}

template <std::size_t size>
std::size_t Correction_History_Tables<size>::get_lemire_index(Zobrist_Hash_Storage_Type hash) const
{
    const __uint128_t product = (static_cast<__uint128_t>(hash) * static_cast<__uint128_t>(size));
    return static_cast<std::size_t>(product >> 64);
}

template <std::size_t size>
Correction_History_Indices Correction_History_Tables<size>::get_lemire_indices(const Zobrist_Hash& hash) const
{
    const Correction_History_Hashes hashes = hash.get_corr_hist_hashes();

    return 
    {
        .pawns_index = get_lemire_index(hashes.pawns_key),
        .diagonals_index = get_lemire_index(hashes.diagonals_key),
        .orthogonals_index = get_lemire_index(hashes.orthogonals_key),
        .knights_index = get_lemire_index(hashes.knights_key),
        .material_index = get_lemire_index(hashes.material_key)
    };
}
