#pragma once

#include <array>
#include <cstddef>
#include <cstdlib>
#include <format>
#include <functional>
#include <iostream>
#include <optional>
#include <stacktrace>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <cmath>

#include "square.hpp"

// =============================================================================
// Engine Name and Version Strings
// =============================================================================
constexpr std::string_view ENGINE_NAME    = "Matrex";
constexpr std::string_view ENGINE_VERSION = "0.0.1";

// =============================================================================
// Pre-processor Definitions
// =============================================================================
// Recursively inlines all code such that there is no function call costs.
// However, should be used at caution because the trade-off is that there may be
// many more variables cluttering registers, the stack, etc.
#define FORCE_INLINE    inline __attribute__((always_inline, flatten))
#define FORCE_NO_INLINE [[gnu::noinline]]

// Used to avoid compilation warnings in cases where a variable may be optimized
// such that the code doesn't use it. Example: variable used only in assertions
// will only be used if it is a debug build of Matrex.
#define MAYBE_UNUSED [[maybe_unused]]

// Pads structures laid out in memory so that each structure represents a single
// cache line. This issue is mainly concerning with multi-threaded code - if a
// single core writes to structure X and another core reads from structure Y if
// they reside on the same cache-line the core reading Y is forced to fetch the
// memory address again to update the cache even though it was only reading Y
// and not X.
constexpr uint64_t CACHE_LINE_SIZE =
    std::hardware_destructive_interference_size;
#define CACHE_ALIGN alignas(CACHE_LINE_SIZE)

#define SIZE_OF_IN_BITS(obj) (sizeof((obj)) << 3)

// =============================================================================
// Essential Chess-Related Entities
// =============================================================================
constexpr uint8_t NUM_OF_PLAYERS = 2;

constexpr uint8_t NUM_OF_RANKS_ON_CHESS_BOARD = 8;
constexpr uint8_t NUM_OF_FILES_ON_CHESS_BOARD = 8;
constexpr uint8_t NUM_OF_SQUARES_ON_CHESS_BOARD =
    NUM_OF_RANKS_ON_CHESS_BOARD * NUM_OF_FILES_ON_CHESS_BOARD;

constexpr uint8_t NUM_OF_PIECES_PER_PLAYER        = 16;
constexpr uint8_t NUM_OF_UNIQUE_PIECES_PER_PLAYER = 6;

enum PIECE_COLOR
{
    WHITE,
    BLACK,
    NO_COLOR
};

constexpr PIECE_COLOR& operator++(PIECE_COLOR& c)
{
    const int8_t value = static_cast<int8_t>(c) + 1;
    c                  = static_cast<PIECE_COLOR>(value);
    return c;
}

constexpr PIECE_COLOR operator~(const PIECE_COLOR c)
{
    if (c == PIECE_COLOR::WHITE) { return PIECE_COLOR::BLACK; }
    else if (c == PIECE_COLOR::BLACK) { return PIECE_COLOR::WHITE; }

    return PIECE_COLOR::NO_COLOR;
}

inline std::ostream& operator<<(std::ostream& os, const PIECE_COLOR color)
{
    switch (color)
    {
        case PIECE_COLOR::WHITE: return os << "White";
        case PIECE_COLOR::BLACK: return os << "Black";
        default                : return os << "NO_COLOR";
    }
}

enum PIECES
{
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    NO_PIECE
};

inline std::ostream& operator<<(std::ostream& os, const PIECES piece)
{
    switch (piece)
    {
        case PIECES::PAWN  : return os << "Pawn";
        case PIECES::KNIGHT: return os << "Knight";
        case PIECES::BISHOP: return os << "Bishop";
        case PIECES::ROOK  : return os << "Rook";
        case PIECES::QUEEN : return os << "Queen";
        case PIECES::KING  : return os << "King";
        default            : return os << "NO_PIECE";
    }
}

constexpr std::string
    UNICODE_PIECES[NUM_OF_PLAYERS * NUM_OF_UNIQUE_PIECES_PER_PLAYER] =
        {"♙", "♘", "♗", "♖", "♕", "♔", "♟︎", "♞", "♝", "♜", "♛", "♚"};

struct Placed_Piece
{
    PIECE_COLOR color  = PIECE_COLOR::NO_COLOR;
    PIECES      piece  = PIECES::NO_PIECE;
    Square      square = NO_SQUARE_OBJ;

    bool operator==(const Placed_Piece& other) const
    {
        return ((other.color == color) && (other.piece == piece)
                && (other.square == square));
    }

    friend std::ostream& operator<<(std::ostream& os, const Placed_Piece& pp)
    {
        os << pp.color << " " << pp.piece << " on " << pp.square;

        return os;
    }
};

constexpr std::string_view START_POSITION_FEN =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

inline uint16_t moves_to_ply(const PIECE_COLOR c, const uint16_t num_of_moves)
{
    if (c == PIECE_COLOR::WHITE) { return (NUM_OF_PLAYERS * num_of_moves); }
    else if (c == PIECE_COLOR::BLACK)
    {
        return ((NUM_OF_PLAYERS * num_of_moves) + 1);
    }

    return 0;
}

constexpr uint8_t NUM_OF_CASTLING_TYPES = 2;

enum CASTLING_TYPE
{
    KINGSIDE,
    QUEENSIDE
};

constexpr uint8_t NUM_OF_CASTLING_RIGHTS_FLAGS = 4;

enum CASTLING_RIGHTS_FLAGS
{
    W_KINGSIDE  = 1,
    W_QUEENSIDE = 2,
    B_KINGSIDE  = 4,
    B_QUEENSIDE = 8
};

constexpr uint8_t NUM_OF_CASTLING_RIGHTS_COMBINATIONS = static_cast<uint8_t>(
    std::pow(static_cast<double>(NUM_OF_CASTLING_TYPES),
             static_cast<double>(NUM_OF_CASTLING_RIGHTS_FLAGS)));

// An enumeration describing directions (diagonal or orthogonal) in a chess
// board. The directions are assigned values based on the number of bits in a
// bitboard one needs to incrementally change the bit index (i.e. square index)
// by to go in that direction on the board.
enum DIRECTION : int8_t
{
    NORTHWEST    = -9,
    NORTH        = -8,
    NORTHEAST    = -7,
    WEST         = -1,
    NO_DIRECTION = 0,
    EAST         = 1,
    SOUTHWEST    = 7,
    SOUTH        = 8,
    SOUTHEAST    = 9
};

// Currently, Matrex has it's own runtime assertion function in order to be more
// verbose when unexpected behavior occurs during runtime. The verbose-ness is
// mainly from the full stack trace being displayed which is useful when trying
// to debug.
#ifdef NDEBUG
    #define MATREX_ASSERT(condition, message, ...) ((void) 0)
#else
    #define MATREX_ASSERT(condition, message, ...)                             \
        if (!(condition))                                                      \
        {                                                                      \
            std::cerr << "Assertion in file " << __FILE__ << " on line "       \
                      << __LINE__ << " failed!" << std::endl;                  \
            std::cerr << "Condition: " << #condition << std::endl;             \
            std::cerr << "Message: "                                           \
                      << std::format(message __VA_OPT__(, ) __VA_ARGS__)       \
                      << std::endl;                                            \
            std::cerr << "STACK TRACE: " << std::endl;                         \
            std::cerr << std::stacktrace::current() << std::endl;              \
            std::abort();                                                      \
        }
#endif

// =============================================================================
// Multi_Array Implementation
// =============================================================================
// Recursive definition
template <typename T, std::size_t this_size, std::size_t... other_sizes>
class Multi_Array
{
    using element_type = Multi_Array<T, other_sizes...>;

  public:

    std::array<element_type, this_size> data {};

    constexpr static std::size_t size = this_size;

    constexpr Multi_Array() = default;

    constexpr Multi_Array(const std::initializer_list<element_type> init)
    {
        if (init.size() == 0)
        {
            for (std::size_t i = 0; i < this_size; ++i)
            {
                // Default construct all members.
                (*this)[i] = element_type();
            }
        }
        else if (init.size() == 1)
        {
            for (std::size_t i = 0; i < this_size; ++i)
            {
                // Fill the array with the only given value;
                (*this)[i] = (*init.begin());
            }
        }
        else
        {
            std::size_t i = 0;
            for (const auto& v : init) // Only sets up to the number of elements
                                       // in the initializer list.
            {
                if (i < this_size) { (*this)[i++] = v; }
                else
                {
                    break; // ignore extra elements
                }
            }
        }
    }

    constexpr element_type& operator[](const std::size_t i) { return data[i]; }

    constexpr const element_type& operator[](const std::size_t i) const
    {
        return data[i];
    }

    constexpr bool operator==(const Multi_Array& other) const
    {
        return data == other.data;
    }

    constexpr auto begin() { return data.begin(); }

    constexpr auto end() { return data.end(); }

    constexpr auto begin() const { return data.begin(); }

    constexpr auto end() const { return data.end(); }

    void fill(const T fill_value)
    {
        for (auto& element : (*this)) { element.fill(fill_value); }
    }
};

// Base case: single-dimension
template <typename T, std::size_t this_size>
class Multi_Array<T, this_size>
{
    using element_type = T;

  public:

    std::array<element_type, this_size> data {};

    constexpr static std::size_t size = this_size;

    constexpr Multi_Array() = default;

    constexpr Multi_Array(const std::initializer_list<T> init)
    {
        if (init.size() == 0)
        {
            for (std::size_t i = 0; i < this_size; ++i)
            {
                // Default construct all members.
                (*this)[i] = element_type();
            }
        }
        else if (init.size() == 1)
        {
            for (std::size_t i = 0; i < this_size; ++i)
            {
                // Fill the array with the only given value;
                (*this)[i] = (*init.begin());
            }
        }
        else
        {
            std::size_t i = 0;
            for (const auto& v : init) // Only sets up to the number of elements
                                       // in the initializer list.
            {
                if (i < this_size) { (*this)[i++] = v; }
                else
                {
                    break; // ignore extra elements
                }
            }
        }
    }

    constexpr element_type& operator[](const std::size_t i) { return data[i]; }

    constexpr const element_type& operator[](const std::size_t i) const
    {
        return data[i];
    }

    constexpr bool operator==(const Multi_Array& other) const
    {
        return data == other.data;
    }

    constexpr auto begin() { return data.begin(); }

    constexpr auto end() { return data.end(); }

    constexpr auto begin() const { return data.begin(); }

    constexpr auto end() const { return data.end(); }

    void fill(const T fill_value)
    {
        for (auto& element : (*this)) { element = fill_value; }
    }
};

// =============================================================================
// Constant Expression For-Loop
//
// Description: A recursive function to loop through code in the function F
// N = (floor((End - Start) / Step) + 1) times where all code executes at
// compile-time. The primary purpose is provide a constant expression index to
// use in the for loop.
// =============================================================================
template <auto Start, auto End, auto Step, class F>
constexpr void constexpr_for(F&& function)
{
    constexpr bool is_positive_iteration_going = ((Step > 0) && (Start < End));
    constexpr bool is_negative_iteration_going = ((Step < 0) && (Start > End));

    if constexpr (is_positive_iteration_going || is_negative_iteration_going)
    {
        function(std::integral_constant<decltype(Start), Start> {});

        constexpr_for<(Start + Step), End, Step>(std::forward<F>(function));
    }
}

// =============================================================================
// Index Sequence Unpacker
//
// Description: Abstraction of index sequence - unpacks the sequence and calls
// f's () operator which means that f must be a functor, function, lambda, etc.
// =============================================================================
template <std::size_t N, class F>
constexpr decltype(auto) index_sequence_unpacker(F&& f)
{
    return []<std::size_t... Is>(std::index_sequence<Is...>,
                                 F&& f) -> decltype(auto)
    {
        return static_cast<F&&>(f).template operator()<Is...>();
    }(std::make_index_sequence<N> {}, static_cast<F&&>(f));
}

// =============================================================================
// Template Parameter Extraction
//
// Generalized structs to extract the template parameters of an In typed object
// and output an Out typed object with the same template parameters.
// =============================================================================
template <class In, template <class...> class Out>
struct Extract_Template_Parameters;

template <template <class...> class In,
          template <class...> class Out,
          class... Ts>
struct Extract_Template_Parameters<In<Ts...>, Out>
{
    using Type = Out<Ts...>;
};

// =============================================================================
// Optional Reference
//
// A container that optionally refers to an existing object without owning it.
// =============================================================================
template <typename T>
class Optional_Reference
{
  public:

    Optional_Reference() = default;

    Optional_Reference(T& ref) : m_optional_reference(std::ref(ref)) {}

    Optional_Reference(std::reference_wrapper<T> ref) :
        m_optional_reference(ref)
    {
    }

    // Copy semantics.
    Optional_Reference(const Optional_Reference&)            = default;
    Optional_Reference& operator=(const Optional_Reference&) = default;

    // Move semantics.
    Optional_Reference(Optional_Reference&&)            = default;
    Optional_Reference& operator=(Optional_Reference&&) = default;

    bool has_ref() const { return m_optional_reference.has_value(); }

    explicit operator bool() const { return has_ref(); }

    void unbound_ref() { m_optional_reference.reset(); }

    T& get_ref()
    {
        MATREX_ASSERT(has_ref(),
                      "Optional Reference Assertion FAILED; Tried to access a "
                      "null reference.");

        return m_optional_reference.value().get();
    }

    const T& get_ref() const
    {
        MATREX_ASSERT(has_ref(),
                      "Optional Reference Assertion FAILED; Tried to access a "
                      "null reference.");

        return m_optional_reference.value().get();
    }

  private:

    std::optional<std::reference_wrapper<T>> m_optional_reference;
};

// =============================================================================
// Parameter Pack Container
//
// Description: A class used for treating parameter packs as containers. Useful
// for indexing and/or slicing the parameter pack. All operations unless stated
// otherwise can be done during compile-time.
// =============================================================================
template <typename... Pack>
class Parameter_Pack_Container
{
  private:

    std::tuple<Pack...> m_p;

    template <std::size_t offset, std::size_t... Is>
    constexpr auto offset_index_sequence(const std::index_sequence<Is...>) const
    {
        return std::index_sequence<(Is + offset)...> {};
    }

    template <typename Tuple, std::size_t... Is>
    constexpr auto extract_parameter_set(const Tuple& t,
                                         const std::index_sequence<Is...>) const
    {
        return std::make_tuple(std::get<Is>(t)...);
    }

    template <std::size_t start, std::size_t end, typename Tuple>
    constexpr auto slice_tuple(const Tuple& t) const
    {
        static_assert((end < (std::tuple_size_v<Tuple>) ),
                      "End of slice for tuple is greater than it's size.");

        constexpr std::size_t length = end - start + 1;
        auto                  slice_sequence =
            offset_index_sequence<start>(std::make_index_sequence<length> {});
        return extract_parameter_set(t, slice_sequence);
    }

    template <std::size_t copy_length>
    constexpr void copy(const Parameter_Pack_Container& other)
    {
        return index_sequence_unpacker<copy_length>(
            [&]<std::size_t... index>()
            { ((std::get<index>(m_p) = std::get<index>(other.m_p)), ...); });
    }

    template <std::size_t move_length>
    constexpr void move(Parameter_Pack_Container&& other)
    {
        return index_sequence_unpacker<move_length>(
            [&]<std::size_t... index>()
            {
                ((std::get<index>(m_p) = std::move(std::get<index>(other.m_p))),
                 ...);
            });
    }

    // Produces a compile-time array of getters function to retrieve an element
    // at an index in the internal tuple during runtime. This avoids a templated
    // recursive call.
    static consteval auto make_getters()
    {
        return index_sequence_unpacker<size>(
            []<std::size_t... Is>()
            {
                using Tuple = decltype(m_p);
                using Getter_Function =
                    Parameter_Pack_Variant (*)(const Tuple&);

                return Multi_Array<Getter_Function, sizeof...(Is)> {
                    +[](const Tuple& p) -> Parameter_Pack_Variant
                    {
                        return Parameter_Pack_Variant(
                            std::in_place_index<Is + 1>,
                            std::cref(std::get<Is>(p)));
                    }...};
            });
    }

  public:

    // Note that the variant uses references to the existing object in the tuple
    // to support large data structures.
    using Parameter_Pack_Variant =
        std::variant<std::monostate, std::reference_wrapper<const Pack>...>;

    constexpr Parameter_Pack_Container() = default;

    // Takes a parameter pack and converts into a tuple to be assigned to the
    // internal tuple.
    template <class... Args>
    requires ((sizeof...(Args) == sizeof...(Pack)) && (sizeof...(Pack) > 0))
    constexpr Parameter_Pack_Container(Args&&... args) :
        m_p(std::forward<Args>(args)...)
    {
    }

    static constexpr std::size_t size = sizeof...(Pack);

    // Returns a std::variant at an index of the internal tuple.
    constexpr Parameter_Pack_Variant get(const std::size_t index) const
    {
        static constexpr auto getters = make_getters();

        MATREX_ASSERT((index < size),
                      "Parameter Pack Container Assertion FAILED: Get index "
                      "({}) is out of bounds with size of {}.",
                      index,
                      size);

        return getters[index](m_p);
    }

    constexpr Parameter_Pack_Variant operator[](const std::size_t index)
    {
        return this->get(index);
    }

    constexpr Parameter_Pack_Variant operator[](const std::size_t index) const
    {
        return this->get(index);
    }

    constexpr auto to_tuple() { return m_p; }

    // Grabs a non-contigious subset of the internal tuple.
    template <std::size_t... Is>
    constexpr auto get_parameter_set(const std::index_sequence<Is...> sequence)
    {
        extract_parameter_set(m_p, sequence);
    }

    // Grabs a contigious subset of the internal tuple.
    template <std::size_t start, std::size_t end>
    constexpr auto slice() const
    {
        return slice_tuple<start, end>(m_p);
    }

    // Takes the internal tuple and expands it out as parameters to any given
    // function (even templated functions).
    template <class Function>
    constexpr auto apply(Function&& function) const
    {
        std::apply([&](auto&&... args) { function(args...); }, m_p);
    }

    // Copies a Parameter_Pack_Container to this from a Parameter_Pack_Container
    // that has a size less than or equal to this.
    constexpr Parameter_Pack_Container(const Parameter_Pack_Container& other)
    {
        static_assert(size >= other.size,
                      "When copying parameter pack containers - the "
                      "destination must be of greater size or equal size.");

        copy<other.size>(other);
    }

    constexpr Parameter_Pack_Container(Parameter_Pack_Container&& other)
    {
        static_assert(size >= other.size,
                      "When moving parameter pack containers - the "
                      "destination must be of greater size or equal size.");

        move<other.size>(std::move(other));
    }

    // Creates an instance of Parameter_Pack_Container from any given tuple.
    template <class Tuple>
    static constexpr auto make(Tuple&& t)
    {
        using Output_Type = typename Extract_Template_Parameters<
            decltype(t),
            Parameter_Pack_Container>::Type;

        return std::make_from_tuple<Output_Type>(t);
    }
};

// =============================================================================
// Reference Array Implementation
// =============================================================================
template <typename T>
struct element_count
{
    static constexpr std::size_t value = 1;
};

// Product of all the sizes of the dimensions of the Multi Array.
template <typename T, std::size_t N, std::size_t... Rest>
struct element_count<Multi_Array<T, N, Rest...>>
{
    static constexpr std::size_t value =
        N * element_count<Multi_Array<T, Rest...>>::value;
};

// Base case of the recursion occuring when multiplying the sizes of the
// dimensions of the Multi Array.
template <typename T, std::size_t N>
struct element_count<Multi_Array<T, N>>
{
    static constexpr std::size_t value = N;
};

template <typename T, typename Array>
void collect_refs(T& value, Array& out, std::size_t& index)
{
    out[index++] = Optional_Reference<T>(value);
}

// Base recursive case: Multi_Array with 1 dimension.
template <typename T, std::size_t N, typename Array>
void collect_refs(Multi_Array<T, N>& arr, Array& out, std::size_t& index)
{
    for (std::size_t i = 0; i < N; ++i) { collect_refs(arr[i], out, index); }
}

// Recursive case: Multi_Array with more than 1 dimension.
template <typename T, std::size_t N, std::size_t... Rest, typename Array>
void collect_refs(Multi_Array<T, N, Rest...>& arr,
                  Array&                      out,
                  std::size_t&                index)
{
    for (std::size_t i = 0; i < N; ++i) { collect_refs(arr[i], out, index); }
}

template <typename... Args>
constexpr auto calculate_reference_array_size()
{
    // Compute total number of elements at compile time
    constexpr std::size_t total =
        (element_count<std::remove_reference_t<Args>>::value + ...);

    return total;
}

template <typename T, typename... Args>
auto make_reference_array(Args&... args)
{
    // Compute total number of elements at compile time
    constexpr std::size_t total = calculate_reference_array_size<Args...>();

    // Fixed-size array of references
    Multi_Array<Optional_Reference<T>, total> result;

    // Current insertion index
    std::size_t index = 0;

    // Helper to append one argument
    auto append = [&](auto& x) { collect_refs(x, result, index); };

    // Expand parameter pack
    (append(args), ...);

    return result;
}

template <typename T, typename... Args>
class Reference_Array
{
  public:

    static constexpr std::size_t size =
        calculate_reference_array_size<Args...>();

  private:

    Multi_Array<Optional_Reference<T>, size> m_refs;

    std::unordered_map<const void*, std::size_t> m_ref_to_index_map;

  public:

    explicit Reference_Array(Args&... args) :
        m_refs(make_reference_array<T>(args...))
    {
        m_ref_to_index_map.reserve(size);

        for (std::size_t i = 0; i < size; ++i)
        {
            if (!m_refs[i].has_ref()) { continue; }

            // The key to the hash table is a void pointer to the memory that is
            // being referenced.
            const void* key = static_cast<const void*>(&m_refs[i].get_ref());

            // Insert the key along with its value (the index at which the
            // reference exists in the reference array).
            auto [it, inserted] = m_ref_to_index_map.emplace(key, i);

            if (!inserted)
            {
                throw std::logic_error(
                    "Reference_Array ERROR: Only unique references are "
                    "allowed, "
                    "but a duplicate reference was found");
            }
        }
    }

    T& operator[](const std::size_t index)
    {
        MATREX_ASSERT(index < size,
                      "Reference Array Assertion FAILURE: operator[] "
                      "Indexed outside of size. Index: {}, Size: {}",
                      index,
                      size);

        return m_refs[index].get_ref();
    }

    const T& operator[](const std::size_t index) const
    {
        MATREX_ASSERT(index < size,
                      "Reference Array Assertion FAILURE: operator[] "
                      "Indexed outside of size. Index: {}, Size: {}",
                      index,
                      size);

        return m_refs[index].get_ref();
    }

    template <typename U> // Using U instead of T to allow get_index_of to
                          // accept references of types derived from T like
                          // const T&
    std::size_t get_index_of(const U& ref) const
    {
        const void* key = static_cast<const void*>(&ref);

        auto it = m_ref_to_index_map.find(key);
        if (it == m_ref_to_index_map.end())
        {
            throw std::out_of_range(
                "Reference_Array ERROR: Reference not found!");
        }

        return it->second;
    }
};

// =============================================================================
// Compile-time Jagged Array
// =============================================================================
template <typename>
struct empty_type
{
};

template <class T, std::size_t N>
struct tuple_of_empty_types
{
    using type = decltype(index_sequence_unpacker<N>(
        []<std::size_t... Is>()
        { return std::tuple<decltype((void) Is, empty_type<T> {})...> {}; }));
};

template <typename T, typename... Pack>
class Compile_Time_Jagged_Array
{
  public:

    static constexpr std::size_t size = sizeof...(Pack);

  private:

    template <class Tuple>
    struct Tuple_to_Jagged_Array;

    template <class... Ts>
    struct Tuple_to_Jagged_Array<std::tuple<Ts...>>
    {
        using Type = Compile_Time_Jagged_Array<T, Ts...>;
    };

    template <class... Ts>
    constexpr auto make_jagged_array_from_tuple(const std::tuple<Ts...> t)
    {
        using Output_Type = typename Tuple_to_Jagged_Array<decltype(t)>::Type;

        return std::make_from_tuple<Output_Type>(std::move(t));
    }

    Parameter_Pack_Container<Pack...> m_parameter_pack;

  public:

    constexpr Compile_Time_Jagged_Array() = default;

    template <class... Args>
    requires ((sizeof...(Args) == sizeof...(Pack)) && (sizeof...(Pack) > 0))
    constexpr Compile_Time_Jagged_Array(Args&&... args) :
        m_parameter_pack(std::forward<Args>(args)...)
    {
    }

    // IMPORTANT: We cannot move or copy Compile_Time_Jagged_Array during
    // compile-time because parameter packs are not move-able/copy-able
    // (different-sized tuples of parameter packs make the tuple default copy or
    // move contructor not usable).

    template <std::size_t set_index, std::size_t inner_array_size>
    constexpr auto set(const Multi_Array<T, inner_array_size> inner_array)
    {
        if constexpr (set_index == 0)
        {
            const auto inserted_tuple = std::make_tuple(inner_array);
            const auto end_of_tuple =
                m_parameter_pack
                    .template slice<1, (m_parameter_pack.size - 1)>();
            const auto full_tuple =
                std::tuple_cat(inserted_tuple, end_of_tuple);

            return make_jagged_array_from_tuple(full_tuple);
        }
        else if constexpr (set_index == (size - 1))
        {
            const auto start_of_tuple =
                m_parameter_pack.template slice<0, (set_index - 1)>();
            const auto inserted_tuple = std::make_tuple(inner_array);
            const auto full_tuple =
                std::tuple_cat(start_of_tuple, inserted_tuple);

            return make_jagged_array_from_tuple(full_tuple);
        }
        // Note: This is the only case where we grow the jagged array.
        else if constexpr (set_index > (size - 1))
        {
            const auto start_of_tuple = m_parameter_pack.to_tuple();
            const tuple_of_empty_types<T, ((size - 1) - set_index - 1)> padding;
            const auto inserted_tuple = std::make_tuple(inner_array);
            const auto full_tuple =
                std::tuple_cat(start_of_tuple, padding, inserted_tuple);

            return make_jagged_array_from_tuple(full_tuple);
        }
        else
        {
            const auto start_of_tuple =
                m_parameter_pack.template slice<0, (set_index - 1)>();
            const auto inserted_tuple = std::make_tuple(inner_array);
            const auto end_of_tuple =
                m_parameter_pack.template slice<(set_index + 1),
                                                (m_parameter_pack.size - 1)>();
            const auto full_tuple =
                std::tuple_cat(start_of_tuple, inserted_tuple, end_of_tuple);

            return make_jagged_array_from_tuple(full_tuple);
        }
    }

    constexpr auto get(const std::size_t inner_array_index,
                       const std::size_t element_index) const
    {
        return std::visit(
            [element_index](const auto& array) -> T
            {
                using T_no_ref = std::remove_cvref_t<decltype(array)>;

                if constexpr (std::is_same_v<T_no_ref, std::monostate>)
                {
                    throw std::out_of_range(
                        "RUNTIME ERROR: Jagged Array indexed monostate.");
                }
                else
                {
                    return array.get()[element_index];
                }
            },
            m_parameter_pack[inner_array_index]);
    }

    template <std::size_t size>
    static constexpr auto make()
    {
        using Tuple       = tuple_of_empty_types<T, size>;
        using Output_Type = typename Tuple_to_Jagged_Array<Tuple>::Type;

        return Output_Type {};
    }
};

// =============================================================================
// Partially Filled Array
// =============================================================================
template <typename T, std::size_t capacity>
class Partially_Filled_Array
{
  public:

    Partially_Filled_Array();

    static constexpr std::size_t max_capacity = capacity;

    inline std::size_t size() const;

    inline void append(const T& data);
    inline T    pop();
    inline void clear();

    inline T& front();
    inline T& back();

    T* begin();
    T* end();

    const T* begin() const;
    const T* end() const;

    int64_t get_max_index() const;
    int64_t truncate(const int64_t max_index);

    T&       operator[](const std::size_t index);
    const T& operator[](const std::size_t index) const;

  private:

    int64_t                  m_max_index;
    Multi_Array<T, capacity> m_list;
};

template <typename T, std::size_t capacity>
Partially_Filled_Array<T, capacity>::Partially_Filled_Array() : m_max_index(-1)
{
}

template <typename T, std::size_t capacity>
inline std::size_t Partially_Filled_Array<T, capacity>::size() const
{
    return static_cast<std::size_t>(m_max_index + 1);
}

template <typename T, std::size_t capacity>
inline void Partially_Filled_Array<T, capacity>::append(const T& data)
{
    ++m_max_index;
    m_list[m_max_index] = data;
}

template <typename T, std::size_t capacity>
inline T Partially_Filled_Array<T, capacity>::pop()
{
    --m_max_index;
    return m_list[m_max_index + 1];
}

template <typename T, std::size_t capacity>
inline void Partially_Filled_Array<T, capacity>::clear()
{
    m_max_index = -1;
}

template <typename T, std::size_t capacity>
inline T& Partially_Filled_Array<T, capacity>::front()
{
    return m_list[0];
}

template <typename T, std::size_t capacity>
inline T& Partially_Filled_Array<T, capacity>::back()
{
    return m_list[m_max_index];
}

template <typename T, std::size_t capacity>
T* Partially_Filled_Array<T, capacity>::begin()
{
    return (T*) &m_list[0];
}

template <typename T, std::size_t capacity>
T* Partially_Filled_Array<T, capacity>::end()
{
    return (T*) &m_list[m_max_index + 1];
}

template <typename T, std::size_t capacity>
const T* Partially_Filled_Array<T, capacity>::begin() const
{
    return (T*) &m_list[0];
}

template <typename T, std::size_t capacity>
const T* Partially_Filled_Array<T, capacity>::end() const
{
    return (T*) &m_list[m_max_index + 1];
}

template <typename T, std::size_t capacity>
int64_t Partially_Filled_Array<T, capacity>::get_max_index() const
{
    return m_max_index;
}

template <typename T, std::size_t capacity>
int64_t Partially_Filled_Array<T, capacity>::truncate(const int64_t max_index)
{
    m_max_index = std::min(m_max_index, max_index);
    return m_max_index;
}

template <typename T, std::size_t capacity>
T& Partially_Filled_Array<T, capacity>::operator[](const std::size_t index)
{
    MATREX_ASSERT(index < capacity,
                  "Partially_Filled_Array Assertion FAILURE: operator[] "
                  "Indexed outside of capacity. Index: {}, Capacity: {}",
                  index,
                  capacity);

    const int64_t index_i64 = static_cast<int64_t>(index);

    // Caution: This allows writes above the max index but below the capacity.
    if (index_i64 > m_max_index) { m_max_index = index_i64; }

    return m_list[index];
}

template <typename T, std::size_t capacity>
const T&
Partially_Filled_Array<T, capacity>::operator[](const std::size_t index) const
{
    MATREX_ASSERT(index < capacity,
                  "Partially_Filled_Array Assertion FAILURE: operator[] "
                  "Indexed outside of capacity. Index: {}, Capacity: {}",
                  index,
                  capacity);

    MATREX_ASSERT(static_cast<int64_t>(index) <= m_max_index,
                  "Partially_Filled_Array Assertion FAILURE: operator[] "
                  "Indexed outside of max index. Index: {}, Max Index: {}",
                  index,
                  m_max_index);

    return m_list[index];
}
