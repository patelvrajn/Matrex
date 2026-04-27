#pragma once

#include "chess_move.hpp"
#include "score.hpp"
#include "zobrist_hash.hpp"

/*
Transposition Table With Depth-Prioritized Segmented LRU

In any segment - if a position is found and the depth is greater for the entry
being made - replace immediately.

Probationary Segment:
  New items enter here.
        When an item is read here, it is promoted from the probationary segment
to the protected segment. When writing a new entry and the cluster here is
already full OR an item is demoted and the cluster is full, evict the LRU item
in the cluster.

Protected Segment:
  Promoted items enter here.
        When an item is promoted here and the cluster is full, demote the LRU
item to the probationary segment.
*/

template <typename T, uint8_t capacity>
class Mini_Queue {
 public:
  class Iterator {
   public:
    Iterator(const Mini_Queue* owner, uint8_t index, uint8_t remaining);
    T& operator*() const;
    Iterator& operator++();
    bool operator==(const Iterator& other) const;
    bool operator!=(const Iterator& other) const;

   private:
    Mini_Queue* m_owner = nullptr;
    uint8_t m_index = 0;

    // We need m_remaining to solve the issue of m_front == m_back when the
    // queue is full, if m_index was used to determine equality - a full
    // queue would not be iterated over.
    uint8_t m_remaining = 0;

    uint8_t wrap_index(uint8_t index) const;
  };

  Iterator begin() const;
  Iterator end() const;

  bool is_empty() const;
  bool is_full() const;
  uint8_t size() const;
  constexpr uint8_t get_capacity() const;
  void clear();
  T& front();
  T& back();

  void push(const T& value);  // Pushes to the front only.
  void pop();                 // Pops from the back only.
  bool conditional_eviction(const T& value, T* evicted_value);
  void swap_up(uint8_t index);
  void move_to_front(uint8_t index);
  void remove(uint8_t index);

 private:
  std::array<T, capacity> m_queue{};

  // m_front = index of the current front element
  // m_back  = index one past the current back element
  // m_size  = number of elements currently in the queue (needed for
  // empty/full conditions)
  uint8_t m_front = 0;
  uint8_t m_back = 0;
  uint8_t m_size = 0;

  uint8_t wrap_index(uint8_t index) const;
};

constexpr uint64_t PARTIAL_ZOBRIST_MASK = 0xFFFF;

constexpr uint8_t TRANSPOSITION_TABLE_CLUSTER_SIZE = 4;

constexpr uint16_t TRANSPOSITION_TABLE_DEPTH_THRESHOLD = 1;

constexpr uint64_t DEFAULT_TRANSPOSITION_TABLE_SIZE = 64;  // MiB

enum class Score_Bound_Type : uint8_t { EXACT, LOWER_BOUND, UPPER_BOUND };

struct Transposition_Table_Entry {
  Chess_Move best_move;
  uint16_t partial_zobrist;
  Score score;
  uint16_t depth;
  Score_Bound_Type score_bound;
};

struct Transposition_Table_Cluster {
  Mini_Queue<Transposition_Table_Entry, TRANSPOSITION_TABLE_CLUSTER_SIZE>
      entries;
};

struct Transposition_Table_Segments {
  Transposition_Table_Cluster* probationary_segment;
  Transposition_Table_Cluster* protected_segment;
};

class Transposition_Table {
 public:
  explicit Transposition_Table(
      const uint64_t size_in_mib = DEFAULT_TRANSPOSITION_TABLE_SIZE);

  void resize(const uint64_t size_in_mib);

  void prefetch(const Zobrist_Hash hash);

  bool read(const uint16_t max_depth, const Zobrist_Hash hash,
            Transposition_Table_Entry& output);

  void write(const uint16_t max_depth, const Zobrist_Hash hash,
             const Transposition_Table_Entry& entry);

  void clear();

  static uint16_t get_partial_zobrist(const Zobrist_Hash hash);

 private:
  Transposition_Table_Segments m_table;
  uint64_t m_segment_size;

  uint64_t get_lemire_index(const Zobrist_Hash hash) const;

  bool is_priority_entry(const uint16_t max_depth,
                         const Transposition_Table_Entry& entry) const;

  void promotion_to_protected_segment(const uint16_t max_depth,
                                      const uint64_t index,
                                      const Transposition_Table_Entry& entry);
};

// Note: This logic limits the capacity to being powers of two. If it is needed
// to support non-powers of two, the function can be modified to use modulus
// instead of bitwise AND.
template <typename T, uint8_t capacity>
uint8_t Mini_Queue<T, capacity>::wrap_index(uint8_t index) const {
  return (index & (capacity - 1));
}

template <typename T, uint8_t capacity>
bool Mini_Queue<T, capacity>::is_empty() const {
  return (m_size == 0);
}

template <typename T, uint8_t capacity>
bool Mini_Queue<T, capacity>::is_full() const {
  return (m_size == capacity);
}

template <typename T, uint8_t capacity>
uint8_t Mini_Queue<T, capacity>::size() const {
  return m_size;
}

template <typename T, uint8_t capacity>
constexpr uint8_t Mini_Queue<T, capacity>::get_capacity() const {
  return capacity;
}

template <typename T, uint8_t capacity>
void Mini_Queue<T, capacity>::clear() {
  m_front = 0;
  m_back = 0;
  m_size = 0;
}

template <typename T, uint8_t capacity>
T& Mini_Queue<T, capacity>::front() {
  if (is_empty()) {
    throw std::runtime_error(
        "Mini_Queue is empty there is no front of an empty queue.");
  }
  return m_queue[m_front];
}

template <typename T, uint8_t capacity>
T& Mini_Queue<T, capacity>::back() {
  if (is_empty()) {
    throw std::runtime_error(
        "Mini_Queue is empty there is no back of an empty queue.");
  }
  return m_queue[wrap_index(static_cast<uint8_t>(m_back - 1))];
}

template <typename T, uint8_t capacity>
void Mini_Queue<T, capacity>::push(const T& value) {
  if (is_full()) {
    return;
  }

  m_front = wrap_index(static_cast<uint8_t>(m_front - 1));
  m_queue[m_front] = value;

  if (m_size == 0) {
    m_back = wrap_index(static_cast<uint8_t>(m_front + 1));
  }

  ++m_size;
}

template <typename T, uint8_t capacity>
void Mini_Queue<T, capacity>::pop() {
  if (is_empty()) {
    return;
  }

  if (m_size == 1) {
    clear();
    return;
  }

  m_back = wrap_index(static_cast<uint8_t>(m_back - 1));
  --m_size;
}

template <typename T, uint8_t capacity>
bool Mini_Queue<T, capacity>::conditional_eviction(const T& value,
                                                   T* evicted_value) {
  if (!is_full()) {
    push(value);
    return false;  // No eviction occurred.
  }

  const uint8_t eviction_index = wrap_index(static_cast<uint8_t>(m_back - 1));

  if (evicted_value) {
    *evicted_value = std::move(m_queue[eviction_index]);
  }

  // Insert a new element in front.
  m_front = wrap_index(static_cast<uint8_t>(m_front - 1));
  m_queue[m_front] = value;

  // Evict the back element by moving the back index back by one.
  m_back = eviction_index;

  return true;  // Eviction occurred.
}

template <typename T, uint8_t capacity>
void Mini_Queue<T, capacity>::move_to_front(uint8_t index) {
  if ((index >= m_size) || (index == 0)) {
    return;
  }

  const uint8_t wrapped_index =
      wrap_index(static_cast<uint8_t>(m_front + index));

  T saved = std::move(m_queue[wrapped_index]);

  for (uint8_t i = index; i != 0; --i) {
    const uint8_t dst = wrap_index(static_cast<uint8_t>(m_front + i));
    const uint8_t src = wrap_index(static_cast<uint8_t>(m_front + i - 1));

    m_queue[dst] = std::move(m_queue[src]);
  }

  m_queue[m_front] = std::move(saved);
}

// Given an index into the queue, swap the element at that index with the
// element in front of it in the queue.
template <typename T, uint8_t capacity>
void Mini_Queue<T, capacity>::swap_up(uint8_t index) {
  if ((m_size <= 1) || (index == 0)) {
    return;
  }

  const uint8_t wrapped_current_index =
      wrap_index(static_cast<uint8_t>(m_front + index));
  const uint8_t wrapped_next_index =
      wrap_index(static_cast<uint8_t>(m_front + index - 1));

  std::swap(m_queue[wrapped_current_index], m_queue[wrapped_next_index]);
}

template <typename T, uint8_t capacity>
void Mini_Queue<T, capacity>::remove(uint8_t index) {
  if (index >= m_size) {
    return;
  }

  if (m_size == 1) {
    clear();
    return;
  }

  // Pop from the front.
  if (index == 0) {
    m_front = wrap_index(static_cast<uint8_t>(m_front + 1));
    --m_size;
    return;
  }

  if (index == static_cast<uint8_t>(m_size - 1)) {
    pop();
    return;
  }

  // Number of elements before the erased one.
  const uint8_t left_count = index;

  // Number of elements after the erased one.
  const uint8_t right_count = static_cast<uint8_t>(m_size - 1 - index);

  // Shift the elements on the left side of the index one step toward the back.
  if (left_count < right_count) {
    for (uint8_t i = index; i != 0; --i) {
      const uint8_t dst = wrap_index(static_cast<uint8_t>(m_front + i));
      const uint8_t src = wrap_index(static_cast<uint8_t>(m_front + i - 1));

      m_queue[dst] = std::move(m_queue[src]);
    }

    m_front = wrap_index(static_cast<uint8_t>(m_front + 1));

    // Shift the elements on the right side of index one step toward the front.
  } else {
    for (uint8_t i = index; i != static_cast<uint8_t>(m_size - 1); ++i) {
      const uint8_t dst = wrap_index(static_cast<uint8_t>(m_front + i));
      const uint8_t src = wrap_index(static_cast<uint8_t>(m_front + i + 1));

      m_queue[dst] = std::move(m_queue[src]);
    }

    m_back = wrap_index(static_cast<uint8_t>(m_back - 1));
  }

  --m_size;
}

template <typename T, uint8_t capacity>
Mini_Queue<T, capacity>::Iterator::Iterator(const Mini_Queue* owner,
                                            uint8_t index, uint8_t remaining)
    : m_owner(const_cast<Mini_Queue*>(owner)),
      m_index(index),
      m_remaining(remaining) {}

template <typename T, uint8_t capacity>
T& Mini_Queue<T, capacity>::Iterator::operator*() const {
  return m_owner->m_queue[m_index];
}

template <typename T, uint8_t capacity>
uint8_t Mini_Queue<T, capacity>::Iterator::wrap_index(uint8_t index) const {
  return (index & (m_owner->get_capacity() - 1));
}

template <typename T, uint8_t capacity>
typename Mini_Queue<T, capacity>::Iterator&
Mini_Queue<T, capacity>::Iterator::operator++() {
  if (m_remaining != 0) {
    m_index = wrap_index(static_cast<uint8_t>(m_index + 1));
    --m_remaining;
  }
  return *this;
}

template <typename T, uint8_t capacity>
bool Mini_Queue<T, capacity>::Iterator::operator==(
    const Iterator& other) const {
  return ((m_owner == other.m_owner) && (m_remaining == other.m_remaining));
}

template <typename T, uint8_t capacity>
bool Mini_Queue<T, capacity>::Iterator::operator!=(
    const Iterator& other) const {
  return !(*this == other);
}

template <typename T, uint8_t capacity>
Mini_Queue<T, capacity>::Iterator Mini_Queue<T, capacity>::begin() const {
  return Mini_Queue::Iterator(this, m_front, m_size);
}

template <typename T, uint8_t capacity>
Mini_Queue<T, capacity>::Iterator Mini_Queue<T, capacity>::end() const {
  return Mini_Queue::Iterator(this, m_back, 0);
}
