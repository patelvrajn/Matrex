#pragma once

#include <memory>
#include <vector>

#include "attacks.hpp"
#include "chess_board.hpp"
#include "chess_move.hpp"
#include "globals.hpp"

extern const Bitboard FIRST_RANK;
extern const Bitboard SECOND_RANK;
extern const Bitboard SEVENTH_RANK;
extern const Bitboard EIGHTH_RANK;

constexpr Multi_Array<Square, NUM_OF_PLAYERS, NUM_OF_CASTLING_TYPES>
    CASTLING_KING_DESTINATION_SQUARES = {
        {{Square(ESQUARE::G1), Square(ESQUARE::C1)},
         {Square(ESQUARE::G8), Square(ESQUARE::C8)}}
};

constexpr Multi_Array<Square, NUM_OF_PLAYERS, NUM_OF_CASTLING_TYPES>
    CASTLING_ROOK_DESTINATION_SQUARES = {
        {{Square(ESQUARE::F1), Square(ESQUARE::D1)},
         {Square(ESQUARE::F8), Square(ESQUARE::D8)}}
};

enum MOVE_GENERATION_TYPE
{
    ALL,
    TACTICAL,
    QUIET
};

struct Moves_Bitboard
{
    PIECES   piece;
    Square   square;
    Bitboard bitboard;
};

class Moves_Bitboard_Matrix
{
  public:

    Moves_Bitboard_Matrix();

    void set_move(PIECE_COLOR color,
                  PIECES      piece,
                  Square      piece_square,
                  Square      move_square);

    bool get_moves_bitboards(PIECE_COLOR     color,
                             PIECES          piece,
                             Square          piece_square,
                             Moves_Bitboard& output) const;

    bool get_piece_moves_bitboards(PIECE_COLOR                  color,
                                   PIECES                       piece,
                                   std::vector<Moves_Bitboard>& output) const;

  private:

    Multi_Array<int8_t, NUM_OF_PLAYERS> m_max_indices;

    Multi_Array<int8_t,
                NUM_OF_PLAYERS,
                NUM_OF_UNIQUE_PIECES_PER_PLAYER,
                NUM_OF_SQUARES_ON_CHESS_BOARD>
        m_index_mappings;

    Multi_Array<uint16_t, NUM_OF_PLAYERS, NUM_OF_UNIQUE_PIECES_PER_PLAYER>
        m_piece_index_masks;

    Multi_Array<Moves_Bitboard, NUM_OF_PLAYERS, NUM_OF_PIECES_PER_PLAYER>
        m_matrix;
};

class Move_Generator
{
  public:

    Move_Generator(const Chess_Board& cb);

    template <MOVE_GENERATION_TYPE gen_type>
    void generate_all_moves(Move_Generation_List&  output,
                            Moves_Bitboard_Matrix& matrix_output);

    template <MOVE_GENERATION_TYPE gen_type>
    void generate_all_moves(PIECE_COLOR            side,
                            Move_Generation_List&  output,
                            Moves_Bitboard_Matrix& matrix_output);

    bool is_side_to_move_in_check();

  private:

    const Chess_Board& m_chess_board;

    bool m_enpassantable_checker;
    bool m_side_to_move_in_check;

    template <PIECE_COLOR moving_side>
    Bitboard generate_check_mask();
    template <PIECE_COLOR moving_side>
    Bitboard generate_pinned() const;
    bool is_pinned(const Bitboard& pinned, const Square& source_square) const;
    template <PIECE_COLOR moving_side>
    Bitboard get_pin_mask(const Bitboard& pinned,
                          const Square&   source_square) const;
    Bitboard attackers_to_square(const Square&   s,
                                 const Bitboard& white_pawn_occupancy,
                                 const Bitboard& white_knight_occupancy,
                                 const Bitboard& white_bishop_occupancy,
                                 const Bitboard& white_rook_occupancy,
                                 const Bitboard& white_queen_occupancy,
                                 const Bitboard& white_king_occupancy,
                                 const Bitboard& black_pawn_occupancy,
                                 const Bitboard& black_knight_occupancy,
                                 const Bitboard& black_bishop_occupancy,
                                 const Bitboard& black_rook_occupancy,
                                 const Bitboard& black_queen_occupancy,
                                 const Bitboard& black_king_occupancy);
    Bitboard attackers_to_square(const Square& s);
    template <PIECE_COLOR moving_side>
    Bitboard is_our_king_ring_attacked();

    template <MOVE_GENERATION_TYPE gen_type, PIECE_COLOR moving_side>
    inline void generate_pawn_promotions(const Square&          source_square,
                                         const Square&          target_square,
                                         Move_Generation_List&  output,
                                         Moves_Bitboard_Matrix& matrix_output);

    template <MOVE_GENERATION_TYPE gen_type, PIECE_COLOR moving_side>
    inline void generate_single_push_promotion_pawn_moves(
        const Bitboard&        pinned,
        const Bitboard&        check_mask,
        Move_Generation_List&  output,
        Moves_Bitboard_Matrix& matrix_output);

    template <MOVE_GENERATION_TYPE gen_type, PIECE_COLOR moving_side>
    inline void generate_single_push_non_promotion_pawn_moves(
        const Bitboard&        pinned,
        const Bitboard&        check_mask,
        Move_Generation_List&  output,
        Moves_Bitboard_Matrix& matrix_output);

    template <MOVE_GENERATION_TYPE gen_type, PIECE_COLOR moving_side>
    inline void
    generate_double_push_pawn_moves(const Bitboard&        pinned,
                                    const Bitboard&        check_mask,
                                    Move_Generation_List&  output,
                                    Moves_Bitboard_Matrix& matrix_output);

    template <MOVE_GENERATION_TYPE gen_type, PIECE_COLOR moving_side>
    inline void
    generate_en_passant_captures(const Bitboard&        pinned,
                                 const Bitboard&        check_mask,
                                 Move_Generation_List&  output,
                                 Moves_Bitboard_Matrix& matrix_output);

    template <MOVE_GENERATION_TYPE gen_type, PIECE_COLOR moving_side>
    inline void
    generate_non_promotion_pawn_captures(const Bitboard&        pinned,
                                         const Bitboard&        check_mask,
                                         Move_Generation_List&  output,
                                         Moves_Bitboard_Matrix& matrix_output);

    template <MOVE_GENERATION_TYPE gen_type, PIECE_COLOR moving_side>
    inline void
    generate_promotion_pawn_captures(const Bitboard&        pinned,
                                     const Bitboard&        check_mask,
                                     Move_Generation_List&  output,
                                     Moves_Bitboard_Matrix& matrix_output);

    template <MOVE_GENERATION_TYPE gen_type,
              PIECE_COLOR          moving_side,
              PIECES               moving_piece>
    inline void
    generate_minor_and_major_piece_moves(const Bitboard&        pinned,
                                         const Bitboard&        check_mask,
                                         Move_Generation_List&  output,
                                         Moves_Bitboard_Matrix& matrix_output);

    template <MOVE_GENERATION_TYPE gen_type, PIECE_COLOR moving_side>
    inline void generate_king_moves(Move_Generation_List&  output,
                                    Moves_Bitboard_Matrix& matrix_output);

    template <MOVE_GENERATION_TYPE gen_type,
              PIECE_COLOR          moving_side,
              CASTLING_TYPE        castle_type>
    inline void generate_castling_moves(const Bitboard&        pinned,
                                        Move_Generation_List&  output,
                                        Moves_Bitboard_Matrix& matrix_output);
};

// Function: generate_check_mask
// Purpose: Creates a "check mask" bitboard used during move generation.
//          - If no check: all moves are legal (mask = all 1's).
//          - If single check: only moves that block or capture the checker are
//          legal.
//          - If double check: only king moves are legal (mask = empty).
template <PIECE_COLOR moving_side>
Bitboard Move_Generator::generate_check_mask()
{
    Bitboard checkers; // Bitboard that will hold all pieces currently checking
                       // the king.
    Attacks
        a; // Attack helper class to compute attack masks for different pieces.

    // Figure out the color of the *opponent* (the side delivering checks).
    // get_side_to_move() = side whose turn it is.
    // ~ flips the bit, & 0x1 ensures we only keep 1-bit color info (WHITE=0,
    // BLACK=1).
    constexpr PIECE_COLOR opposing_side = ~moving_side;

    // The square our king is on.
    const Square our_king_square = m_chess_board.get_king_square(moving_side);

    // The squares on a bitboard occupied by any side's pieces.
    const Bitboard both_color_occupancies =
        m_chess_board.get_both_color_occupancies();

    // --- Find all enemy pawns that give check ---
    // 1. Get pawn attack squares relative to the king's position.
    // 2. AND it with all opponent pawns currently on the board.
    // 3. Result: opponent pawns that are directly attacking our king.
    checkers |=
        (a.get_pawn_attacks(our_king_square, moving_side)
         & m_chess_board.get_piece_occupancies(opposing_side, PIECES::PAWN));

    // --- Find all enemy knights giving check ---
    // Similar logic: knight attack mask from king's square & opponent knights.
    checkers |=
        (a.get_knight_attacks(our_king_square)
         & m_chess_board.get_piece_occupancies(opposing_side, PIECES::KNIGHT));

    // --- Find all enemy bishops or queens giving check (diagonal sliders) ---
    // Use bishop attack rays from the king's square, intersect with enemy
    // bishops
    // + queens.
    checkers |=
        (a.get_bishop_attacks(our_king_square, both_color_occupancies)
         & (m_chess_board.get_piece_occupancies(opposing_side, PIECES::BISHOP)
            | m_chess_board.get_piece_occupancies(opposing_side,
                                                  PIECES::QUEEN)));

    // --- Find all enemy rooks or queens giving check (orthogonal sliders) ---
    // Use rook attack rays from the king's square, intersect with enemy rooks +
    // queens.
    checkers |=
        (a.get_rook_attacks(our_king_square, both_color_occupancies)
         & (m_chess_board.get_piece_occupancies(opposing_side, PIECES::ROOK)
            | m_chess_board.get_piece_occupancies(opposing_side,
                                                  PIECES::QUEEN)));

    // --- Case 1: No check at all ---
    // If "checkers" is empty (king is not under check),
    // return a mask of all 1s (~0ULL), meaning all moves are legal.
    if (checkers == Bitboard(0)) { return Bitboard(~0ULL); }
    else
    {
        // Checkers is not empty, side to move is in check.
        m_side_to_move_in_check = true;
    }

    // --- Case 2: Double check detection ---
    // A double check means TWO pieces are giving check simultaneously.
    // To detect: temporarily clear the least significant checker
    // and see if any other bits remain set.
    Bitboard double_check = checkers;
    double_check.unset_square(Square(checkers.get_index_of_high_lsb()));

    // If still non-empty, it means we had more than one checker.
    // In double check, ONLY king moves are legal, so return empty mask.
    if (double_check.get_board()) { return Bitboard(0); }

    // --- Case 3: Single check ---
    // If only ONE checker exists, then legal moves must either:
    //   (1) Capture the checking piece directly, OR
    //   (2) Block the attack ray (only possible against sliding checkers:
    //       bishop/rook/queen).
    //
    // Special case: if the checker is a pawn that just advanced two squares and
    // is en-passant capturable, an en passant capture could neutralize the
    // check. We do NOT add the en passant target square to the generic mask
    // here, because that would incorrectly allow other pieces to "move" there.
    // Instead, we record this condition in `m_enpassantable_checker`, and the
    // pawn move generator will allow the en passant capture specifically.
    const Square checker_square = Square(checkers.get_index_of_high_lsb());

    if (m_chess_board.get_en_passant_square().get_index() != ESQUARE::NO_SQUARE)
    {
        if (m_chess_board.get_en_passant_victim_square() == checker_square)
        {
            m_enpassantable_checker = true;
        }
    }

    return (Bitboard(checkers.get_between_squares_mask(our_king_square,
                                                       checker_square))
            | checkers);
}

template <PIECE_COLOR moving_side>
Bitboard Move_Generator::generate_pinned() const
{
    Attacks a;

    // Opposite color (side not moving) — bit hack: flip 0 ↔ 1
    constexpr PIECE_COLOR opposing_side = ~moving_side;

    const Bitboard our_king_occupancy =
        m_chess_board.get_piece_occupancies(moving_side, PIECES::KING);
    const Square our_king_square = m_chess_board.get_king_square(moving_side);

    const Bitboard friendly_pieces =
        m_chess_board.get_color_occupancies(moving_side);
    const Bitboard enemy_pieces =
        m_chess_board.get_color_occupancies(opposing_side);

    const Bitboard enemy_orthogonal_pieces =
        m_chess_board.get_piece_occupancies(opposing_side, PIECES::ROOK)
        | m_chess_board.get_piece_occupancies(opposing_side, PIECES::QUEEN);
    const Bitboard enemy_diagonal_pieces =
        m_chess_board.get_piece_occupancies(opposing_side, PIECES::BISHOP)
        | m_chess_board.get_piece_occupancies(opposing_side, PIECES::QUEEN);

    const Bitboard orthogonal_rays =
        a.get_rook_attacks(our_king_square, enemy_pieces);
    const Bitboard diagonal_rays =
        a.get_bishop_attacks(our_king_square, enemy_pieces);

    Bitboard orthogonal_pinners = enemy_orthogonal_pieces & orthogonal_rays;
    Bitboard diagonal_pinners   = enemy_diagonal_pieces & diagonal_rays;

    Bitboard pinned;

    while (orthogonal_pinners.get_board())
    {
        const Square pinner_square =
            Square(orthogonal_pinners.get_index_of_high_lsb());

        const Bitboard ray_from_pinner_to_king =
            orthogonal_rays
            & a.get_rook_attacks(pinner_square, our_king_occupancy);

        const Bitboard potentially_pinned =
            ray_from_pinner_to_king & friendly_pieces;

        if (potentially_pinned.high_bit_count() == 1)
        {
            pinned |= potentially_pinned;
        }

        orthogonal_pinners.unset_square(pinner_square);
    }

    while (diagonal_pinners.get_board())
    {
        const Square pinner_square =
            Square(diagonal_pinners.get_index_of_high_lsb());

        const Bitboard ray_from_pinner_to_king =
            diagonal_rays
            & a.get_bishop_attacks(pinner_square, our_king_occupancy);

        const Bitboard potentially_pinned =
            ray_from_pinner_to_king & friendly_pieces;

        if (potentially_pinned.high_bit_count() == 1)
        {
            pinned |= potentially_pinned;
        }

        diagonal_pinners.unset_square(pinner_square);
    }

    return pinned;
}

template <PIECE_COLOR moving_side>
Bitboard Move_Generator::get_pin_mask(const Bitboard& pinned,
                                      const Square&   source_square) const
{
    const Square our_king_square = m_chess_board.get_king_square(moving_side);

    if (is_pinned(pinned, source_square))
    {
        return Attacks::get_infinite_ray(our_king_square, source_square);
    }
    else
    {
        return Bitboard((uint64_t) -1);
    }
}

// ======================================================================
// Function : Move_Generator::is_our_king_ring_attacked
// Purpose  : Determine which squares in the "king ring" (the 8 surrounding
//            squares around *our* king) are currently under attack
//            by the opponent.
// Returns  : Bitboard mask of attacked king-ring squares.
// ======================================================================
template <PIECE_COLOR moving_side>
Bitboard Move_Generator::is_our_king_ring_attacked()
{
    Bitboard mask; // Final result: set of attacked squares around our king
    Attacks  a;    // Attack pattern generator (precomputed masks + magics)

    // ------------------------------------------------------
    // Determine sides
    // ------------------------------------------------------
    // The OPPOSING side (not our_side).
    // Trick: (~our_side) flips bits, & 0x1 makes it 0 ↔ 1.
    // Example: WHITE=0, BLACK=1 → flips nicely.
    const PIECE_COLOR opposing_side = ~moving_side;

    // ------------------------------------------------------
    // Get our king’s position
    // ------------------------------------------------------

    // Find the square that our king occupies.
    // Required because we want to look at the 8 surrounding squares.
    const Square our_king_square = m_chess_board.get_king_square(moving_side);

    // Build a "king ring" bitboard: all squares a king could move to
    // if it stood on our_king_square (i.e. its 8 neighbors).
    Bitboard king_ring = a.get_king_attacks(our_king_square);

    // ------------------------------------------------------
    // Loop through each square in the king ring
    // ------------------------------------------------------

    while (king_ring.get_board())
    {
        // Extract the *lowest set bit* (one square from king ring).
        // `get_index_of_high_lsb()` = index of least significant bit set.
        const Square king_ring_square =
            Square(king_ring.get_index_of_high_lsb());

        Bitboard black_king_occupancy = Bitboard(
            m_chess_board
                .get_piece_occupancies(PIECE_COLOR::BLACK, PIECES::KING)
                .get_board()
            * (moving_side == PIECE_COLOR::WHITE));
        Bitboard white_king_occupancy = Bitboard(
            m_chess_board
                .get_piece_occupancies(PIECE_COLOR::WHITE, PIECES::KING)
                .get_board()
            * (moving_side == PIECE_COLOR::BLACK));

        Bitboard king_ring_square_attackers = attackers_to_square(
            king_ring_square,
            m_chess_board.get_piece_occupancies(PIECE_COLOR::WHITE,
                                                PIECES::PAWN),
            m_chess_board.get_piece_occupancies(PIECE_COLOR::WHITE,
                                                PIECES::KNIGHT),
            m_chess_board.get_piece_occupancies(PIECE_COLOR::WHITE,
                                                PIECES::BISHOP),
            m_chess_board.get_piece_occupancies(PIECE_COLOR::WHITE,
                                                PIECES::ROOK),
            m_chess_board.get_piece_occupancies(PIECE_COLOR::WHITE,
                                                PIECES::QUEEN),
            white_king_occupancy,
            m_chess_board.get_piece_occupancies(PIECE_COLOR::BLACK,
                                                PIECES::PAWN),
            m_chess_board.get_piece_occupancies(PIECE_COLOR::BLACK,
                                                PIECES::KNIGHT),
            m_chess_board.get_piece_occupancies(PIECE_COLOR::BLACK,
                                                PIECES::BISHOP),
            m_chess_board.get_piece_occupancies(PIECE_COLOR::BLACK,
                                                PIECES::ROOK),
            m_chess_board.get_piece_occupancies(PIECE_COLOR::BLACK,
                                                PIECES::QUEEN),
            black_king_occupancy);

        // Check if that king-ring square is attacked:
        //   - attackers_to_square(square) → all pieces attacking this square
        //   - & m_chess_board.get_color_occupancies(opposing_side) → restrict
        //   to enemy side
        //   - If result != 0 → at least one enemy piece attacks this square.
        if ((king_ring_square_attackers
             & m_chess_board.get_color_occupancies(opposing_side))
                .get_board())
        {
            // If attacked, set that square’s bit in our result mask.
            mask |= Bitboard(king_ring_square.get_mask());
        }

        // Remove this square from the king_ring bitboard (unset processed
        // square).
        king_ring.unset_square(king_ring_square);
    }

    // ------------------------------------------------------
    // Done: return the mask of all attacked king-ring squares
    // ------------------------------------------------------
    return mask;
}

template <MOVE_GENERATION_TYPE gen_type, PIECE_COLOR moving_side>
inline void
Move_Generator::generate_pawn_promotions(const Square& source_square,
                                         const Square& destination_square,
                                         Move_Generation_List&  output,
                                         Moves_Bitboard_Matrix& matrix_output)
{
    const auto who_is_on_destination_square =
        m_chess_board.what_piece_is_on_square(destination_square);

    // Pawn to knight promotion
    Chess_Move move = {
        .source_square      = (ESQUARE) source_square.get_index(),
        .destination_square = (ESQUARE) destination_square.get_index(),
        .moving_piece       = PIECES::PAWN,
        .promoted_piece     = PIECES::KNIGHT,
        .captured_piece     = who_is_on_destination_square.second,
        .is_capture = (who_is_on_destination_square.second != PIECES::NO_PIECE),
        .is_short_castling                = false,
        .is_long_castling                 = false,
        .castling_rook_source_square      = ESQUARE::NO_SQUARE,
        .castling_rook_destination_square = ESQUARE::NO_SQUARE,
        .is_double_pawn_push              = false,
        .is_en_passant                    = false,
        .en_passant_victim_square         = ESQUARE::NO_SQUARE,
        .is_promotion                     = true};

    matrix_output.set_move(moving_side,
                           PIECES::PAWN,
                           source_square,
                           destination_square);

    if constexpr ((gen_type == MOVE_GENERATION_TYPE::ALL)
                  || (gen_type == MOVE_GENERATION_TYPE::TACTICAL))
    {
        output.append(move);

        // Pawn to bishop promotion.
        move.promoted_piece = PIECES::BISHOP;
        output.append(move);

        // Pawn to rook promotion.
        move.promoted_piece = PIECES::ROOK;
        output.append(move);

        // Pawn to queen promotion.
        move.promoted_piece = PIECES::QUEEN;
        output.append(move);
    }
}

template <MOVE_GENERATION_TYPE gen_type, PIECE_COLOR moving_side>
inline void Move_Generator::generate_single_push_promotion_pawn_moves(
    const Bitboard&        pinned,
    const Bitboard&        check_mask,
    Move_Generation_List&  output,
    Moves_Bitboard_Matrix& matrix_output)
{
    const Bitboard pawns =
        m_chess_board.get_piece_occupancies(moving_side, PIECES::PAWN);

    const Bitboard single_pawn_pushes =
        (moving_side == PIECE_COLOR::WHITE)
            ? (pawns >> 8) & (~m_chess_board.get_both_color_occupancies())
            : (pawns << 8) & (~m_chess_board.get_both_color_occupancies());

    const Bitboard promotion_single_pushes =
        (moving_side == PIECE_COLOR::WHITE) ? (single_pawn_pushes & EIGHTH_RANK)
                                            : (single_pawn_pushes & FIRST_RANK);

    for (const Square& destination_square : promotion_single_pushes)
    {
        const Square source_square((moving_side == PIECE_COLOR::WHITE)
                                       ? (destination_square.get_index() + 8)
                                       : (destination_square.get_index() - 8));

        const Bitboard pin_mask =
            get_pin_mask<moving_side>(pinned, source_square);

        const Bitboard pawn_moves =
            Bitboard(destination_square.get_mask()) & check_mask & pin_mask;

        if (pawn_moves != 0)
        {
            generate_pawn_promotions<gen_type, moving_side>(source_square,
                                                            destination_square,
                                                            output,
                                                            matrix_output);
        }
    }
}

template <MOVE_GENERATION_TYPE gen_type, PIECE_COLOR moving_side>
inline void Move_Generator::generate_single_push_non_promotion_pawn_moves(
    const Bitboard&        pinned,
    const Bitboard&        check_mask,
    Move_Generation_List&  output,
    Moves_Bitboard_Matrix& matrix_output)
{
    const Bitboard pawns =
        m_chess_board.get_piece_occupancies(moving_side, PIECES::PAWN);

    const Bitboard single_pawn_pushes =
        (moving_side == PIECE_COLOR::WHITE)
            ? (pawns >> 8) & (~m_chess_board.get_both_color_occupancies())
            : (pawns << 8) & (~m_chess_board.get_both_color_occupancies());

    const Bitboard promotion_single_pushes =
        (moving_side == PIECE_COLOR::WHITE) ? (single_pawn_pushes & EIGHTH_RANK)
                                            : (single_pawn_pushes & FIRST_RANK);

    const Bitboard non_promotion_single_pushes =
        single_pawn_pushes & (~promotion_single_pushes);

    for (const Square& destination_square : non_promotion_single_pushes)
    {
        const Square source_square((moving_side == PIECE_COLOR::WHITE)
                                       ? (destination_square.get_index() + 8)
                                       : (destination_square.get_index() - 8));

        const Bitboard pin_mask =
            get_pin_mask<moving_side>(pinned, source_square);

        const Bitboard pawn_moves =
            Bitboard(destination_square.get_mask()) & check_mask & pin_mask;

        if (pawn_moves != 0)
        {
            const Chess_Move move = {
                .source_square      = (ESQUARE) source_square.get_index(),
                .destination_square = (ESQUARE) destination_square.get_index(),
                .moving_piece       = PIECES::PAWN,
                .promoted_piece     = PIECES::NO_PIECE,
                .captured_piece     = PIECES::NO_PIECE,
                .is_capture         = false,
                .is_short_castling  = false,
                .is_long_castling   = false,
                .castling_rook_source_square      = ESQUARE::NO_SQUARE,
                .castling_rook_destination_square = ESQUARE::NO_SQUARE,
                .is_double_pawn_push              = false,
                .is_en_passant                    = false,
                .en_passant_victim_square         = ESQUARE::NO_SQUARE,
                .is_promotion                     = false};

            matrix_output.set_move(moving_side,
                                   PIECES::PAWN,
                                   source_square,
                                   destination_square);

            if constexpr ((gen_type == MOVE_GENERATION_TYPE::ALL)
                          || (gen_type == MOVE_GENERATION_TYPE::QUIET))
            {
                output.append(move);
            }
        }
    }
}

template <MOVE_GENERATION_TYPE gen_type, PIECE_COLOR moving_side>
inline void Move_Generator::generate_double_push_pawn_moves(
    const Bitboard&        pinned,
    const Bitboard&        check_mask,
    Move_Generation_List&  output,
    Moves_Bitboard_Matrix& matrix_output)
{
    const Bitboard pawns =
        m_chess_board.get_piece_occupancies(moving_side, PIECES::PAWN);

    const Bitboard not_occupied = (~m_chess_board.get_both_color_occupancies());

    const Bitboard double_pawn_pushes =
        (moving_side == PIECE_COLOR::WHITE)
            ? (((((pawns & SECOND_RANK) >> 8) & not_occupied) >> 8)
               & not_occupied)
            : (((((pawns & SEVENTH_RANK) << 8) & not_occupied) << 8)
               & not_occupied);

    for (const Square& destination_square : double_pawn_pushes)
    {
        const Square source_square((moving_side == PIECE_COLOR::WHITE)
                                       ? (destination_square.get_index() + 16)
                                       : (destination_square.get_index() - 16));

        const Bitboard pin_mask =
            get_pin_mask<moving_side>(pinned, source_square);

        const Bitboard pawn_moves =
            Bitboard(destination_square.get_mask()) & check_mask & pin_mask;

        if (pawn_moves != 0)
        {
            const Chess_Move move = {
                .source_square      = (ESQUARE) source_square.get_index(),
                .destination_square = (ESQUARE) destination_square.get_index(),
                .moving_piece       = PIECES::PAWN,
                .promoted_piece     = PIECES::NO_PIECE,
                .captured_piece     = PIECES::NO_PIECE,
                .is_capture         = false,
                .is_short_castling  = false,
                .is_long_castling   = false,
                .castling_rook_source_square      = ESQUARE::NO_SQUARE,
                .castling_rook_destination_square = ESQUARE::NO_SQUARE,
                .is_double_pawn_push              = true,
                .is_en_passant                    = false,
                .en_passant_victim_square         = ESQUARE::NO_SQUARE,
                .is_promotion                     = false};

            matrix_output.set_move(moving_side,
                                   PIECES::PAWN,
                                   source_square,
                                   destination_square);

            if constexpr ((gen_type == MOVE_GENERATION_TYPE::ALL)
                          || (gen_type == MOVE_GENERATION_TYPE::QUIET))
            {
                output.append(move);
            }
        }
    }
}

template <MOVE_GENERATION_TYPE gen_type, PIECE_COLOR moving_side>
inline void Move_Generator::generate_en_passant_captures(
    const Bitboard&        pinned,
    const Bitboard&        check_mask,
    Move_Generation_List&  output,
    Moves_Bitboard_Matrix& matrix_output)
{
    Attacks a;

    constexpr PIECE_COLOR opposing_side = ~moving_side;

    const Square king_square = m_chess_board.get_king_square(moving_side);

    Square en_passant_square = m_chess_board.get_en_passant_square();
    if (en_passant_square.has_square())
    {
        const Bitboard en_passant_pawns =
            a.get_pawn_attacks(en_passant_square, opposing_side)
            & m_chess_board.get_piece_occupancies(moving_side, PIECES::PAWN);

        for (const Square& source_square : en_passant_pawns)
        {
            const Square en_passant_victim_square =
                m_chess_board.get_en_passant_victim_square();

            const Bitboard pin_mask =
                get_pin_mask<moving_side>(pinned, source_square);

            // Special case where the checker can be removed via en passant.
            const Bitboard can_remove_checker =
                m_chess_board.get_en_passant_square().get_mask()
                & ((uint64_t) (-(uint64_t) (m_enpassantable_checker == true)));

            const Bitboard can_do_enpassant =
                Bitboard(en_passant_square.get_mask())
                & (check_mask | can_remove_checker) & pin_mask;

            // Takes care of the situation where an enemy rook or queen is on
            // the same rank as a friendly enpassant alongside the friendly
            // king. En passant is not possible because the friendly king would
            // be in check after the move.
            const Bitboard en_passant_rank_mask =
                Bitboard::get_rank_mask(source_square);
            const Bitboard occupancy_without_en_passant_pawns =
                m_chess_board.get_both_color_occupancies()
                ^ en_passant_victim_square.get_mask()
                ^ source_square.get_mask();
            const Bitboard enemy_orthogonal_movers =
                m_chess_board.get_piece_occupancies(opposing_side, PIECES::ROOK)
                | m_chess_board.get_piece_occupancies(opposing_side,
                                                      PIECES::QUEEN);
            const Bitboard ortogonal_attacks_to_king_without_en_passant_pawns =
                (a.get_rook_attacks(king_square,
                                    occupancy_without_en_passant_pawns)
                 & en_passant_rank_mask)
                & enemy_orthogonal_movers;

            if ((can_do_enpassant != 0)
                && (ortogonal_attacks_to_king_without_en_passant_pawns == 0))
            {
                const Chess_Move move = {
                    .source_square = (ESQUARE) source_square.get_index(),
                    .destination_square =
                        (ESQUARE) en_passant_square.get_index(),
                    .moving_piece   = PIECES::PAWN,
                    .promoted_piece = PIECES::NO_PIECE,
                    .captured_piece =
                        PIECES::NO_PIECE, // Yes we capture a PIECES::PAWN but
                                          // this becomes a hinderance when
                                          // writing the calculate next board
                                          // state function.
                    .is_capture =
                        false, // Same hindereance here as explained above.
                    .is_short_castling                = false,
                    .is_long_castling                 = false,
                    .castling_rook_source_square      = ESQUARE::NO_SQUARE,
                    .castling_rook_destination_square = ESQUARE::NO_SQUARE,
                    .is_double_pawn_push              = false,
                    .is_en_passant                    = true,
                    .en_passant_victim_square =
                        (ESQUARE) en_passant_victim_square.get_index(),
                    .is_promotion = false};

                matrix_output.set_move(moving_side,
                                       PIECES::PAWN,
                                       source_square,
                                       en_passant_square);

                if constexpr ((gen_type == MOVE_GENERATION_TYPE::ALL)
                              || (gen_type == MOVE_GENERATION_TYPE::TACTICAL))
                {
                    output.append(move);
                }
            }
        }
    }
}

template <MOVE_GENERATION_TYPE gen_type, PIECE_COLOR moving_side>
inline void Move_Generator::generate_non_promotion_pawn_captures(
    const Bitboard&        pinned,
    const Bitboard&        check_mask,
    Move_Generation_List&  output,
    Moves_Bitboard_Matrix& matrix_output)
{
    Attacks a;

    constexpr PIECE_COLOR opposing_side = ~moving_side;

    const Bitboard pawns =
        m_chess_board.get_piece_occupancies(moving_side, PIECES::PAWN);

    for (const Square& source_square : pawns)
    {
        const Bitboard pin_mask =
            get_pin_mask<moving_side>(pinned, source_square);

        const Bitboard pawn_attacks =
            a.get_pawn_attacks(source_square, moving_side)
            & m_chess_board.get_color_occupancies(opposing_side) & check_mask
            & pin_mask;

        const Bitboard promotable_pawn_attacks =
            (moving_side == PIECE_COLOR::WHITE) ? pawn_attacks & EIGHTH_RANK
                                                : pawn_attacks & FIRST_RANK;
        const Bitboard non_promotable_pawn_attacks =
            pawn_attacks & (~promotable_pawn_attacks);

        for (const Square& destination_square : non_promotable_pawn_attacks)
        {
            const auto who_is_on_destination_square =
                m_chess_board.what_piece_is_on_square(destination_square);

            const Chess_Move move = {
                .source_square      = (ESQUARE) source_square.get_index(),
                .destination_square = (ESQUARE) destination_square.get_index(),
                .moving_piece       = PIECES::PAWN,
                .promoted_piece     = PIECES::NO_PIECE,
                .captured_piece     = who_is_on_destination_square.second,
                .is_capture         = true,
                .is_short_castling  = false,
                .is_long_castling   = false,
                .castling_rook_source_square      = ESQUARE::NO_SQUARE,
                .castling_rook_destination_square = ESQUARE::NO_SQUARE,
                .is_double_pawn_push              = false,
                .is_en_passant                    = false,
                .en_passant_victim_square         = ESQUARE::NO_SQUARE,
                .is_promotion                     = false};

            matrix_output.set_move(moving_side,
                                   PIECES::PAWN,
                                   source_square,
                                   destination_square);

            if constexpr ((gen_type == MOVE_GENERATION_TYPE::ALL)
                          || (gen_type == MOVE_GENERATION_TYPE::TACTICAL))
            {
                output.append(move);
            }
        }
    }
}

template <MOVE_GENERATION_TYPE gen_type, PIECE_COLOR moving_side>
inline void Move_Generator::generate_promotion_pawn_captures(
    const Bitboard&        pinned,
    const Bitboard&        check_mask,
    Move_Generation_List&  output,
    Moves_Bitboard_Matrix& matrix_output)
{
    Attacks a;

    constexpr PIECE_COLOR opposing_side = ~moving_side;

    const Bitboard pawns =
        m_chess_board.get_piece_occupancies(moving_side, PIECES::PAWN);

    for (const Square& source_square : pawns)
    {
        const Bitboard pin_mask =
            get_pin_mask<moving_side>(pinned, source_square);

        const Bitboard pawn_attacks =
            a.get_pawn_attacks(source_square, moving_side)
            & m_chess_board.get_color_occupancies(opposing_side) & check_mask
            & pin_mask;

        const Bitboard promotable_pawn_attacks =
            (moving_side == PIECE_COLOR::WHITE) ? pawn_attacks & EIGHTH_RANK
                                                : pawn_attacks & FIRST_RANK;

        for (const Square& destination_square : promotable_pawn_attacks)
        {
            generate_pawn_promotions<gen_type, moving_side>(source_square,
                                                            destination_square,
                                                            output,
                                                            matrix_output);
        }
    }
}

template <MOVE_GENERATION_TYPE gen_type,
          PIECE_COLOR          moving_side,
          PIECES               moving_piece>
inline void Move_Generator::generate_minor_and_major_piece_moves(
    const Bitboard&        pinned,
    const Bitboard&        check_mask,
    Move_Generation_List&  output,
    Moves_Bitboard_Matrix& matrix_output)
{
    Attacks a;

    constexpr PIECE_COLOR opposing_side = ~moving_side;

    const Bitboard both_color_occupancies =
        m_chess_board.get_both_color_occupancies();

    Bitboard enemy_or_empty = m_chess_board.get_color_occupancies(opposing_side)
                            | (~(both_color_occupancies));

    const Bitboard piece_occupancies =
        m_chess_board.get_piece_occupancies(moving_side, moving_piece);

    for (const Square& source_square : piece_occupancies)
    {
        const Bitboard pin_mask =
            get_pin_mask<moving_side>(pinned, source_square);

        const uint64_t piece_attacks =
            ((moving_piece == PIECES::KNIGHT)
             * a.get_knight_attacks(source_square).get_board())
            + ((moving_piece == PIECES::BISHOP)
               * a.get_bishop_attacks(source_square, both_color_occupancies)
                     .get_board())
            + ((moving_piece == PIECES::ROOK)
               * a.get_rook_attacks(source_square, both_color_occupancies)
                     .get_board())
            + ((moving_piece == PIECES::QUEEN)
               * a.get_queen_attacks(source_square, both_color_occupancies)
                     .get_board());

        const Bitboard piece_moves =
            Bitboard(piece_attacks) & enemy_or_empty & check_mask & pin_mask;

        for (const Square& destination_square : piece_moves)
        {
            const auto who_is_on_destination_square =
                m_chess_board.what_piece_is_on_square(destination_square);

            const Chess_Move move = {
                .source_square = (ESQUARE) (source_square.get_index()),
                .destination_square =
                    (ESQUARE) (destination_square.get_index()),
                .moving_piece   = moving_piece,
                .promoted_piece = PIECES::NO_PIECE,
                .captured_piece = who_is_on_destination_square.second,
                .is_capture =
                    (who_is_on_destination_square.first == opposing_side),
                .is_short_castling                = false,
                .is_long_castling                 = false,
                .castling_rook_source_square      = ESQUARE::NO_SQUARE,
                .castling_rook_destination_square = ESQUARE::NO_SQUARE,
                .is_double_pawn_push              = false,
                .is_en_passant                    = false,
                .en_passant_victim_square         = ESQUARE::NO_SQUARE,
                .is_promotion                     = false};

            matrix_output.set_move(moving_side,
                                   moving_piece,
                                   source_square,
                                   destination_square);

            if constexpr (gen_type == MOVE_GENERATION_TYPE::TACTICAL)
            {
                if (m_chess_board.get_color_occupancies(opposing_side)
                        .get_board()
                    & destination_square.get_mask())
                {
                    output.append(move);
                }
            }

            if constexpr (gen_type == MOVE_GENERATION_TYPE::QUIET)
            {
                if ((~(both_color_occupancies)).get_board()
                    & destination_square.get_mask())
                {
                    output.append(move);
                }
            }

            if constexpr (gen_type == MOVE_GENERATION_TYPE::ALL)
            {
                output.append(move);
            }
        }
    }
}

template <MOVE_GENERATION_TYPE gen_type, PIECE_COLOR moving_side>
inline void
Move_Generator::generate_king_moves(Move_Generation_List&  output,
                                    Moves_Bitboard_Matrix& matrix_output)
{
    Attacks a;

    constexpr PIECE_COLOR opposing_side = ~moving_side;

    const Bitboard both_color_occupancies =
        m_chess_board.get_both_color_occupancies();

    Bitboard enemy_or_empty = m_chess_board.get_color_occupancies(opposing_side)
                            | (~(both_color_occupancies));

    const Square king_square = m_chess_board.get_king_square(moving_side);

    const Bitboard king_moves = a.get_king_attacks(king_square) & enemy_or_empty
                              & (~is_our_king_ring_attacked<moving_side>());

    for (const Square& destination_square : king_moves)
    {
        const auto who_is_on_destination_square =
            m_chess_board.what_piece_is_on_square(destination_square);

        Chess_Move move = {
            .source_square      = (ESQUARE) king_square.get_index(),
            .destination_square = (ESQUARE) destination_square.get_index(),
            .moving_piece       = PIECES::KING,
            .promoted_piece     = PIECES::NO_PIECE,
            .captured_piece     = who_is_on_destination_square.second,
            .is_capture = (who_is_on_destination_square.first == opposing_side),
            .is_short_castling                = false,
            .is_long_castling                 = false,
            .castling_rook_source_square      = ESQUARE::NO_SQUARE,
            .castling_rook_destination_square = ESQUARE::NO_SQUARE,
            .is_double_pawn_push              = false,
            .is_en_passant                    = false,
            .en_passant_victim_square         = ESQUARE::NO_SQUARE,
            .is_promotion                     = false};

        matrix_output.set_move(moving_side,
                               PIECES::KING,
                               king_square,
                               destination_square);

        if constexpr (gen_type == MOVE_GENERATION_TYPE::TACTICAL)
        {
            if (m_chess_board.get_color_occupancies(opposing_side).get_board()
                & destination_square.get_mask())
            {
                output.append(move);
            }
        }

        if constexpr (gen_type == MOVE_GENERATION_TYPE::QUIET)
        {
            if ((~(both_color_occupancies)).get_board()
                & destination_square.get_mask())
            {
                output.append(move);
            }
        }

        if constexpr (gen_type == MOVE_GENERATION_TYPE::ALL)
        {
            output.append(move);
        }
    }
}

// A generalized function to generate legal castling moves in both standard
// chess and Fischer Random chess.
template <MOVE_GENERATION_TYPE gen_type,
          PIECE_COLOR          moving_side,
          CASTLING_TYPE        castle_type>
inline void
Move_Generator::generate_castling_moves(const Bitboard&        pinned,
                                        Move_Generation_List&  output,
                                        Moves_Bitboard_Matrix& matrix_output)
{
    bool has_castling_rights;

    if constexpr ((moving_side == PIECE_COLOR::WHITE)
                  && (castle_type == CASTLING_TYPE::KINGSIDE))
    {
        has_castling_rights =
            m_chess_board.does_white_have_short_castle_rights();
    }
    else if constexpr ((moving_side == PIECE_COLOR::WHITE)
                       && (castle_type == CASTLING_TYPE::QUEENSIDE))
    {
        has_castling_rights =
            m_chess_board.does_white_have_long_castle_rights();
    }
    else if constexpr ((moving_side == PIECE_COLOR::BLACK)
                       && (castle_type == CASTLING_TYPE::KINGSIDE))
    {
        has_castling_rights =
            m_chess_board.does_black_have_short_castle_rights();
    }
    else if constexpr ((moving_side == PIECE_COLOR::BLACK)
                       && (castle_type == CASTLING_TYPE::QUEENSIDE))
    {
        has_castling_rights =
            m_chess_board.does_black_have_long_castle_rights();
    }

    if (has_castling_rights)
    {
        constexpr PIECE_COLOR opposing_side =
            ~moving_side;

        const Bitboard enemy_occupancies =
            m_chess_board.get_color_occupancies(opposing_side);

        const Bitboard both_occupancies =
            m_chess_board.get_both_color_occupancies();

        const Square king_source_square =
            m_chess_board.get_king_square(moving_side);

        const Square rook_source_square =
            m_chess_board.get_castling_rook_source_square(moving_side,
                                                          castle_type);

        // Are squares between the moving side's king's source square to
        // destination square (excluding source square, including destination
        // square, excluding the rook's (the rook being castled with) square)
        // empty? Also, if the destination square is the same as the source
        // square ensure this is counted as an empty square thus, we use the
        // following XOR trick: (both_occupancies ^
        // king_source_square.get_mask()).
        const Square king_destination_square =
            CASTLING_KING_DESTINATION_SQUARES[moving_side][castle_type];

        const Bitboard king_should_be_empty_squares =
            ((Bitboard::get_between_squares_mask(king_source_square,
                                                 king_destination_square)
              | king_destination_square.get_mask())
             & (~rook_source_square.get_mask()));

        bool is_king_squares_empty =
            ((king_should_be_empty_squares
              & (both_occupancies ^ king_source_square.get_mask()))
             == 0);

        // Are squares between the moving side's rook's (the rook being castled
        // with) source square to destination square (excluding source square,
        // including destination square, excluding the moving side's king's
        // square) empty? Just like with the king, ensure that if the
        // destination square and source square are the same then it counts an
        // empty square, a similar XOR trick is used here. Additionally, in
        // Fischer Random Chess it is possible to pin a castling rook to the
        // friendly king in this case, castling is not allowed.
        const Square rook_destination_square =
            CASTLING_ROOK_DESTINATION_SQUARES[moving_side][castle_type];

        const Bitboard rook_should_be_empty_squares =
            ((Bitboard::get_between_squares_mask(rook_source_square,
                                                 rook_destination_square)
              | rook_destination_square.get_mask())
             & (~king_source_square.get_mask()));

        bool is_rook_squares_empty =
            ((rook_should_be_empty_squares
              & (both_occupancies ^ rook_source_square.get_mask()))
             == 0);

        bool is_rook_pinned = is_pinned(pinned, rook_source_square);

        // Is the path from the king's source square to destination square
        // (including source & destination square) not attacked?
        const Bitboard king_should_be_not_attacked_squares =
            Bitboard::get_between_squares_mask(king_source_square,
                                               king_destination_square)
            | king_source_square.get_mask()
            | king_destination_square.get_mask();

        bool is_king_safe = true;
        for (const Square& s : king_should_be_not_attacked_squares)
        {
            const bool is_square_safe =
                ((attackers_to_square(s) & enemy_occupancies).get_board() == 0);
            is_king_safe = (is_king_safe && is_square_safe);
        }

        if (is_king_squares_empty && is_rook_squares_empty && is_king_safe
            && (!is_rook_pinned))
        {
            const Chess_Move move = {
                .source_square = (ESQUARE) king_source_square.get_index(),
                .destination_square =
                    (ESQUARE) king_destination_square.get_index(),
                .moving_piece      = PIECES::KING,
                .promoted_piece    = PIECES::NO_PIECE,
                .captured_piece    = PIECES::NO_PIECE,
                .is_capture        = false,
                .is_short_castling = (castle_type == CASTLING_TYPE::KINGSIDE),
                .is_long_castling  = (castle_type == CASTLING_TYPE::QUEENSIDE),
                .castling_rook_source_square =
                    (ESQUARE) rook_source_square.get_index(),
                .castling_rook_destination_square =
                    (ESQUARE) rook_destination_square.get_index(),
                .is_double_pawn_push      = false,
                .is_en_passant            = false,
                .en_passant_victim_square = ESQUARE::NO_SQUARE,
                .is_promotion             = false};

            matrix_output.set_move(moving_side,
                                   PIECES::KING,
                                   king_source_square,
                                   king_destination_square);
            matrix_output.set_move(moving_side,
                                   PIECES::ROOK,
                                   rook_source_square,
                                   rook_destination_square);
            if constexpr ((gen_type == MOVE_GENERATION_TYPE::ALL)
                          || (gen_type == MOVE_GENERATION_TYPE::QUIET))
            {
                output.append(move);
            }
        }
    }
}

template <MOVE_GENERATION_TYPE gen_type>
void Move_Generator::generate_all_moves(Move_Generation_List&  output,
                                        Moves_Bitboard_Matrix& matrix_output)
{
    const PIECE_COLOR side = m_chess_board.get_side_to_move();
    generate_all_moves<gen_type>(side, output, matrix_output);
}

template <MOVE_GENERATION_TYPE gen_type>
void Move_Generator::generate_all_moves(PIECE_COLOR            side,
                                        Move_Generation_List&  output,
                                        Moves_Bitboard_Matrix& matrix_output)
{
    if (side == PIECE_COLOR::WHITE)
    {
        const Bitboard check_mask = generate_check_mask<PIECE_COLOR::WHITE>();
        const Bitboard pinned     = generate_pinned<PIECE_COLOR::WHITE>();

        /***************************************************************************
         * TACTICAL MOVE GENERATION
         ***************************************************************************/
        generate_single_push_promotion_pawn_moves<gen_type, PIECE_COLOR::WHITE>(
            pinned,
            check_mask,
            output,
            matrix_output);
        generate_en_passant_captures<gen_type, PIECE_COLOR::WHITE>(
            pinned,
            check_mask,
            output,
            matrix_output);
        generate_non_promotion_pawn_captures<gen_type, PIECE_COLOR::WHITE>(
            pinned,
            check_mask,
            output,
            matrix_output);
        generate_promotion_pawn_captures<gen_type, PIECE_COLOR::WHITE>(
            pinned,
            check_mask,
            output,
            matrix_output);

        /***************************************************************************
         * QUIET MOVE GENERATION
         ***************************************************************************/
        generate_single_push_non_promotion_pawn_moves<gen_type,
                                                      PIECE_COLOR::WHITE>(
            pinned,
            check_mask,
            output,
            matrix_output);
        generate_double_push_pawn_moves<gen_type, PIECE_COLOR::WHITE>(
            pinned,
            check_mask,
            output,
            matrix_output);
        generate_castling_moves<gen_type,
                                PIECE_COLOR::WHITE,
                                CASTLING_TYPE::KINGSIDE>(pinned,
                                                         output,
                                                         matrix_output);
        generate_castling_moves<gen_type,
                                PIECE_COLOR::WHITE,
                                CASTLING_TYPE::QUEENSIDE>(pinned,
                                                          output,
                                                          matrix_output);

        /***************************************************************************
         * GENERAL MOVE GENERATION
         ***************************************************************************/
        generate_minor_and_major_piece_moves<gen_type,
                                             PIECE_COLOR::WHITE,
                                             PIECES::KNIGHT>(pinned,
                                                             check_mask,
                                                             output,
                                                             matrix_output);
        generate_minor_and_major_piece_moves<gen_type,
                                             PIECE_COLOR::WHITE,
                                             PIECES::BISHOP>(pinned,
                                                             check_mask,
                                                             output,
                                                             matrix_output);
        generate_minor_and_major_piece_moves<gen_type,
                                             PIECE_COLOR::WHITE,
                                             PIECES::ROOK>(pinned,
                                                           check_mask,
                                                           output,
                                                           matrix_output);
        generate_minor_and_major_piece_moves<gen_type,
                                             PIECE_COLOR::WHITE,
                                             PIECES::QUEEN>(pinned,
                                                            check_mask,
                                                            output,
                                                            matrix_output);
        generate_king_moves<gen_type, PIECE_COLOR::WHITE>(output,
                                                          matrix_output);
    }
    else
    {
        const Bitboard check_mask = generate_check_mask<PIECE_COLOR::BLACK>();
        const Bitboard pinned     = generate_pinned<PIECE_COLOR::BLACK>();

        /***************************************************************************
         * TACTICAL MOVE GENERATION
         ***************************************************************************/
        generate_single_push_promotion_pawn_moves<gen_type, PIECE_COLOR::BLACK>(
            pinned,
            check_mask,
            output,
            matrix_output);
        generate_en_passant_captures<gen_type, PIECE_COLOR::BLACK>(
            pinned,
            check_mask,
            output,
            matrix_output);
        generate_non_promotion_pawn_captures<gen_type, PIECE_COLOR::BLACK>(
            pinned,
            check_mask,
            output,
            matrix_output);
        generate_promotion_pawn_captures<gen_type, PIECE_COLOR::BLACK>(
            pinned,
            check_mask,
            output,
            matrix_output);

        /***************************************************************************
         * QUIET MOVE GENERATION
         ***************************************************************************/
        generate_single_push_non_promotion_pawn_moves<gen_type,
                                                      PIECE_COLOR::BLACK>(
            pinned,
            check_mask,
            output,
            matrix_output);
        generate_double_push_pawn_moves<gen_type, PIECE_COLOR::BLACK>(
            pinned,
            check_mask,
            output,
            matrix_output);
        generate_castling_moves<gen_type,
                                PIECE_COLOR::BLACK,
                                CASTLING_TYPE::KINGSIDE>(pinned,
                                                         output,
                                                         matrix_output);
        generate_castling_moves<gen_type,
                                PIECE_COLOR::BLACK,
                                CASTLING_TYPE::QUEENSIDE>(pinned,
                                                          output,
                                                          matrix_output);

        /***************************************************************************
         * GENERAL MOVE GENERATION
         ***************************************************************************/
        generate_minor_and_major_piece_moves<gen_type,
                                             PIECE_COLOR::BLACK,
                                             PIECES::KNIGHT>(pinned,
                                                             check_mask,
                                                             output,
                                                             matrix_output);
        generate_minor_and_major_piece_moves<gen_type,
                                             PIECE_COLOR::BLACK,
                                             PIECES::BISHOP>(pinned,
                                                             check_mask,
                                                             output,
                                                             matrix_output);
        generate_minor_and_major_piece_moves<gen_type,
                                             PIECE_COLOR::BLACK,
                                             PIECES::ROOK>(pinned,
                                                           check_mask,
                                                           output,
                                                           matrix_output);
        generate_minor_and_major_piece_moves<gen_type,
                                             PIECE_COLOR::BLACK,
                                             PIECES::QUEEN>(pinned,
                                                            check_mask,
                                                            output,
                                                            matrix_output);
        generate_king_moves<gen_type, PIECE_COLOR::BLACK>(output,
                                                          matrix_output);
    }
}
