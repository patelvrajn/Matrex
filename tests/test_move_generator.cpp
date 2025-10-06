#include <fstream>

#include "gtest/gtest.h"
#include "move_generator.hpp"
#include "perft.hpp"
#include "timer.hpp"

TEST(move_generator_tests, perft_test_suite) {
  // Read perft suite into std::string memory
  std::ifstream input_file_stream("assets/perft_suite.epd");
  std::stringstream buffer;
  buffer << input_file_stream.rdbuf();
  std::string perft_suite_str = buffer.str();

  std::istringstream iss(perft_suite_str);
  std::string line;
  while (std::getline(iss, line)) {
    std::cout << "============================================================="
                 "==================="
              << std::endl;

    const size_t first_semicolon_position = line.find_first_of(";");
    const size_t end_of_fen_position = first_semicolon_position - 2;
    std::string fen = line.substr(0, (end_of_fen_position + 1));
    std::cout << "FEN: " << fen << std::endl;
    std::cout << "-------------------------------------------------------------"
                 "-------------------"
              << std::endl;

    Chess_Board cb;
    cb.set_from_fen(fen);

    std::size_t start_of_perft_string =
        line.find_first_of(";", first_semicolon_position);

    while (start_of_perft_string != std::string::npos) {
      const std::size_t next_semicolon =
          line.find_first_of(";", (start_of_perft_string + 1));

      const std::string perft_string =
          line.substr((start_of_perft_string + 1),
                      ((next_semicolon - start_of_perft_string) - 1));
      const size_t space_delimiter_position = perft_string.find_first_of(" ");
      const std::string depth_string =
          perft_string.substr(1, (space_delimiter_position - 1));
      const std::string leaf_node_count_string = perft_string.substr(
          (space_delimiter_position + 1),
          ((perft_string.length() - 1) - (space_delimiter_position + 1)));

      std::cout << "PERFT STRING: " << "(" << depth_string << ","
                << leaf_node_count_string << ")" << std::endl;

      const uint64_t perft_depth = std::stoull(depth_string);
      const uint64_t perft_leaf_node_count =
          std::stoull(leaf_node_count_string);

      Timer t;
      const uint64_t perft_count = perft(cb, perft_depth);
      const uint64_t time_taken = t.stop();

      EXPECT_EQ(perft_count, perft_leaf_node_count);

      std::cout << "Perft(" << perft_depth << "): " << perft_count << std::endl;
      std::cout << "Time taken (ns): " << time_taken << std::endl;
      std::cout << "NPS: "
                << (((double)perft_count) / ((double)time_taken / 1e9))
                << std::endl;
      std::cout << "-----------------------------------------------------------"
                   "---------------------"
                << std::endl;

      start_of_perft_string = next_semicolon;
    }
    std::cout << "============================================================="
                 "==================="
              << std::endl;
  }
}
