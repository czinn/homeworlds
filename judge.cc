#include <iostream>

#include "game.h"
#include "game_io.h"

int main() {
  std::map<int, std::string> system_names;
  Game *g = read_game(std::cin, system_names);

  while (!std::cin.eof()) {
    Action a;
    std::string system_name = read_action(std::cin, a, system_names);
    a.player = g->cur_player();
    g->perform_action(a);
    if (a.type == DISCOVER) {
      system_names.emplace(a.system_target, system_name);
    } else if (a.type == PASS) {
      break;
    }
  }

  print_game(std::cout, *g, system_names);

  if (g->winner() != 0) {
    std::cout << g->winner() << std::endl;
  }
}
