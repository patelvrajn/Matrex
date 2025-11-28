#include "transposition_table.hpp"

#define PARTIAL_ZOBRIST_MASK 0xFFFF

Transposition_Table::Transposition_Table(const uint64_t size_in_mib)
    : m_table(nullptr), m_size(0) {
  resize(size_in_mib);
  clear();
}

void Transposition_Table::resize(const uint64_t size_in_mib) {
  constexpr uint64_t SIZE_OF_ENTRY = sizeof(Transposition_Table_Entry);
  uint64_t size_in_bytes = size_in_mib * 1024 * 1024;
  m_size = size_in_bytes / SIZE_OF_ENTRY;

  if (m_table != nullptr) {
    delete[] m_table;
  }

  m_table = new Transposition_Table_Entry[m_size];
  clear();
}

void Transposition_Table::prefetch(const Zobrist_Hash hash) {
  uint64_t index = hash.get_hash_value() % m_size;
  __builtin_prefetch(&m_table[index]);
}

bool Transposition_Table::read(const Zobrist_Hash hash,
                               Transposition_Table_Entry& output) {
  uint64_t index = hash.get_hash_value() % m_size;

  if (m_table[index].partial_zobrist ==
      (hash.get_hash_value() & PARTIAL_ZOBRIST_MASK)) {
    output = m_table[index];
    return true;
  } else {
    return false;
  }
}

void Transposition_Table::write(const Zobrist_Hash hash,
                                const Transposition_Table_Entry& entry) {
  uint64_t index = hash.get_hash_value() % m_size;
  m_table[index] = entry;
}

void Transposition_Table::clear() {
  for (uint64_t i = 0; i < m_size; ++i) {
    m_table[i] = Transposition_Table_Entry{};
  }
}
