#include <vector>

#include "gtest/gtest.h"
#include "transposition_table.hpp"

template <typename T, uint8_t Capacity>
std::vector<T> mini_queue_to_vector(const Mini_Queue<T, Capacity>& q) {
  std::vector<T> out;
  for (const T& value : q) {
    out.push_back(value);
  }
  return out;
}

TEST(transposition_table_tests, mini_queue_test) {
  Mini_Queue<int, 4> q;

  // Initial state
  EXPECT_TRUE(q.is_empty());
  EXPECT_FALSE(q.is_full());
  EXPECT_EQ(q.size(), 0);

  // push(), front(), back()
  q.push(1);  // [1]
  q.push(2);  // [2,1]
  q.push(3);  // [3,2,1]

  EXPECT_FALSE(q.is_empty());
  EXPECT_EQ(q.size(), 3);
  EXPECT_EQ(q.front(), 3);
  EXPECT_EQ(q.back(), 1);

  // Iterator traversal
  EXPECT_EQ(mini_queue_to_vector(q), std::vector<int>({3, 2, 1}));

  // Swap up test.
  q.swap_up(2);  // [3,1,2]
  EXPECT_EQ(mini_queue_to_vector(q), std::vector<int>({3, 1, 2}));

  // move_to_front()
  q.move_to_front(2);  // [2,3,1]
  EXPECT_EQ(mini_queue_to_vector(q), std::vector<int>({2, 3, 1}));

  // remove()
  q.remove(1);  // [2,1]
  EXPECT_EQ(mini_queue_to_vector(q), std::vector<int>({2, 1}));

  // pop()
  q.pop();  // [2]
  EXPECT_EQ(q.size(), 1);
  EXPECT_EQ(q.front(), 2);
  EXPECT_EQ(q.back(), 2);

  // conditional_eviction() when not full
  int evicted = -1;
  EXPECT_FALSE(q.conditional_eviction(4, &evicted));  // [4,2]
  EXPECT_EQ(mini_queue_to_vector(q), std::vector<int>({4, 2}));

  // Fill queue, then conditional_eviction() when full
  q.push(5);  // [5,4,2]
  q.push(6);  // [6,5,4,2]
  EXPECT_TRUE(q.is_full());
  EXPECT_TRUE(q.conditional_eviction(7, &evicted));  // [7,6,5,4]
  EXPECT_EQ(evicted, 2);
  EXPECT_EQ(q.size(), 4);
  EXPECT_EQ(q.front(), 7);
  EXPECT_EQ(q.back(), 4);
  EXPECT_EQ(mini_queue_to_vector(q), std::vector<int>({7, 6, 5, 4}));

  // clear()
  q.clear();
  EXPECT_TRUE(q.is_empty());
  EXPECT_EQ(q.size(), 0);
}
