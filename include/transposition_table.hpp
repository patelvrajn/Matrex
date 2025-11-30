#pragma once

#include "chess_move.hpp"
#include "score.hpp"
#include "zobrist_hash.hpp"

#define PARTIAL_ZOBRIST_MASK 0xFFFF

constexpr uint64_t DEFAULT_TRANSPOSITION_TABLE_SIZE = 64;  // MiB

enum class Score_Bound_Type : uint8_t { EXACT, LOWER_BOUND, UPPER_BOUND };

struct Transposition_Table_Entry {
  uint16_t partial_zobrist;
  uint8_t age;
  Chess_Move best_move;
  Score score;
  uint16_t depth;
  Score_Bound_Type score_bound;
};

class Transposition_Table {
 public:
  explicit Transposition_Table(
      const uint64_t size_in_mib = DEFAULT_TRANSPOSITION_TABLE_SIZE);

  void resize(const uint64_t size_in_mib);

  void prefetch(const Zobrist_Hash hash);

  bool read(const Zobrist_Hash hash, Transposition_Table_Entry& output);

  void write(const Zobrist_Hash hash, const Transposition_Table_Entry& entry);

  void clear();

  static uint16_t get_partial_zobrist(const Zobrist_Hash hash);

 private:
  Transposition_Table_Entry* m_table;
  uint64_t m_size;
};
