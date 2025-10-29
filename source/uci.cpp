#include "uci.hpp"

#include <iostream>
#include <sstream>

#include "move_generator.hpp"
#include "search.hpp"

UCI::UCI() : m_is_frc(false) {}

void UCI::loop() {
  std::string line;

  while (std::getline(std::cin, line)) {
    const std::size_t first_space_idx = line.find_first_of(" ");

    std::string command;
    std::string arguments;

    if (first_space_idx == std::string::npos) {
      command = line;
    } else {
      command = line.substr(0, first_space_idx);
      arguments = line.substr((first_space_idx + 1));
    }

#define HANDLE_COMMAND(c)  \
  if (command == #c) {     \
    handle_##c(arguments); \
  }

    HANDLE_COMMAND(position)
    HANDLE_COMMAND(go)
    HANDLE_COMMAND(quit)
    HANDLE_COMMAND(uci)
    HANDLE_COMMAND(ucinewgame)
    HANDLE_COMMAND(isready)

#undef HANDLE_COMMAND
  }
}

void UCI::handle_position(const std::string& arguments) {
  const std::size_t first_space_idx = arguments.find_first_of(" ");
  const std::string subcommand = arguments.substr(0, first_space_idx);

  if (subcommand == "startpos") {
    m_chess_board.set_from_fen(std::string(START_POSITION_FEN));

    const std::size_t end_of_moves_keyword_index =
        arguments.find_first_of(" ", (first_space_idx + 1));

    if (end_of_moves_keyword_index != std::string::npos) {
      const std::string moves_str =
          arguments.substr(end_of_moves_keyword_index + 1);

      make_moves_from_string(moves_str, m_is_frc);
    }
  }

  if (subcommand == "fen") {
    constexpr uint8_t SPACES_IN_A_FEN = 5;

    std::size_t fen_space_index =
        arguments.find_first_of(" ", (first_space_idx + 1));
    uint8_t fen_space_count = 0;
    while ((fen_space_index != std::string::npos) &&
           (fen_space_count != SPACES_IN_A_FEN)) {
      fen_space_index = arguments.find_first_of(" ", (fen_space_index + 1));
      fen_space_count++;
    }

    bool is_end_of_string = false;

    std::string fen;
    if (fen_space_index == std::string::npos) {
      fen = arguments.substr((first_space_idx + 1));
      is_end_of_string = true;
    } else {
      fen = arguments.substr(
          (first_space_idx + 1),
          (((fen_space_index - 1) - (first_space_idx + 1)) + 1));
    }

    m_chess_board.set_from_fen(fen);

    if (!is_end_of_string) {
      const std::size_t end_of_moves_keyword_index =
          arguments.find_first_of(" ", (fen_space_index + 1));

      const std::string moves_str =
          arguments.substr(end_of_moves_keyword_index + 1);

      make_moves_from_string(moves_str, m_is_frc);
    }
  }

  if (subcommand == "print") {  // Not part of specification.
    m_chess_board.pretty_print();
  }
}

void UCI::handle_go(const std::string& arguments) {
  const std::size_t first_space_idx = arguments.find_first_of(" ");
  const std::string subcommand = arguments.substr(0, first_space_idx);

  const std::string subcommand_first_argument_str =
      arguments.substr(first_space_idx + 1);

  if (subcommand == "wtime") {
    m_time_controls[PIECE_COLOR::WHITE].time_remaining =
        std::stoull(subcommand_first_argument_str);
  } else if (subcommand == "btime") {
    m_time_controls[PIECE_COLOR::BLACK].time_remaining =
        std::stoull(subcommand_first_argument_str);
  } else if (subcommand == "winc") {
    m_time_controls[PIECE_COLOR::WHITE].increment =
        std::stoull(subcommand_first_argument_str);
  } else if (subcommand == "binc") {
    m_time_controls[PIECE_COLOR::BLACK].increment =
        std::stoull(subcommand_first_argument_str);
  }

  Search_Engine search(m_chess_board);
  Search_Engine_Result search_result =
      search.negamax(5);  // Fixed depth for now...

  std::cout << "info score cp " << search_result.second.to_int() << std::endl;

  std::cout << "bestmove "
            << search_result.first.to_coordinate_notation(m_is_frc)
            << std::endl;
}

void UCI::handle_quit(const std::string&) { exit(0); }

void UCI::handle_uci(const std::string&) {
  std::cout << "id name " << ENGINE_NAME << " " << ENGINE_VERSION << std::endl;
  std::cout << "uciok" << std::endl;
}

void UCI::handle_ucinewgame(const std::string&) { return; }

void UCI::handle_isready(const std::string&) {
  std::cout << "readyok" << std::endl;
}

void UCI::make_moves_from_string(const std::string& moves_str, bool is_frc) {
  std::istringstream iss(moves_str);

  std::string move_str;

  while (iss >> move_str) {  // Loop over space seperated moves string.
    Move_Generator mg(m_chess_board);
    Chess_Move_List moves;
    mg.generate_all_moves(moves);

    for (const auto& move : moves) {
      if (move_str == move.to_coordinate_notation(is_frc)) {
        m_chess_board.make_move(move);
        break;
      }
    }
  }
}
