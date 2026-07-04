#pragma once

#include "chess_move.hpp"
#include "score.hpp"
#include "zobrist_hash.hpp"

/*
Segmented LRU (Least Recently Used) Transposition Table

The idea behind this transposition table architecture comes from cache
architectures used in computer systems. The idea is to protect any entries that
are likely to be important to the current search from being evicted from the
tranposition table. It is implemented by a table of clusters where each cluster
contains two queues, one for protected entries and one for probationary entries.

In any segment - if a position is found (matched entry) and the depth is greater
for the entry being written or the entry being written is an an exact score -
replace on a write.

A priority entry is defined as any entry with a depth within a certain range of
the current search depth or the entry is an exact score.

Probationary Segment - this segment is where new entries enter or demoted
priority entries are placed. When an item is read here it is moved foward one
position in the queue, if an item is already at the front of the queue it is
promoted to the protected segment. When writing a new entry and the queue is
full, evict the LRU entry, that is the entry at the back of the queue. If the
queue is not full, push the new entry to the front of the queue.

Protected Segment - this segment is where either promoted entries are written or
priority entries bypassing the probationary segment enter. When an item is read
here it is moved to the front of the queue. When an item is written here and the
queue is full, evict the LRU item and only if it is a priority entry demote it
to the probationary segment where it is placed in the front of the queu. If the
queue is not full, push the new entry to the front of the queue.
*/

#define COLLECT_TT_STATISTICS 0

template <typename T, uint8_t capacity>
class Mini_Queue
{

    static_assert((std::floor(std::log2(capacity)) == std::log2(capacity)),
                  "Mini_Queue capacity should be a power of two for efficient "
                  "modulo operations.");

  public:

    class Iterator
    {
      public:

        Iterator(const Mini_Queue* owner, uint8_t index, uint8_t remaining);
        T&        operator*() const;
        Iterator& operator++();
        bool      operator==(const Iterator& other) const;
        bool      operator!=(const Iterator& other) const;

      private:

        Mini_Queue* m_owner = nullptr;
        uint8_t     m_index = 0;

        // We need m_remaining to solve the issue of m_front == m_back when the
        // queue is full, if m_index was used to determine equality - a full
        // queue would not be iterated over.
        uint8_t m_remaining = 0;

        uint8_t wrap_index(uint8_t index) const;
    };

    Iterator begin() const;
    Iterator end() const;

    bool              is_empty() const;
    bool              is_full() const;
    uint8_t           size() const;
    constexpr uint8_t get_capacity() const;
    void              clear();
    T&                front();
    T&                back();

    void push(const T& value); // Pushes to the front only.
    void pop();                // Pops from the back only.
    bool conditional_eviction(const T& value, T* evicted_value);
    void swap_up(uint8_t index);
    void move_to_front(uint8_t index);
    void remove(uint8_t index);

  private:

    std::array<T, capacity> m_queue {};

    // m_front = index of the current front element
    // m_back  = index one past the current back element
    // m_size  = number of elements currently in the queue (needed for
    // empty/full conditions)
    uint8_t m_front = 0;
    uint8_t m_back  = 0;
    uint8_t m_size  = 0;

    uint8_t wrap_index(uint8_t index) const;
};

constexpr uint64_t PARTIAL_ZOBRIST_MASK = 0xFFFF;

constexpr uint8_t TT_PROTECTED_CLUSTER_SIZE    = 4;
constexpr uint8_t TT_PROBATIONARY_CLUSTER_SIZE = 4;

constexpr uint16_t TRANSPOSITION_TABLE_DEPTH_THRESHOLD = 2;

constexpr uint64_t DEFAULT_TRANSPOSITION_TABLE_SIZE = 128; // MiB

enum class Score_Bound_Type : uint8_t
{
    EXACT,
    LOWER_BOUND,
    UPPER_BOUND
};

struct Transposition_Table_Entry
{
    Chess_Move       best_move;       // 12 bytes
    Score            score;           // 4 bytes
    uint16_t         partial_zobrist; // 2 bytes
    uint16_t         depth;           // 2 bytes
    Score_Bound_Type score_bound;     // 1 bytes
};

static_assert(sizeof(Transposition_Table_Entry) == 24,
              "Transposition_Table_Entry should be 24 bytes in size.");

struct CACHE_ALIGN Transposition_Table_Cluster
{
    Mini_Queue<Transposition_Table_Entry, TT_PROTECTED_CLUSTER_SIZE>
        protected_entries;
    Mini_Queue<Transposition_Table_Entry, TT_PROBATIONARY_CLUSTER_SIZE>
        probationary_entries;
};

struct Transposition_Table_Statistics
{
    uint64_t nodes_written       = 0;
    uint64_t nodes_read          = 0;
    uint64_t protected_hits      = 0;
    uint64_t protected_misses    = 0;
    uint64_t probationary_hits   = 0;
    uint64_t probationary_misses = 0;

    void print() const
    {
        double protected_hit_rate = static_cast<double>(protected_hits)
                                  / static_cast<double>(nodes_read) * 100;
        double protected_miss_rate = static_cast<double>(protected_misses)
                                   / static_cast<double>(nodes_read) * 100;
        double probationary_hit_rate = static_cast<double>(probationary_hits)
                                     / static_cast<double>(nodes_read) * 100;
        double probationary_miss_rate = static_cast<double>(probationary_misses)
                                      / static_cast<double>(nodes_read) * 100;

        std::cout << "Transposition Table Statistics:\n"
                  << "Nodes Written: " << nodes_written << "\n"
                  << "Nodes Read: " << nodes_read << "\n"
                  << "Protected Hits: " << protected_hits << " ("
                  << protected_hit_rate << "%)\n"
                  << "Protected Misses: " << protected_misses << " ("
                  << protected_miss_rate << "%)\n"
                  << "Probationary Hits: " << probationary_hits << " ("
                  << probationary_hit_rate << "%)\n"
                  << "Probationary Misses: " << probationary_misses << " ("
                  << probationary_miss_rate << "%)" << std::endl;
    }
};

class Transposition_Table
{
  public:

    explicit Transposition_Table(
        const uint64_t size_in_mib = DEFAULT_TRANSPOSITION_TABLE_SIZE);

    void resize(const uint64_t size_in_mib);

    FORCE_INLINE void prefetch(const Zobrist_Hash& hash);

    bool read(const uint16_t             max_depth,
              const uint16_t             ply,
              const Zobrist_Hash&        hash,
              Transposition_Table_Entry& output);

    void write(const uint16_t             max_depth,
               const uint16_t             ply,
               const Zobrist_Hash&        hash,
               Transposition_Table_Entry& entry);

    void clear();

    static uint16_t get_partial_zobrist(const Zobrist_Hash& hash);

    const Transposition_Table_Statistics& get_statistics() const;
    void                                  clear_statistics();

  private:

    Transposition_Table_Cluster* m_table;
    uint64_t                     m_size;

#if COLLECT_TT_STATISTICS == 1
    Transposition_Table_Statistics m_statistics;
#endif

    uint64_t get_lemire_index(const Zobrist_Hash& hash) const;

    bool is_priority_entry(const uint16_t                   max_depth,
                           const Transposition_Table_Entry& entry) const;

    void promotion_to_protected_segment(const uint16_t max_depth,
                                        const uint64_t index,
                                        const Transposition_Table_Entry& entry);

    bool should_replace_matched_entry(
        const Transposition_Table_Entry& existing_entry,
        const Transposition_Table_Entry& new_entry);

    template <bool is_read>
    void make_mate_score_relative_to_node(Transposition_Table_Entry& entry,
                                          const uint16_t node_ply) const;
};

FORCE_INLINE void Transposition_Table::prefetch(const Zobrist_Hash& hash)
{
    constexpr uint64_t SIZE_OF_CLUSTER = sizeof(Transposition_Table_Cluster);
    constexpr bool     IS_CLUSTER_SIZE_MULTIPLE_OF_CACHE_LINE_SIZE =
        ((SIZE_OF_CLUSTER % CACHE_LINE_SIZE) == 0);
    constexpr uint64_t PREFETCH_STRIDE =
        (SIZE_OF_CLUSTER / CACHE_LINE_SIZE)
        + (IS_CLUSTER_SIZE_MULTIPLE_OF_CACHE_LINE_SIZE ? 0 : 1);

    uint64_t index   = get_lemire_index(hash);
    uint8_t* address = reinterpret_cast<uint8_t*>(&m_table[index]);

    for (uint8_t i = 0; i < PREFETCH_STRIDE; ++i)
    {
        // Note: we want to prefetch for a write because of how our cluster's
        // queues work on a read.
        __builtin_prefetch((address + (i * CACHE_LINE_SIZE)), 1, 3);
    }
}

// Note: This logic limits the capacity to being powers of two. If it is needed
// to support non-powers of two, the function can be modified to use modulus
// instead of bitwise AND.
template <typename T, uint8_t capacity>
uint8_t Mini_Queue<T, capacity>::wrap_index(uint8_t index) const
{
    return (index & (capacity - 1));
}

template <typename T, uint8_t capacity>
bool Mini_Queue<T, capacity>::is_empty() const
{
    return (m_size == 0);
}

template <typename T, uint8_t capacity>
bool Mini_Queue<T, capacity>::is_full() const
{
    return (m_size == capacity);
}

template <typename T, uint8_t capacity>
uint8_t Mini_Queue<T, capacity>::size() const
{
    return m_size;
}

template <typename T, uint8_t capacity>
constexpr uint8_t Mini_Queue<T, capacity>::get_capacity() const
{
    return capacity;
}

template <typename T, uint8_t capacity>
void Mini_Queue<T, capacity>::clear()
{
    m_front = 0;
    m_back  = 0;
    m_size  = 0;
}

template <typename T, uint8_t capacity>
T& Mini_Queue<T, capacity>::front()
{
    return m_queue[m_front];
}

template <typename T, uint8_t capacity>
T& Mini_Queue<T, capacity>::back()
{
    return m_queue[wrap_index(static_cast<uint8_t>(m_back - 1))];
}

template <typename T, uint8_t capacity>
void Mini_Queue<T, capacity>::push(const T& value)
{
    if (is_full()) { return; }

    m_front          = wrap_index(static_cast<uint8_t>(m_front - 1));
    m_queue[m_front] = value;

    if (m_size == 0) { m_back = wrap_index(static_cast<uint8_t>(m_front + 1)); }

    ++m_size;
}

template <typename T, uint8_t capacity>
void Mini_Queue<T, capacity>::pop()
{
    if (is_empty()) { return; }

    if (m_size == 1)
    {
        clear();
        return;
    }

    m_back = wrap_index(static_cast<uint8_t>(m_back - 1));
    --m_size;
}

template <typename T, uint8_t capacity>
bool Mini_Queue<T, capacity>::conditional_eviction(const T& value,
                                                   T*       evicted_value)
{
    if (!is_full())
    {
        push(value);
        return false; // No eviction occurred.
    }

    const uint8_t eviction_index = wrap_index(static_cast<uint8_t>(m_back - 1));

    if (evicted_value) { *evicted_value = std::move(m_queue[eviction_index]); }

    // Insert a new element in front.
    m_front          = wrap_index(static_cast<uint8_t>(m_front - 1));
    m_queue[m_front] = value;

    // Evict the back element by moving the back index back by one.
    m_back = eviction_index;

    return true; // Eviction occurred.
}

template <typename T, uint8_t capacity>
void Mini_Queue<T, capacity>::move_to_front(uint8_t index)
{
    if ((index >= m_size) || (index == 0)) { return; }

    const uint8_t wrapped_index =
        wrap_index(static_cast<uint8_t>(m_front + index));

    T saved = std::move(m_queue[wrapped_index]);

    for (uint8_t i = index; i != 0; --i)
    {
        const uint8_t dst = wrap_index(static_cast<uint8_t>(m_front + i));
        const uint8_t src = wrap_index(static_cast<uint8_t>(m_front + i - 1));

        m_queue[dst] = std::move(m_queue[src]);
    }

    m_queue[m_front] = std::move(saved);
}

// Given an index into the queue, swap the element at that index with the
// element in front of it in the queue.
template <typename T, uint8_t capacity>
void Mini_Queue<T, capacity>::swap_up(uint8_t index)
{
    if ((m_size <= 1) || (index == 0)) { return; }

    const uint8_t wrapped_current_index =
        wrap_index(static_cast<uint8_t>(m_front + index));
    const uint8_t wrapped_next_index =
        wrap_index(static_cast<uint8_t>(m_front + index - 1));

    std::swap(m_queue[wrapped_current_index], m_queue[wrapped_next_index]);
}

template <typename T, uint8_t capacity>
void Mini_Queue<T, capacity>::remove(uint8_t index)
{
    if (index >= m_size) { return; }

    if (m_size == 1)
    {
        clear();
        return;
    }

    // Pop from the front.
    if (index == 0)
    {
        m_front = wrap_index(static_cast<uint8_t>(m_front + 1));
        --m_size;
        return;
    }

    if (index == static_cast<uint8_t>(m_size - 1))
    {
        pop();
        return;
    }

    // Number of elements before the erased one.
    const uint8_t left_count = index;

    // Number of elements after the erased one.
    const uint8_t right_count = static_cast<uint8_t>(m_size - 1 - index);

    // Shift the elements on the left side of the index one step toward the
    // back.
    if (left_count < right_count)
    {
        for (uint8_t i = index; i != 0; --i)
        {
            const uint8_t dst = wrap_index(static_cast<uint8_t>(m_front + i));
            const uint8_t src =
                wrap_index(static_cast<uint8_t>(m_front + i - 1));

            m_queue[dst] = std::move(m_queue[src]);
        }

        m_front = wrap_index(static_cast<uint8_t>(m_front + 1));

        // Shift the elements on the right side of index one step toward the
        // front.
    }
    else
    {
        for (uint8_t i = index; i != static_cast<uint8_t>(m_size - 1); ++i)
        {
            const uint8_t dst = wrap_index(static_cast<uint8_t>(m_front + i));
            const uint8_t src =
                wrap_index(static_cast<uint8_t>(m_front + i + 1));

            m_queue[dst] = std::move(m_queue[src]);
        }

        m_back = wrap_index(static_cast<uint8_t>(m_back - 1));
    }

    --m_size;
}

template <typename T, uint8_t capacity>
Mini_Queue<T, capacity>::Iterator::Iterator(const Mini_Queue* owner,
                                            uint8_t           index,
                                            uint8_t           remaining) :
    m_owner(const_cast<Mini_Queue*>(owner)),
    m_index(index),
    m_remaining(remaining)
{
}

template <typename T, uint8_t capacity>
T& Mini_Queue<T, capacity>::Iterator::operator*() const
{
    return m_owner->m_queue[m_index];
}

template <typename T, uint8_t capacity>
uint8_t Mini_Queue<T, capacity>::Iterator::wrap_index(uint8_t index) const
{
    return (index & (m_owner->get_capacity() - 1));
}

template <typename T, uint8_t capacity>
typename Mini_Queue<T, capacity>::Iterator&
Mini_Queue<T, capacity>::Iterator::operator++()
{
    if (m_remaining != 0)
    {
        m_index = wrap_index(static_cast<uint8_t>(m_index + 1));
        --m_remaining;
    }
    return *this;
}

template <typename T, uint8_t capacity>
bool Mini_Queue<T, capacity>::Iterator::operator==(const Iterator& other) const
{
    return ((m_owner == other.m_owner) && (m_remaining == other.m_remaining));
}

template <typename T, uint8_t capacity>
bool Mini_Queue<T, capacity>::Iterator::operator!=(const Iterator& other) const
{
    return !(*this == other);
}

template <typename T, uint8_t capacity>
Mini_Queue<T, capacity>::Iterator Mini_Queue<T, capacity>::begin() const
{
    return Mini_Queue::Iterator(this, m_front, m_size);
}

template <typename T, uint8_t capacity>
Mini_Queue<T, capacity>::Iterator Mini_Queue<T, capacity>::end() const
{
    return Mini_Queue::Iterator(this, m_back, 0);
}
