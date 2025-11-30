#pragma once

#include <string>
#include <vector>

#include "chess_board.hpp"
#include "search.hpp"

class UCI {
 public:
  UCI();

  void loop();

 private:
  Chess_Board m_chess_board;
  Search_Engine m_search_engine;

  bool m_is_frc;

  Search_Constraints m_search_constraints;

  // UCI commands
  void handle_position(const std::string& arguments);
  void handle_go(const std::string& arguments);
  void handle_quit(const std::string& arguments);
  void handle_uci(const std::string& arguments);
  void handle_ucinewgame(const std::string& arguments);
  void handle_isready(const std::string& arguments);
  void handle_setoption(const std::string& arguments);

  // Helpers
  void make_moves_from_string(const std::string& moves_str, bool is_frc);
  std::unique_ptr<std::vector<std::string>> split_string(
      std::string s, const std::string& delimiter);
};
