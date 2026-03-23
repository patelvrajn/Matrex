#pragma once

#include <stdint.h>

#include <array>
#include <cstddef>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>

constexpr std::string_view ENGINE_NAME = "Matrex";
constexpr std::string_view ENGINE_VERSION = "0.0.1";

constexpr uint8_t NUM_OF_PLAYERS = 2;

constexpr uint8_t NUM_OF_RANKS_ON_CHESS_BOARD = 8;
constexpr uint8_t NUM_OF_FILES_ON_CHESS_BOARD = 8;
constexpr uint8_t NUM_OF_SQUARES_ON_CHESS_BOARD =
    NUM_OF_RANKS_ON_CHESS_BOARD * NUM_OF_FILES_ON_CHESS_BOARD;

constexpr uint8_t NUM_OF_PIECES_PER_PLAYER = 16;
constexpr uint8_t NUM_OF_UNIQUE_PIECES_PER_PLAYER = 6;

enum PIECE_COLOR { WHITE, BLACK, NO_COLOR };

enum PIECES { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NO_PIECE };

constexpr std::string PIECE_STRINGS[] = {"PAWN",  "KNIGHT", "BISHOP",  "ROOK",
                                         "QUEEN", "KING",   "NO_PIECE"};

constexpr std::string
    UNICODE_PIECES[NUM_OF_PLAYERS * NUM_OF_UNIQUE_PIECES_PER_PLAYER] = {
        "♙", "♘", "♗", "♖", "♕", "♔", "♟︎", "♞", "♝", "♜", "♛", "♚"};

constexpr std::string_view START_POSITION_FEN =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

// =============================================================================
// Multi_Array Implementation (Short-hand version of std::array for multiple
// dimensions)
// =============================================================================
// Recursive multi_array definition
template <typename T, std::size_t thisSize, std::size_t... otherSizes>
class multi_array
    : private std::array<multi_array<T, otherSizes...>, thisSize> {
  using base_array = std::array<multi_array<T, otherSizes...>, thisSize>;

 public:
  using base_array::operator[];

  // Constructor from initializer list of multi_arrays
  constexpr multi_array(
      std::initializer_list<multi_array<T, otherSizes...>> init) {
    std::size_t i = 0;
    for (auto& v : init)  // Only sets up to the number of elements in the
                          // initializer list.
    {
      if (i < thisSize) {
        (*this)[i++] = v;
      } else
        break;  // ignore extra elements
    }
  }

  // Default constructor
  constexpr multi_array() = default;
};

// Base case: single-dimension multi_array
template <typename T, std::size_t thisSize>
class multi_array<T, thisSize> : private std::array<T, thisSize> {
  using base_array = std::array<T, thisSize>;

 public:
  using base_array::operator[];

  // Constructor from initializer list
  constexpr multi_array(std::initializer_list<T> init) {
    std::size_t i = 0;
    for (auto& v : init)  // Only sets up to the number of elements in the
                          // initializer list.
    {
      if (i < thisSize) {
        (*this)[i++] = v;
      } else
        break;  // ignore extra elements
    }
  }

  // Default constructor
  constexpr multi_array() = default;
};

// =============================================================================
// Compile-time element counting
// =============================================================================
// Scalar → 1 element
template <typename T>
struct element_count {
  static constexpr std::size_t value = 1;
};

// multi_array → product of all dimensions
template <typename T, std::size_t N, std::size_t... Rest>
struct element_count<multi_array<T, N, Rest...>> {
  static constexpr std::size_t value =
      N * element_count<multi_array<T, Rest...>>::value;
};

// Base case of recursion
template <typename T, std::size_t N>
struct element_count<multi_array<T, N>> {
  static constexpr std::size_t value = N;
};

// =============================================================================
// Recursive reference collection (index-based, private-inheritance-safe)
// =============================================================================
// Base case: scalar element
template <typename T, typename Array>
void collect_refs(T& value, Array& out, std::size_t& index) {
  out[index++] = std::ref(value);
}

// Recursive case: multi_array
template <typename T, std::size_t N, typename Array>
void collect_refs(multi_array<T, N>& arr, Array& out, std::size_t& index) {
  for (std::size_t i = 0; i < N; ++i) {
    collect_refs(arr[i], out, index);
  }
}

// Recursive case: multi_array with more dimensions
template <typename T, std::size_t N, std::size_t... Rest, typename Array>
void collect_refs(multi_array<T, N, Rest...>& arr, Array& out,
                  std::size_t& index) {
  for (std::size_t i = 0; i < N; ++i) {
    collect_refs(arr[i], out, index);
  }
}

// =============================================================================
// Reference_Array
// =============================================================================
template <typename... Args>
constexpr auto calculate_reference_array_size() {
  // Compute total number of elements at compile time
  constexpr std::size_t total =
      (element_count<std::remove_reference_t<Args>>::value + ...);

  return total;
}

template <typename T, typename... Args>
auto make_reference_array(Args&... args) {
  // Compute total number of elements at compile time
  constexpr std::size_t total = calculate_reference_array_size<Args...>();

  // Fixed-size array of references
  std::array<std::optional<std::reference_wrapper<T>>, total> result;

  // Current insertion index
  std::size_t index = 0;

  // Helper to append one argument
  auto append = [&](auto& x) { collect_refs(x, result, index); };

  // Expand parameter pack
  (append(args), ...);

  return result;
}

template <typename T, typename... Args>
class Reference_Array {
 public:
  static constexpr std::size_t size = calculate_reference_array_size<Args...>();

 private:
  std::array<std::optional<std::reference_wrapper<T>>, size> m_refs;
  std::unordered_map<const void*, std::size_t> m_ref_to_index_map;

 public:
  explicit Reference_Array(Args&... args)
      : m_refs(make_reference_array<T>(args...)) {
    m_ref_to_index_map.reserve(size);

    for (std::size_t i = 0; i < size; i++) {
      if (!m_refs[i].has_value()) {
        continue;
      }

      const void* key = static_cast<const void*>(&m_refs[i].value().get());
      auto [it, inserted] = m_ref_to_index_map.emplace(key, i);
      if (!inserted) {
        throw std::logic_error(
            "Reference_Array constructor: Only unique references are allowed, "
            "but a duplicate reference was found");
      }
    }
  }

  auto& get_array() { return m_refs; }

  const auto& get_array() const { return m_refs; }

  template <typename U>  // Using U instead of T to allow get_index_of to accept
                         // references of types derived from T like const T&
  std::size_t get_index_of(const U& ref) const {
    const void* key = static_cast<const void*>(&ref);

    auto it = m_ref_to_index_map.find(key);
    if (it == m_ref_to_index_map.end()) {
      throw std::out_of_range(
          "Reference_Array::get_index_of: reference not found");
    }

    return it->second;
  }
};
