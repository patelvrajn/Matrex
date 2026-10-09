#pragma once

#include "chess_move.hpp"
#include "globals.hpp"

#include <algorithm>
#include <functional>
#include <utility>

using History_Score_Storage_Type = Move_Score;

// Maximum history score is one below what the transposition table's score is.
constexpr History_Score_Storage_Type MAX_HISTORY =
    std::numeric_limits<History_Score_Storage_Type>::max() - 1;

constexpr History_Score_Storage_Type MIN_HISTORY =
    std::numeric_limits<History_Score_Storage_Type>::min();

// Arbitrarily selected minimum and maximum history bonuses.
constexpr History_Score_Storage_Type MIN_QUIET_HISTORY_BONUS = -128;
constexpr History_Score_Storage_Type MAX_QUIET_HISTORY_BONUS = 128;

constexpr History_Score_Storage_Type MIN_CAPTURE_HISTORY_BONUS = -256;
constexpr History_Score_Storage_Type MAX_CAPTURE_HISTORY_BONUS = 256;

// Number of plies to look back in continuation histories.
constexpr std::size_t QUIET_CONTINUATION_HISTORY_LOOKBACK_DEPTH   = 4;
constexpr std::size_t CAPTURE_CONTINUATION_HISTORY_LOOKBACK_DEPTH = 2;

class Quiet_History_Table
{
  public:

    Quiet_History_Table();

    History_Score_Storage_Type& operator[](const Chess_Move& move);

    const History_Score_Storage_Type& operator[](const Chess_Move& move) const;

    template <bool is_malus>
    void gravity_update(const Chess_Move&                move,
                        const History_Score_Storage_Type change);

    void clear();

  private:

    Multi_Array<History_Score_Storage_Type,
                NUM_OF_UNIQUE_PIECES_PER_PLAYER,
                NUM_OF_SQUARES_ON_CHESS_BOARD>
        m_table;
};

class Quiet_Continuation_History_Table
{
  public:

    Quiet_Continuation_History_Table();

    Quiet_History_Table& operator[](const Chess_Move& move);

    const Quiet_History_Table& operator[](const Chess_Move& move) const;

    void clear();

  private:

    Multi_Array<Quiet_History_Table,
                NUM_OF_UNIQUE_PIECES_PER_PLAYER,
                NUM_OF_SQUARES_ON_CHESS_BOARD>
        m_table;
};

template <std::size_t STACK_SIZE>
class Quiet_Continuation_History_Stack
{
  public:

    using Table_Access = std::function<void(
        const std::function<void(Quiet_Continuation_History_Table&)>&)>;

    explicit Quiet_Continuation_History_Stack(Table_Access access);

    void bind_to_move(const Chess_Move& move, std::size_t index);

    History_Score_Storage_Type get_score(const Chess_Move& move) const;

    Partially_Filled_Array<Chess_Move, STACK_SIZE> stack;

  private:

    Table_Access m_access;
};

class Capture_History_Table
{
  public:

    Capture_History_Table();

    History_Score_Storage_Type& operator[](const Chess_Move& move);

    const History_Score_Storage_Type& operator[](const Chess_Move& move) const;

    template <bool is_malus>
    void gravity_update(const Chess_Move&                move,
                        const History_Score_Storage_Type change);

    void clear();

  private:

    Multi_Array<History_Score_Storage_Type,
                NUM_OF_UNIQUE_PIECES_PER_PLAYER, // Moving piece
                NUM_OF_SQUARES_ON_CHESS_BOARD,   // Destination square
                NUM_OF_UNIQUE_PIECES_PER_PLAYER> // Captured piece
        m_table;
};

class Capture_Continuation_History_Table
{
  public:

    Capture_Continuation_History_Table();

    Capture_History_Table& operator[](const Chess_Move& move);

    const Capture_History_Table& operator[](const Chess_Move& move) const;

    void clear();

  private:

    Multi_Array<Capture_History_Table,
                NUM_OF_UNIQUE_PIECES_PER_PLAYER, // Moving piece
                NUM_OF_SQUARES_ON_CHESS_BOARD>   // Destination square
        m_table;
};

template <std::size_t STACK_SIZE>
class Capture_Continuation_History_Stack
{
  public:

    // The table access function passes an operation on the shared continuation
    // history table to call_shared_data(), which locks the table, executes the
    // operation, and unlocks afterward i.e. a thread-safe way to perform 
    // operations on the shared continuation history table.
    using Table_Access = std::function<void(
        const std::function<void(Capture_Continuation_History_Table&)>&)>;

    explicit Capture_Continuation_History_Stack(Table_Access access);

    void bind_to_move(const Chess_Move& move, std::size_t index);

    History_Score_Storage_Type get_score(const Chess_Move& move) const;

    Partially_Filled_Array<Chess_Move, STACK_SIZE> stack;

  private:

    Table_Access m_access;
};

template <bool is_malus>
void Quiet_History_Table::gravity_update(
    const Chess_Move&                move,
    const History_Score_Storage_Type change)
{
    auto& selected_entry = m_table[move.moving_piece][move.destination_square];

    // Clamp the bonus before gravity is applied.
    const History_Score_Storage_Type clamped_change =
        std::clamp(change, MIN_QUIET_HISTORY_BONUS, MAX_QUIET_HISTORY_BONUS);

    // History gravity is simply the closer you are to the max history value,
    // the more the update is saturated.
    const History_Score_Storage_Type gravitized_change =
        static_cast<History_Score_Storage_Type>(
            static_cast<double>(clamped_change)
            * (1.0
               - std::abs(static_cast<double>(std::abs(selected_entry))
                          / static_cast<double>(is_malus ? MIN_HISTORY
                                                         : MAX_HISTORY))));

    // Select whether the bonus is applied as a penalty or not at compile-time.
    if constexpr (is_malus) { selected_entry -= gravitized_change; }
    else
    {
        selected_entry += gravitized_change;
    }
}

template <bool is_malus>
void Capture_History_Table::gravity_update(
    const Chess_Move&                move,
    const History_Score_Storage_Type change)
{
    auto& selected_entry = m_table[move.moving_piece][move.destination_square]
                                  [move.captured_piece];

    // Clamp the bonus before gravity is applied.
    const History_Score_Storage_Type clamped_change =
        std::clamp(change,
                   MIN_CAPTURE_HISTORY_BONUS,
                   MAX_CAPTURE_HISTORY_BONUS);

    // History gravity is simply the closer you are to the max history value,
    // the more the update is saturated.
    const History_Score_Storage_Type gravitized_change =
        static_cast<History_Score_Storage_Type>(
            static_cast<double>(clamped_change)
            * (1.0
               - std::abs(static_cast<double>(std::abs(selected_entry))
                          / static_cast<double>(is_malus ? MIN_HISTORY
                                                         : MAX_HISTORY))));

    // Select whether the bonus is applied as a penalty or not at compile-time.
    if constexpr (is_malus) { selected_entry -= gravitized_change; }
    else
    {
        selected_entry += gravitized_change;
    }
}

template <std::size_t STACK_SIZE>
Quiet_Continuation_History_Stack<STACK_SIZE>::Quiet_Continuation_History_Stack(
    Table_Access access) :
    m_access(std::move(access))
{
}

template <std::size_t STACK_SIZE>
void Quiet_Continuation_History_Stack<STACK_SIZE>::bind_to_move(
    const Chess_Move& move, const std::size_t index)
{
    stack[index] = move;
}

template <std::size_t STACK_SIZE>
History_Score_Storage_Type
Quiet_Continuation_History_Stack<STACK_SIZE>::get_score(
    const Chess_Move& move) const
{
    const std::size_t ply = stack.size();

    const int64_t start = static_cast<int64_t>(ply) - 1;
    const int64_t end   = (ply < QUIET_CONTINUATION_HISTORY_LOOKBACK_DEPTH)
                            ? 0
                            : static_cast<int64_t>(ply)
                                - static_cast<int64_t>(
                                    QUIET_CONTINUATION_HISTORY_LOOKBACK_DEPTH);

    int64_t score = 0;
    if ((start >= 0) && (end >= 0))
    {
        m_access([&](Quiet_Continuation_History_Table& table)
        {
            for (int64_t i = start; i >= end; --i)
            {
                score += table[stack[static_cast<std::size_t>(i)]][move];
            }
        });
    }

    return static_cast<History_Score_Storage_Type>(
        std::clamp(score,
                   static_cast<int64_t>(MIN_HISTORY),
                   static_cast<int64_t>(MAX_HISTORY)));
}

template <std::size_t STACK_SIZE>
Capture_Continuation_History_Stack<STACK_SIZE>::Capture_Continuation_History_Stack(
    Table_Access access) :
    m_access(std::move(access))
{
}

template <std::size_t STACK_SIZE>
void Capture_Continuation_History_Stack<STACK_SIZE>::bind_to_move(
    const Chess_Move& move, const std::size_t index)
{
    stack[index] = move;
}

template <std::size_t STACK_SIZE>
History_Score_Storage_Type
Capture_Continuation_History_Stack<STACK_SIZE>::get_score(
    const Chess_Move& move) const
{
    const std::size_t ply = stack.size();

    const int64_t start = static_cast<int64_t>(ply) - 1;
    const int64_t end =
        (ply < CAPTURE_CONTINUATION_HISTORY_LOOKBACK_DEPTH)
            ? 0
            : static_cast<int64_t>(ply)
                  - static_cast<int64_t>(
                      CAPTURE_CONTINUATION_HISTORY_LOOKBACK_DEPTH);

    int64_t score = 0;
    if ((start >= 0) && (end >= 0))
    {
        m_access([&](Capture_Continuation_History_Table& table)
        {
            for (int64_t i = start; i >= end; --i)
            {
                score += table[stack[static_cast<std::size_t>(i)]][move];
            }
        });
    }

    return static_cast<History_Score_Storage_Type>(
        std::clamp(score,
                   static_cast<int64_t>(MIN_HISTORY),
                   static_cast<int64_t>(MAX_HISTORY)));
}
