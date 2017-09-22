#include <iostream>

#include "game.h"
#include "negamax.h"
#include "game_io.h"

int main() {
  std::map<int, std::string> system_names;
  Game *g = read_game(std::cin, system_names);

  if (g->winner() != 0) {
    print_game(std::cout, *g, system_names);
    return 0;
  }

  Negamax nm(g);
  std::vector<Action> actions;
  for (int i = 1; i <= 2; i++) {
    actions = nm.get_actions(i);
  }
  std::vector<std::string> new_names({"Sirius","AlphaCentauri","Mars","Venus"});
  auto new_names_it = new_names.begin();
  for (Action& a : actions) {
    if (a.type == DISCOVER) {
      system_names.emplace(a.system_target, *new_names_it);
    }
    print_action(std::cout, a, system_names, new_names_it);
    std::cout << std::endl;
    g->perform_action(a);
  }

  delete g;
}
