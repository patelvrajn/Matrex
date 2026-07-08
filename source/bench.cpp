#include "bench.hpp"
#include "search.hpp"
#include "timer.hpp"
#include "perft.hpp"

double Performance_Bench::bench_move_generation(uint16_t depth)
{
    uint64_t total_leaf_node_count = 0;
    uint64_t total_time            = 0;

    Timer t;
    for (const auto& fen : M_BENCH_FENS)
    {
        Chess_Board board;
        board.set_from_fen(std::string(fen));

        t.start();
        total_leaf_node_count += perft(board, depth);
        total_time            += t.elapsed();
    }

    double nps = static_cast<double>(total_leaf_node_count)
               / (static_cast<double>(total_time) / 1.0e9);

    std::cout << "=== MOVE GENERATION BENCH FOR DEPTH " << depth
              << " ===" << std::endl;
    std::cout << "Total leaf node count is " << total_leaf_node_count
              << std::endl;
    std::cout << "Total time taken (in ns) is " << total_time << std::endl;
    std::cout << "NPS: " << nps << std::endl;

    return nps;
}

double Performance_Bench::bench_search(uint16_t depth)
{
    Search_Engine se;

    // Constraints for a depth search.
    Search_Constraints constraints;
    constraints.should_ignore_time = true;
    constraints.depth              = depth;

    uint64_t total_node_count = 0;
    uint64_t total_time       = 0;

    Timer t;
    for (const auto& fen : M_BENCH_FENS)
    {
        Chess_Board board;
        board.set_from_fen(std::string(fen));

        t.start();
        se.search(board, constraints);

        total_time       += t.elapsed();
        total_node_count += se.get_node_count();
    }

    double nps = static_cast<double>(total_node_count)
               / (static_cast<double>(total_time) / 1.0e9);

    std::cout << "=== SEARCH BENCH FOR DEPTH " << depth << " ===" << std::endl;
    std::cout << "Total node count is " << total_node_count << std::endl;
    std::cout << "Total time taken (in ns) is " << total_time << std::endl;
    std::cout << "NPS: " << nps << std::endl;

    return nps;
}
