#include "chess_move.hpp"
#include "globals.hpp"

using History_Score_Storage_Type = int64_t;

class History_Table
{
  public:

    History_Table();

    History_Score_Storage_Type get_history_score(const Chess_Move move) const;

    void increment_history(const Chess_Move move);
    void decrement_history(const Chess_Move move);

    void update_history(const Chess_Move                 move,
                        const History_Score_Storage_Type history);

    void clear();

  private:

    multi_array<History_Score_Storage_Type,
                NUM_OF_UNIQUE_PIECES_PER_PLAYER,
                NUM_OF_SQUARES_ON_CHESS_BOARD>
        m_table;
};

using History_Table_Pair = multi_array<History_Table, NUM_OF_PLAYERS>;
