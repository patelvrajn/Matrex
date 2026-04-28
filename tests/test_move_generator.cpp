#include "gtest/gtest.h"
#include "move_generator.hpp"
#include "tests/epd.hpp"

TEST(move_generator_tests, moves_bitboard_test)
{
    Chess_Board cb;
    cb.set_from_fen("1brqbnn1/PPPPPPPP/2kNN3/R7/6K1/3BB3/1Q6/7R w - - 0 1");

    const PIECE_COLOR     moving_side = cb.get_side_to_move();
    Chess_Move_List       moving_side_moves_list;
    Moves_Bitboard_Matrix moving_side_matrix;
    Move_Generator        mg_moving_side(cb);
    mg_moving_side.generate_all_moves<MOVE_GENERATION_TYPE::ALL>(
        moving_side,
        moving_side_moves_list,
        moving_side_matrix);

    for (const Chess_Move& move : moving_side_moves_list)
    {
        Moves_Bitboard mb;
        const bool     move_exists_in_matrix =
            moving_side_matrix.get_moves_bitboards(moving_side,
                                                   move.moving_piece,
                                                   Square(move.source_square),
                                                   mb);
        EXPECT_TRUE(move_exists_in_matrix);
        EXPECT_EQ(mb.piece, move.moving_piece);
        EXPECT_EQ(mb.square, Square(move.source_square));
        EXPECT_EQ(mb.bitboard.get_square(Square(move.destination_square)),
                  Square(move.destination_square).get_mask());
    }
}

TEST(move_generator_tests, standard_perft_test_suite)
{
    epd_perft_test("assets/standard_perft_suite.epd");
}

TEST(move_generator_tests, DISABLED_frc_perft_test_suite)
{
    epd_perft_test("assets/frc_perft_suite.epd");
}
