#include "transposition_table.hpp"

Transposition_Table::Transposition_Table(const uint64_t size_in_mib)
    : m_table(nullptr), m_size(0) {
  resize(size_in_mib);
  clear();
}

void Transposition_Table::resize(const uint64_t size_in_mib) {
  constexpr uint64_t SIZE_OF_ENTRY = sizeof(Transposition_Table_Entry);
  uint64_t size_in_bytes = size_in_mib * 1024 * 1024;
  uint64_t new_size = size_in_bytes / SIZE_OF_ENTRY;

  if (new_size == m_size) {
    return;
  }

  m_size = new_size;

  if (m_table != nullptr) {
    delete[] m_table;
  }

  m_table = new Transposition_Table_Entry[m_size];
  clear();
}

// To convert the hash to an index, we use Lemire's method described here:
// https://lemire.me/blog/2016/06/27/a-fast-alternative-to-the-modulo-reduction/
// We use the upper 48 bits of the hash for index this gives a better
// distribution than using the lower bits - reduces collisions.
uint64_t Transposition_Table::get_lemire_index(const Zobrist_Hash hash) const {
  __uint128_t product =
      (static_cast<__uint128_t>((hash.get_hash_value() & LEMIRE_INDEX_MASK) >>
                                LEMIRE_INDEX_SHIFT) *
       static_cast<__uint128_t>(m_size));
  return static_cast<uint64_t>(product >> 64);
}

void Transposition_Table::prefetch(const Zobrist_Hash hash) {
  uint64_t index = get_lemire_index(hash);
  __builtin_prefetch(&m_table[index]);
}

bool Transposition_Table::read(const Zobrist_Hash hash,
                               Transposition_Table_Entry& output) {
  uint64_t index = get_lemire_index(hash);

  if (m_table[index].partial_zobrist ==
      (hash.get_hash_value() & PARTIAL_ZOBRIST_MASK)) {
    output = m_table[index];
    return true;
  } else {
    output = Transposition_Table_Entry{};
    return false;
  }
}

void Transposition_Table::write(const Zobrist_Hash hash,
                                const Transposition_Table_Entry& entry) {
  uint64_t index = get_lemire_index(hash);
  m_table[index] = entry;
}

void Transposition_Table::clear() {
  for (uint64_t i = 0; i < m_size; ++i) {
    m_table[i] = Transposition_Table_Entry{};
  }
}

// We only store the lower 16 bits of the hash to save memory - these bits tend
// to be noiser and more correlated than the higher bits.
uint16_t Transposition_Table::get_partial_zobrist(const Zobrist_Hash hash) {
  return static_cast<uint16_t>(hash.get_hash_value() & PARTIAL_ZOBRIST_MASK);
}
