#pragma once

#include <string>

#include "chess_board.hpp"

struct Time_Control {
  uint64_t time_remaining;
  uint64_t increment;
};

class UCI {
 public:
  UCI();

  void loop();

 private:
  Chess_Board m_chess_board;

  bool m_is_frc;

  std::array<Time_Control, NUM_OF_PLAYERS> m_time_controls;

  // UCI commands
  void handle_position(const std::string& arguments);
  void handle_go(const std::string& arguments);
  void handle_quit(const std::string& arguments);
  void handle_uci(const std::string& arguments);
  void handle_ucinewgame(const std::string& arguments);
  void handle_isready(const std::string& arguments);

  // Helpers
  void make_moves_from_string(const std::string& moves_str, bool is_frc);
};
