#pragma once

#include <string>
#include <vector>

namespace step {

enum class Action {
  Help,
  Version,
  Info,
  Sync,
  Install,
  Remove,
  Update,
  Search,
  Which,
  Depclean,
  Unmask,
  Mask,
  Use,
};

struct Options {
  Action action = Action::Help;
  std::vector<std::string> packages;
  std::string query;
  bool ask = false;
  bool verbose = false;
  bool pretend = false;
  bool deep = false;
  bool update = false;
  bool oneshot = false;
  bool tree = false;
  bool world = false;
};

Options parse_cli(int argc, char** argv);
std::string action_name(Action action);
void print_help();

}
