#include <algorithm>
#include <vector>

template <typename T>
bool are_vectors_equal(std::vector<T> vec1, std::vector<T> vec2) {
  if (vec1.size() != vec2.size()) {
    return false;
  }

  for (size_t i = 0; i < vec1.size(); i++) {
    if (std::find(vec2.begin(), vec2.end(), vec1[i]) == vec2.end()) {
      return false;
    }
  }

  return true;
}
