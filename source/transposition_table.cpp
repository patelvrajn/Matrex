#include "transposition_table.hpp"

Transposition_Table::Transposition_Table(const uint64_t size_in_mib) :
    m_table(nullptr), m_size(0)
{
    resize(size_in_mib);
}

void Transposition_Table::resize(const uint64_t size_in_mib)
{
    constexpr uint64_t SIZE_OF_CLUSTER = sizeof(Transposition_Table_Cluster);
    uint64_t           size_in_bytes   = size_in_mib * 1024 * 1024;
    uint64_t           new_size        = (size_in_bytes / SIZE_OF_CLUSTER);

    if (new_size == m_size) { return; }

    m_size = new_size;

    if (m_table != nullptr) { delete[] m_table; }

    m_table = new Transposition_Table_Cluster[m_size];
    clear();
}

// To convert the hash to an index, we use Lemire's method described here:
// https://lemire.me/blog/2016/06/27/a-fast-alternative-to-the-modulo-reduction/
uint64_t Transposition_Table::get_lemire_index(const Zobrist_Hash& hash) const
{
    __uint128_t product = (static_cast<__uint128_t>(hash.get_hash_value())
                           * static_cast<__uint128_t>(m_size));
    return static_cast<uint64_t>(product >> 64);
}

bool Transposition_Table::is_priority_entry(
    const uint16_t                   max_depth,
    const Transposition_Table_Entry& entry) const
{
    if (max_depth <= TRANSPOSITION_TABLE_DEPTH_THRESHOLD)
    {
        return ((entry.depth == max_depth)
                || (entry.score_bound == Score_Bound_Type::EXACT));
    }
    else
    {
        return (
            (entry.depth >= (max_depth - TRANSPOSITION_TABLE_DEPTH_THRESHOLD))
            || (entry.score_bound == Score_Bound_Type::EXACT));
    }
}

void Transposition_Table::promotion_to_protected_segment(
    const uint16_t                   max_depth,
    const uint64_t                   index,
    const Transposition_Table_Entry& entry)
{
    // Add the entry to protected segment using conditional eviction. Add the
    // demoted entry to the probationary segment only if it's a priority entry.
    Transposition_Table_Entry demoted_entry;
    if (m_table[index].protected_entries.conditional_eviction(entry,
                                                              &demoted_entry))
    {
        if (is_priority_entry(max_depth, demoted_entry))
        {
            Transposition_Table_Entry evicted_entry;
            m_table[index].probationary_entries.conditional_eviction(
                demoted_entry,
                &evicted_entry);
        }
    }
}

bool Transposition_Table::should_replace_matched_entry(
    const Transposition_Table_Entry& existing_entry,
    const Transposition_Table_Entry& new_entry)
{
    return ((existing_entry.depth < new_entry.depth)
            || (new_entry.score_bound == Score_Bound_Type::EXACT));
}

// When writing mate scores determined by this node's tree to the transposition
// table, the mating score will be relative to the root node of the search tree,
// however, when reading mate scores from the transposition table we need the
// mate score of the position to be relative to the current node. Thus, we add
// on the ply of the current node to make it relative since the mate score will
// be (mate_min - distance from root) thus, adding the distance of this node to
// the root will make it relative.
template <bool is_read>
void Transposition_Table::make_mate_score_relative_to_node(
    Transposition_Table_Entry& entry,
    const uint16_t             node_ply) const
{
    if (!entry.score.is_mating_score()) { return; }

    constexpr bool is_write = !is_read;

    const auto numerical_score = entry.score.to_fixed_point();

    const auto ply = static_cast<Fixed_Point_Int_Storage_Type>(node_ply);

    if (entry.score.is_friendly_mate() && is_write)
    {
        entry.score = Score(numerical_score + ply);
    }

    if (entry.score.is_enemy_mate() && is_write)
    {
        entry.score = Score(numerical_score - ply);
    }

    if (entry.score.is_friendly_mate() && is_read)
    {
        entry.score = Score(numerical_score - ply);
    }

    if (entry.score.is_enemy_mate() && is_read)
    {
        entry.score = Score(numerical_score + ply);
    }
}

bool Transposition_Table::read(const uint16_t             max_depth,
                               const uint16_t             ply,
                               const Zobrist_Hash&        hash,
                               Transposition_Table_Entry& output)
{
    uint64_t index = get_lemire_index(hash);

#if COLLECT_TT_STATISTICS == 1
    m_statistics.nodes_read++;
#endif

    // Check protected segment first.
    uint8_t found_entry_index = 0;
    for (const Transposition_Table_Entry& e : m_table[index].protected_entries)
    {
        // Is this entry a partial zobrist match?
        if (e.partial_zobrist == Transposition_Table::get_partial_zobrist(hash))
        {
            // Output is the found entry.
            output = e;

            // The read entry is removed from it's current position and goes to
            // the front of the deque (LRU Policy).
            m_table[index].protected_entries.move_to_front(found_entry_index);

#if COLLECT_TT_STATISTICS == 1
            ++m_statistics.protected_hits;
#endif

            make_mate_score_relative_to_node<true>(output, ply);

            // We found an entry!
            return true;
        }

        ++found_entry_index;
    }

#if COLLECT_TT_STATISTICS == 1
    ++m_statistics.protected_misses;
#endif

    // Check probationary segment next.
    found_entry_index = 0;
    for (const Transposition_Table_Entry& e :
         m_table[index].probationary_entries)
    {
        // Is this entry a partial zobrist match?
        if (e.partial_zobrist == Transposition_Table::get_partial_zobrist(hash))
        {
            // Output is the found entry.
            output = e;

            constexpr uint8_t FRONT_OF_QUEUE = 0;

            if (found_entry_index == FRONT_OF_QUEUE)
            {
                // We found an entry that is at the front of the probationary
                // segment. Remove the entry that will be promoted from the
                // probationary segment. Promote this entry to the protected
                // segment.
                m_table[index].probationary_entries.remove(found_entry_index);
                promotion_to_protected_segment(max_depth, index, output);
            }
            else
            {
                // Otherwise, the entry works its way up towards the front.
                m_table[index].probationary_entries.swap_up(found_entry_index);
            }

#if COLLECT_TT_STATISTICS == 1
            ++m_statistics.probationary_hits;
#endif

            make_mate_score_relative_to_node<true>(output, ply);

            // We found an entry.
            return true;
        }

        ++found_entry_index;
    }

#if COLLECT_TT_STATISTICS == 1
    ++m_statistics.probationary_misses;
#endif

    return false;
}

void Transposition_Table::write(const uint16_t             max_depth,
                                const uint16_t             ply,
                                const Zobrist_Hash&        hash,
                                Transposition_Table_Entry& entry)
{
    uint64_t index = get_lemire_index(hash);

    make_mate_score_relative_to_node<false>(entry, ply);

#if COLLECT_TT_STATISTICS == 1
    ++m_statistics.nodes_written;
#endif

    // Loop through protected segment.
    uint8_t found_entry_index = 0;
    for (Transposition_Table_Entry& e : m_table[index].protected_entries)
    {
        // Same logic for protected segment as probationary segment below.
        if (e.partial_zobrist == Transposition_Table::get_partial_zobrist(hash))
        {
            if (should_replace_matched_entry(e, entry))
            {
                e = entry;
                m_table[index].protected_entries.move_to_front(
                    found_entry_index);
            }
            return;
        }
        ++found_entry_index;
    }

    // Loop through probationary segment.
    found_entry_index = 0;
    for (Transposition_Table_Entry& e : m_table[index].probationary_entries)
    {
        // Is this probationary segment entry a partial zobrist match?
        if (e.partial_zobrist == Transposition_Table::get_partial_zobrist(hash))
        {
            // Replace the entry in the probationary segment if it's depth
            // searched is less than the given entry. Move the replaced entry to
            // the front.
            if (should_replace_matched_entry(e, entry))
            {
                e = entry;
                m_table[index].probationary_entries.move_to_front(
                    found_entry_index);
            }

            // We either replaced an entry with an entry with better depth or we
            // are writing nothing either because the entry is a duplicate
            // (hopefully) or it has worse depth.
            return;
        }

        ++found_entry_index;
    }

    // Priority entries can pass-thru to the protected segment.
    if (is_priority_entry(max_depth, entry))
    {
        promotion_to_protected_segment(max_depth, index, entry);
        return;
    }

    // We didn't find a match and it cannot pass-thru then this is a new
    // position (hopefully), write the new entry using conditional eviction into
    // the probationary segment.
    Transposition_Table_Entry evicted_entry;
    m_table[index].probationary_entries.conditional_eviction(entry,
                                                             &evicted_entry);
}

void Transposition_Table::clear()
{
    for (uint64_t cluster_index = 0; cluster_index < m_size; ++cluster_index)
    {
        m_table[cluster_index].protected_entries.clear();
        m_table[cluster_index].probationary_entries.clear();
    }
}

// We only store the lower 16 bits of the hash to save memory.
uint16_t Transposition_Table::get_partial_zobrist(const Zobrist_Hash& hash)
{
    return static_cast<uint16_t>(hash.get_hash_value() & PARTIAL_ZOBRIST_MASK);
}

const Transposition_Table_Statistics&
Transposition_Table::get_statistics() const
{
#if COLLECT_TT_STATISTICS == 1
    return m_statistics;
#else
    static const Transposition_Table_Statistics empty_stats {};
    return empty_stats;
#endif
}

void Transposition_Table::clear_statistics()
{
#if COLLECT_TT_STATISTICS == 1
    m_statistics = Transposition_Table_Statistics();
#endif
}
