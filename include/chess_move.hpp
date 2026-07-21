#pragma once

#include <algorithm>
#include <array>
#include <iostream>

#include "globals.hpp"
#include "score.hpp"
#include "square.hpp"

constexpr std::size_t MAXIMUM_NUM_OF_MOVES_IN_A_POSITION = 256;
constexpr std::size_t PRINCIPAL_VARIATION_LIST_CAPACITY  = 256;

typedef int16_t Move_Score; // Not the same as a search evaluation score.

struct Chess_Move
{
    ESQUARE    source_square                    : 7  = NO_SQUARE; // 7
    ESQUARE    destination_square               : 7  = NO_SQUARE; // 14
    PIECES     moving_piece                     : 4  = NO_PIECE;  // 18
    PIECES     promoted_piece                   : 4  = NO_PIECE;  // 22
    PIECES     captured_piece                   : 4  = NO_PIECE;  // 26
    bool       is_capture                       : 1  = false;     // 27
    bool       is_short_castling                : 1  = false;     // 28
    bool       is_long_castling                 : 1  = false;     // 29
    ESQUARE    castling_rook_source_square      : 7  = NO_SQUARE; // 36
    ESQUARE    castling_rook_destination_square : 7  = NO_SQUARE; // 43
    bool       is_double_pawn_push              : 1  = false;     // 44
    bool       is_en_passant                    : 1  = false;     // 45
    ESQUARE    en_passant_victim_square         : 7  = NO_SQUARE; // 52
    bool       is_promotion                     : 1  = false;     // 53
    uint16_t   padding                          : 11 = 0;         // 64
    Move_Score score                                 = 0;         // 80

    constexpr static Chess_Move reversible_move(const PIECES piece,
                                                const Square source,
                                                const Square destination)
    {
        return Chess_Move {
            .source_square      = static_cast<ESQUARE>(source.get_index()),
            .destination_square = static_cast<ESQUARE>(destination.get_index()),
            .moving_piece       = piece,
            .promoted_piece     = static_cast<PIECES>(0),
            .captured_piece     = static_cast<PIECES>(0),
            .is_capture         = false,
            .is_short_castling  = false,
            .is_long_castling   = false,
            .castling_rook_source_square      = static_cast<ESQUARE>(0),
            .castling_rook_destination_square = static_cast<ESQUARE>(0),
            .is_double_pawn_push              = false,
            .is_en_passant                    = false,
            .en_passant_victim_square         = static_cast<ESQUARE>(0),
            .is_promotion                     = false};
    }

    std::string to_coordinate_notation(const bool is_frc) const
    {
        const std::string source_square_str = SQUARE_STRINGS[source_square];
        std::string       destination_square_str = "";

        if ((is_short_castling || is_long_castling) && is_frc)
        {
            destination_square_str =
                SQUARE_STRINGS[castling_rook_source_square];
        }
        else
        {
            destination_square_str = SQUARE_STRINGS[destination_square];
        }

        std::string promotion_string = "";

        switch (promoted_piece)
        {
            case (PIECES::KNIGHT): promotion_string = "n"; break;
            case (PIECES::BISHOP): promotion_string = "b"; break;
            case (PIECES::ROOK)  : promotion_string = "r"; break;
            case (PIECES::QUEEN) : promotion_string = "q"; break;
            default              : promotion_string = "";
        }

        return (source_square_str + destination_square_str + promotion_string);
    }

    void pretty_print() const
    {
        std::cout << "Move (" << SQUARE_STRINGS[source_square] << " to "
                  << SQUARE_STRINGS[destination_square] << "):" << std::endl;
        std::cout << "\tMoving piece: " << moving_piece << "\n"
                  << "\tPromoted piece: " << promoted_piece << "\n"
                  << "\tCaptured piece: " << captured_piece << "\n"
                  << "\tis_capture: " << is_capture << "\n"
                  << "\tis_short_castling: " << is_short_castling << "\n"
                  << "\tis_long_castling: " << is_long_castling << "\n"
                  << "\tCastling Rook Source Square: "
                  << SQUARE_STRINGS[castling_rook_source_square] << "\n"
                  << "\tCastling Rook Target Square: "
                  << SQUARE_STRINGS[castling_rook_destination_square] << "\n"
                  << "\tis_double_pawn_push: " << is_double_pawn_push << "\n"
                  << "\tis_en_passant: " << is_en_passant << "\n"
                  << "\tEn Passsant Victim Square: "
                  << SQUARE_STRINGS[en_passant_victim_square] << "\n"
                  << "\tis_promotion: " << is_promotion << "\n"
                  << "\tScore: " << score << std::endl;
    }

    constexpr bool is_same_move(const Chess_Move& m) const
    {
        return (m.source_square == source_square)
            && (m.destination_square == destination_square)
            && (m.moving_piece == moving_piece)
            && (m.promoted_piece == promoted_piece)
            && (m.captured_piece == captured_piece)
            && (m.is_capture == is_capture)
            && (m.is_short_castling == is_short_castling)
            && (m.is_long_castling == is_long_castling)
            && (m.castling_rook_source_square == castling_rook_source_square)
            && (m.castling_rook_destination_square
                == castling_rook_destination_square)
            && (m.is_double_pawn_push == is_double_pawn_push)
            && (m.is_en_passant == is_en_passant)
            && (m.en_passant_victim_square == en_passant_victim_square)
            && (m.is_promotion == is_promotion);
    }

    // All comparison operators for chess moves are based on the score they
    // recieve from move ordering.
    inline auto operator<=>(const Chess_Move& other) const
    {
        return (score <=> other.score);
    }

    bool is_noisy_move() const
    {
        return (is_capture || is_en_passant || is_promotion);
    }

    bool is_quiet_move() const { return (!is_noisy_move()); }
};

struct Undo_Chess_Move
{
    Chess_Move move;
    uint8_t    castling_rights     : 4;
    uint8_t    half_move_clock     : 7;
    ESQUARE    enpassant_square    : 7;
    uint16_t   hash_history_start  : 7;
    uint16_t   hash_history_length : 7;
};

// A partially filled array class specialized for Chess Moves.
template <std::size_t capacity>
class Chess_Move_List
{
  public:

    Chess_Move_List();

    template <std::size_t S>
    inline void push_and_copy(const Chess_Move&         move,
                              const Chess_Move_List<S>& move_list);
    inline void append(const Chess_Move& move);
    inline void clear();

    Chess_Move* begin() const;
    Chess_Move* end() const;

    int16_t get_max_index() const;

    Chess_Move& operator[](const uint16_t index);

    void sort();

    template <std::size_t S>
    friend std::ostream& operator<<(std::ostream&             os,
                                    const Chess_Move_List<S>& moves);

  private:

    int16_t                           m_max_index;
    Multi_Array<Chess_Move, capacity> m_list;
};

template <std::size_t capacity>
Chess_Move_List<capacity>::Chess_Move_List() : m_max_index(-1)
{
}

// This operation is used for collecting the principal variation. As you go down
// the search tree, the principal variation is built by pushing any move that
// raised alpha to the parent's principal variation list and copying the child's
// principal variation list after the pushed move. The reason we push to the
// front is because as you go back up the search tree, plies decrease.
template <std::size_t capacity>
template <std::size_t S>
inline void
Chess_Move_List<capacity>::push_and_copy(const Chess_Move&         move,
                                         const Chess_Move_List<S>& move_list)
{
    m_list[0] = move;

    if (move_list.get_max_index() != -1)
    {
        std::copy(move_list.begin(), move_list.end(), (m_list.begin() + 1));
        m_max_index = move_list.m_max_index + 1;
    }
    else
    {
        m_max_index = 0;
    }
}

template <std::size_t capacity>
inline void Chess_Move_List<capacity>::append(const Chess_Move& move)
{
    m_max_index++;
    m_list[m_max_index] = move;
}

template <std::size_t capacity>
inline void Chess_Move_List<capacity>::clear()
{
    m_max_index = -1;
}

template <std::size_t capacity>
Chess_Move* Chess_Move_List<capacity>::begin() const
{
    return (Chess_Move*) &m_list[0];
}

template <std::size_t capacity>
Chess_Move* Chess_Move_List<capacity>::end() const
{
    return (Chess_Move*) &m_list[m_max_index + 1];
}

template <std::size_t capacity>
int16_t Chess_Move_List<capacity>::get_max_index() const
{
    return m_max_index;
}

template <std::size_t capacity>
Chess_Move& Chess_Move_List<capacity>::operator[](const uint16_t index)
{
    return m_list[index];
}

// Sort moves according to their score in non-ascending order.
template <std::size_t capacity>
void Chess_Move_List<capacity>::sort()
{
    std::stable_sort(begin(), end(), std::greater<Chess_Move>());
}

// Handles printing the principal variation.
template <std::size_t S>
std::ostream& operator<<(std::ostream& os, const Chess_Move_List<S>& moves)
{
    for (const Chess_Move& move : moves)
    {
        os << " " << move.to_coordinate_notation(false);
    }

    return os;
}

using Move_Generation_List =
    Chess_Move_List<MAXIMUM_NUM_OF_MOVES_IN_A_POSITION>;
using Principal_Variation_List =
    Chess_Move_List<PRINCIPAL_VARIATION_LIST_CAPACITY>;
