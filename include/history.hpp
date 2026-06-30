#include "chess_move.hpp"
#include "globals.hpp"

#include <algorithm>
#include <functional>

using History_Score_Storage_Type = Move_Score;

// Maximum history score is one below what the transposition table's score is.
constexpr History_Score_Storage_Type MAX_HISTORY =
    std::numeric_limits<History_Score_Storage_Type>::max() - 1;

// Arbitrarily selected minimum and maximum history bonuses.
constexpr History_Score_Storage_Type MIN_HISTORY_BONUS =
    -1 * (MAX_HISTORY / 16);
constexpr History_Score_Storage_Type MAX_HISTORY_BONUS = MAX_HISTORY / 16;

class History_Table
{
  public:

    History_Table();

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

class Continuation_History_Table
{
  public:

    Continuation_History_Table();

    History_Table& operator[](const Chess_Move& move);

    const History_Table& operator[](const Chess_Move& move) const;

    void clear();

  private:

    multi_array<History_Table,
                NUM_OF_UNIQUE_PIECES_PER_PLAYER,
                NUM_OF_SQUARES_ON_CHESS_BOARD>
        m_table;
};

template <std::size_t STACK_SIZE>
class Continuation_History_Stack
{
  public:

    Continuation_History_Stack();

    void bind_to_history_table(History_Table& data, std::size_t index);

  private:

    Partially_Filled_Array<Optional_Reference<History_Table>, STACK_SIZE>
        m_stack;
};

template <bool is_malus>
void History_Table::gravity_update(const Chess_Move&          move,
                                   History_Score_Storage_Type bonus)
{
    auto& selected_entry = m_table[move.moving_piece][move.destination_square];

    // Clamp the bonus before gravity is applied.
    History_Score_Storage_Type clamped_bonus =
        std::clamp(bonus, MIN_HISTORY_BONUS, MAX_HISTORY_BONUS);

    // History gravity is simply the closer you are to the max history value,
    // the more the update is saturated.
    History_Score_Storage_Type gravitized_bonus =
        (clamped_bonus * (1 - (std::abs(selected_entry) / MAX_HISTORY)));

    // Select whether the bonus is applied as a penalty or not at compile-time.
    if constexpr (is_malus) { selected_entry -= gravitized_bonus; }
    else
    {
        selected_entry += gravitized_bonus;
    }
}

template <std::size_t STACK_SIZE>
Continuation_History_Stack<STACK_SIZE>::Continuation_History_Stack()
{
}

template <std::size_t STACK_SIZE>
void Continuation_History_Stack<STACK_SIZE>::bind_to_history_table(
    History_Table& table,
    std::size_t    index)
{
    m_stack[index] = std::ref(table);
}
