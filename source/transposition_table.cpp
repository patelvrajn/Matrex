#include "transposition_table.hpp"

Transposition_Table::Transposition_Table(const uint64_t size_in_mib)
    : m_table({nullptr, nullptr}), m_segment_size(0) {
  resize(size_in_mib);
}

void Transposition_Table::resize(const uint64_t size_in_mib) {
  constexpr uint64_t SIZE_OF_CLUSTER = sizeof(Transposition_Table_Cluster);
  uint64_t size_in_bytes = size_in_mib * 1024 * 1024;
  uint64_t new_segment_size = (size_in_bytes / SIZE_OF_CLUSTER) / 2;

  if (new_segment_size == m_segment_size) {
    return;
  }

  m_segment_size = new_segment_size;

  if (m_table.probationary_segment != nullptr) {
    delete[] m_table.probationary_segment;
  }

  if (m_table.protected_segment != nullptr) {
    delete[] m_table.protected_segment;
  }

  m_table.probationary_segment =
      new Transposition_Table_Cluster[m_segment_size];
  m_table.protected_segment = new Transposition_Table_Cluster[m_segment_size];
  clear();
}

// To convert the hash to an index, we use Lemire's method described here:
// https://lemire.me/blog/2016/06/27/a-fast-alternative-to-the-modulo-reduction/
uint64_t Transposition_Table::get_lemire_index(const Zobrist_Hash hash) const {
  __uint128_t product = (static_cast<__uint128_t>(hash.get_hash_value()) *
                         static_cast<__uint128_t>(m_segment_size));
  return static_cast<uint64_t>(product >> 64);
}

void Transposition_Table::prefetch(const Zobrist_Hash hash) {
  uint64_t index = get_lemire_index(hash);
  __builtin_prefetch(&m_table.probationary_segment[index]);
  __builtin_prefetch(&m_table.protected_segment[index]);
}

bool Transposition_Table::is_priority_entry(
    const uint16_t max_depth, const Transposition_Table_Entry& entry) const {
  if (max_depth <= TRANSPOSITION_TABLE_DEPTH_THRESHOLD) {
    return ((entry.depth == max_depth) ||
            (entry.score_bound == Score_Bound_Type::EXACT));
  } else {
    return (
        (entry.depth >= (max_depth - TRANSPOSITION_TABLE_DEPTH_THRESHOLD)) ||
        (entry.score_bound == Score_Bound_Type::EXACT));
  }
}

void Transposition_Table::promotion_to_protected_segment(
    const uint16_t max_depth, const uint64_t index,
    const Transposition_Table_Entry& entry) {
  // Add the entry to protected segment using conditional eviction. Add the
  // demoted entry to the probationary segment only if it's a priority entry.
  Transposition_Table_Entry demoted_entry;
  if (m_table.protected_segment[index].entries.conditional_eviction(
          entry, &demoted_entry)) {
    if (is_priority_entry(max_depth, demoted_entry)) {
      Transposition_Table_Entry evicted_entry;
      m_table.probationary_segment[index].entries.conditional_eviction(
          demoted_entry, &evicted_entry);
    }
  }
}

bool Transposition_Table::read(const uint16_t max_depth,
                               const Zobrist_Hash hash,
                               Transposition_Table_Entry& output) {
  uint64_t index = get_lemire_index(hash);

  // Check protected segment first.
  uint8_t found_entry_index = 0;
  for (const Transposition_Table_Entry& e :
       m_table.protected_segment[index].entries) {
    // Is this entry a partial zobrist match?
    if (e.partial_zobrist == Transposition_Table::get_partial_zobrist(hash)) {
      // Output is the found entry.
      output = e;

      // The read entry is removed from it's current position and goes to the
      // front of the deque (LRU Policy).
      m_table.protected_segment[index].entries.move_to_front(found_entry_index);

      // We found an entry!
      return true;
    }

    found_entry_index++;
  }

  // Check probationary segment next.
  found_entry_index = 0;
  for (const Transposition_Table_Entry& e :
       m_table.probationary_segment[index].entries) {
    // Is this entry a partial zobrist match?
    if (e.partial_zobrist == Transposition_Table::get_partial_zobrist(hash)) {
      // Output is the found entry.
      output = e;

      constexpr uint8_t FRONT_OF_QUEUE = 0;

      if (found_entry_index == FRONT_OF_QUEUE) {
        // We found an entry that is at the front of the probationary segment.
        // Remove the entry that will be promoted from the probationary segment.
        // Promote this entry to the protected segment.
        m_table.probationary_segment[index].entries.remove(found_entry_index);
        promotion_to_protected_segment(max_depth, index, output);
      } else {
        // Otherwise, the entry works its way up towards the front.
        m_table.probationary_segment[index].entries.swap_up(found_entry_index);
      }

      // We found an entry.
      return true;
    }

    found_entry_index++;
  }

  return false;
}

void Transposition_Table::write(const uint16_t max_depth,
                                const Zobrist_Hash hash,
                                const Transposition_Table_Entry& entry) {
  uint64_t index = get_lemire_index(hash);

  // Loop through protected segment.
  uint8_t found_entry_index = 0;
  for (Transposition_Table_Entry& e :
       m_table.protected_segment[index].entries) {
    // Same logic for protected segment as probationary segment below.
    if (e.partial_zobrist == Transposition_Table::get_partial_zobrist(hash)) {
      if (e.depth < entry.depth) {
        e = entry;
        m_table.protected_segment[index].entries.move_to_front(
            found_entry_index);
      }
      return;
    }
    found_entry_index++;
  }

  // Loop through probationary segment.
  found_entry_index = 0;
  for (Transposition_Table_Entry& e :
       m_table.probationary_segment[index].entries) {
    // Is this probationary segment entry a partial zobrist match?
    if (e.partial_zobrist == Transposition_Table::get_partial_zobrist(hash)) {
      // Replace the entry in the probationary segment if it's depth searched is
      // less than the given entry. Move the replaced entry to the front.
      if (e.depth < entry.depth) {
        e = entry;
        m_table.probationary_segment[index].entries.move_to_front(
            found_entry_index);
      }

      // We either replaced an entry with an entry with better depth or we are
      // writing nothing either because the entry is a duplicate (hopefully) or
      // it has worse depth.
      return;
    }

    found_entry_index++;
  }

  // Priority entries can pass-thru to the protected segment.
  if (is_priority_entry(max_depth, entry)) {
    promotion_to_protected_segment(max_depth, index, entry);
    return;
  }

  // We didn't find a match and it cannot pass-thru then this is a new position
  // (hopefully), write the new entry using conditional eviction into the
  // probationary segment.
  Transposition_Table_Entry evicted_entry;
  m_table.probationary_segment[index].entries.conditional_eviction(
      entry, &evicted_entry);
}

void Transposition_Table::clear() {
  for (uint64_t cluster_index = 0; cluster_index < m_segment_size;
       ++cluster_index) {
    m_table.probationary_segment[cluster_index].entries.clear();
    m_table.protected_segment[cluster_index].entries.clear();
  }
}

// We only store the lower 16 bits of the hash to save memory.
uint16_t Transposition_Table::get_partial_zobrist(const Zobrist_Hash hash) {
  return static_cast<uint16_t>(hash.get_hash_value() & PARTIAL_ZOBRIST_MASK);
}
