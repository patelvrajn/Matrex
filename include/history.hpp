#pragma once

#include "chess_move.hpp"
#include "globals.hpp"

#include <algorithm>
#include <functional>

using History_Score_Storage_Type = Move_Score;

// Maximum history score is one below what the transposition table's score is.
constexpr History_Score_Storage_Type MAX_HISTORY =
    std::numeric_limits<History_Score_Storage_Type>::max() - 1;

// Arbitrarily selected minimum and maximum history bonuses.
constexpr History_Score_Storage_Type MIN_QUIET_HISTORY_BONUS = -128;
constexpr History_Score_Storage_Type MAX_QUIET_HISTORY_BONUS = 128;

constexpr History_Score_Storage_Type MIN_CAPTURE_HISTORY_BONUS = -128;
constexpr History_Score_Storage_Type MAX_CAPTURE_HISTORY_BONUS = 128;

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
    void gravity_update(const Chess_Move&          move,
                        History_Score_Storage_Type bonus);

    void clear();

  private:

    multi_array<History_Score_Storage_Type,
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

    multi_array<Quiet_History_Table,
                NUM_OF_UNIQUE_PIECES_PER_PLAYER,
                NUM_OF_SQUARES_ON_CHESS_BOARD>
        m_table;
};

template <std::size_t STACK_SIZE>
class Quiet_Continuation_History_Stack
{
  public:

    Quiet_Continuation_History_Stack();

    void bind_to_history_table(Quiet_History_Table& table, std::size_t index);

    Partially_Filled_Array<Optional_Reference<Quiet_History_Table>, STACK_SIZE>
        stack;
};

class Capture_History_Table
{
  public:

    Capture_History_Table();

    History_Score_Storage_Type& operator[](const Chess_Move& move);

    const History_Score_Storage_Type& operator[](const Chess_Move& move) const;

    template <bool is_malus>
    void gravity_update(const Chess_Move&          move,
                        History_Score_Storage_Type bonus);

    void clear();

  private:

    multi_array<History_Score_Storage_Type,
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

    multi_array<Capture_History_Table,
                NUM_OF_UNIQUE_PIECES_PER_PLAYER, // Moving piece
                NUM_OF_SQUARES_ON_CHESS_BOARD>   // Destination square
        m_table;
};

template <std::size_t STACK_SIZE>
class Capture_Continuation_History_Stack
{
  public:

    Capture_Continuation_History_Stack();

    void bind_to_history_table(Capture_History_Table& table, std::size_t index);

    Partially_Filled_Array<Optional_Reference<Capture_History_Table>,
                           STACK_SIZE>
        stack;
};

template <bool is_malus>
void Quiet_History_Table::gravity_update(const Chess_Move&          move,
                                         History_Score_Storage_Type bonus)
{
    auto& selected_entry = m_table[move.moving_piece][move.destination_square];

    // Clamp the bonus before gravity is applied.
    History_Score_Storage_Type clamped_bonus =
        std::clamp(bonus, MIN_QUIET_HISTORY_BONUS, MAX_QUIET_HISTORY_BONUS);

    // History gravity is simply the closer you are to the max history value,
    // the more the update is saturated.
    History_Score_Storage_Type gravitized_bonus =
        static_cast<History_Score_Storage_Type>(
            static_cast<double>(clamped_bonus)
            * (1.0
               - (static_cast<double>(std::abs(selected_entry))
                  / static_cast<double>(MAX_HISTORY))));

    // Select whether the bonus is applied as a penalty or not at compile-time.
    if constexpr (is_malus) { selected_entry -= gravitized_bonus; }
    else
    {
        selected_entry += gravitized_bonus;
    }
}

template <bool is_malus>
void Capture_History_Table::gravity_update(const Chess_Move&          move,
                                           History_Score_Storage_Type bonus)
{
    auto& selected_entry = m_table[move.moving_piece][move.destination_square]
                                  [move.captured_piece];

    // Clamp the bonus before gravity is applied.
    History_Score_Storage_Type clamped_bonus =
        std::clamp(bonus, MIN_CAPTURE_HISTORY_BONUS, MAX_CAPTURE_HISTORY_BONUS);

    // History gravity is simply the closer you are to the max history value,
    // the more the update is saturated.
    History_Score_Storage_Type gravitized_bonus =
        static_cast<History_Score_Storage_Type>(
            static_cast<double>(clamped_bonus)
            * (1.0
               - (static_cast<double>(std::abs(selected_entry))
                  / static_cast<double>(MAX_HISTORY))));

    // Select whether the bonus is applied as a penalty or not at compile-time.
    if constexpr (is_malus) { selected_entry -= gravitized_bonus; }
    else
    {
        selected_entry += gravitized_bonus;
    }
}

template <std::size_t STACK_SIZE>
Quiet_Continuation_History_Stack<STACK_SIZE>::Quiet_Continuation_History_Stack()
{
}

template <std::size_t STACK_SIZE>
void Quiet_Continuation_History_Stack<STACK_SIZE>::bind_to_history_table(
    Quiet_History_Table& table,
    std::size_t          index)
{
    stack[index] = table;
}

template <std::size_t STACK_SIZE>
Capture_Continuation_History_Stack<
    STACK_SIZE>::Capture_Continuation_History_Stack()
{
}

template <std::size_t STACK_SIZE>
void Capture_Continuation_History_Stack<STACK_SIZE>::bind_to_history_table(
    Capture_History_Table& table,
    std::size_t            index)
{
    stack[index] = table;
}
