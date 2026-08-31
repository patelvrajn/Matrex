#include "move_generator.hpp"

#include <algorithm>
#include <iostream>
#include "chess_board.hpp"
#include "globals.hpp"

Move_Generator::Move_Generator(const Chess_Board& cb) :
    m_chess_board(cb),
    m_enpassantable_checker(false),
    m_side_to_move_in_check(false)
{
}

Moves_Bitboard_Matrix::Moves_Bitboard_Matrix() :
    m_piece_index_masks {}, m_matrix {}
{
    std::fill(&m_max_indices[0],
              &m_max_indices[0] + sizeof(m_max_indices),
              int8_t(-1));

    std::fill(&m_index_mappings[0][0][0],
              &m_index_mappings[0][0][0] + sizeof(m_index_mappings),
              int8_t(-1));
}

bool Moves_Bitboard_Matrix::get_moves_bitboards(const PIECE_COLOR color,
                                                const PIECES      piece,
                                                const Square      piece_square,
                                                Moves_Bitboard&   output) const
{
    const int8_t index =
        m_index_mappings[color][piece][piece_square.get_index()];
    if (index == -1) { return false; }
    output = m_matrix[color][index];
    return true;
}

// Called externally to check if the side to move is in check. Never done
// internally to check if a specific side is in check so we may assume
// m_chess_board.get_side_to_move() is sufficient.
bool Move_Generator::is_side_to_move_in_check()
{
    const PIECE_COLOR moving_side = m_chess_board.get_side_to_move();
    if (moving_side == PIECE_COLOR::WHITE)
    {
        generate_check_mask<PIECE_COLOR::WHITE>();
    }
    else
    {
        generate_check_mask<PIECE_COLOR::BLACK>();
    }
    return m_side_to_move_in_check;
}

bool Move_Generator::is_pinned(const Bitboard pinned,
                               const Square   source_square) const
{
    return ((pinned & Bitboard(source_square.get_mask())).get_board() != 0);
}

Bitboard Move_Generator::attackers_to_square(const Square s)
{
    return attackers_to_square(
        s,
        m_chess_board.get_piece_occupancies(PIECE_COLOR::WHITE, PIECES::PAWN),
        m_chess_board.get_piece_occupancies(PIECE_COLOR::WHITE, PIECES::KNIGHT),
        m_chess_board.get_piece_occupancies(PIECE_COLOR::WHITE, PIECES::BISHOP),
        m_chess_board.get_piece_occupancies(PIECE_COLOR::WHITE, PIECES::ROOK),
        m_chess_board.get_piece_occupancies(PIECE_COLOR::WHITE, PIECES::QUEEN),
        m_chess_board.get_piece_occupancies(PIECE_COLOR::WHITE, PIECES::KING),
        m_chess_board.get_piece_occupancies(PIECE_COLOR::BLACK, PIECES::PAWN),
        m_chess_board.get_piece_occupancies(PIECE_COLOR::BLACK, PIECES::KNIGHT),
        m_chess_board.get_piece_occupancies(PIECE_COLOR::BLACK, PIECES::BISHOP),
        m_chess_board.get_piece_occupancies(PIECE_COLOR::BLACK, PIECES::ROOK),
        m_chess_board.get_piece_occupancies(PIECE_COLOR::BLACK, PIECES::QUEEN),
        m_chess_board.get_piece_occupancies(PIECE_COLOR::BLACK, PIECES::KING));
}

// Return a bitboard of ALL pieces (any color) that are currently attacking a
// given square.
Bitboard
Move_Generator::attackers_to_square(const Square   s,
                                    const Bitboard white_pawn_occupancy,
                                    const Bitboard white_knight_occupancy,
                                    const Bitboard white_bishop_occupancy,
                                    const Bitboard white_rook_occupancy,
                                    const Bitboard white_queen_occupancy,
                                    const Bitboard white_king_occupancy,
                                    const Bitboard black_pawn_occupancy,
                                    const Bitboard black_knight_occupancy,
                                    const Bitboard black_bishop_occupancy,
                                    const Bitboard black_rook_occupancy,
                                    const Bitboard black_queen_occupancy,
                                    const Bitboard black_king_occupancy)
{
    Bitboard attackers;

    Attacks a;

    Bitboard both_color_occupancies =
        white_pawn_occupancy | white_knight_occupancy | white_bishop_occupancy
        | white_rook_occupancy | white_queen_occupancy | white_king_occupancy
        | black_pawn_occupancy | black_knight_occupancy | black_bishop_occupancy
        | black_rook_occupancy | black_queen_occupancy | black_king_occupancy;

    // Pawn attacks.
    attackers |=
        (a.get_pawn_attacks(s, PIECE_COLOR::WHITE) & black_pawn_occupancy);

    attackers |=
        (a.get_pawn_attacks(s, PIECE_COLOR::BLACK) & white_pawn_occupancy);

    // Knight attacks.
    attackers |= (a.get_knight_attacks(s)
                  & (white_knight_occupancy | black_knight_occupancy));

    // Bishop and queen attacks. (Diagonal attacks)
    attackers |= (a.get_bishop_attacks(s, both_color_occupancies)
                  & (white_bishop_occupancy | white_queen_occupancy
                     | black_bishop_occupancy | black_queen_occupancy));

    // Rook and queen attacks. (Orthogonal attacks)
    attackers |= (a.get_rook_attacks(s, both_color_occupancies)
                  & (white_rook_occupancy | white_queen_occupancy
                     | black_rook_occupancy | black_queen_occupancy));

    // King attacks.
    attackers |=
        (a.get_king_attacks(s) & (white_king_occupancy | black_king_occupancy));

    return attackers;
}
