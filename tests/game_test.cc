#include <iostream>

#include "../game.h"
#include "catch.hpp"

TEST_CASE("creating and destroying systems and ships affects the stash") {
  Game g = Game(2);

  for (Colour colour : {RED, YELLOW, GREEN, BLUE}) {
    for (Size size : {SMALL, MEDIUM, LARGE}) {
      REQUIRE((g.stash().at(Pyramid{size, colour}) == 3));
    }
  }

  SECTION("adding system removes stars from stash") {
    int system_id = g.create_system({Pyramid{SMALL, YELLOW},
        Pyramid{MEDIUM, GREEN}});

    REQUIRE((g.stash().at(Pyramid{SMALL, YELLOW}) == 2));
    REQUIRE((g.stash().at(Pyramid{MEDIUM, GREEN}) == 2));

    SECTION("adding ship removes pyramid from stash") {
      g.add_ship(system_id, Ship{1, Pyramid{LARGE, RED}});

      REQUIRE((g.stash().at(Pyramid{LARGE, RED}) == 2));

      SECTION("removing ship adds pyramid to stash") {
        Ship removed_ship = g.remove_ship(system_id,
            Ship{1, Pyramid{LARGE, RED}});

        REQUIRE((removed_ship == Ship{1, Pyramid{LARGE, RED}}));
        REQUIRE((g.stash().at(Pyramid{LARGE, RED}) == 3));
      }

      SECTION("removing system adds all pyramids to stash") {
        g.destroy_system(system_id);

        REQUIRE((g.stash().at(Pyramid{SMALL, YELLOW}) == 3));
        REQUIRE((g.stash().at(Pyramid{MEDIUM, GREEN}) == 3));
        REQUIRE((g.stash().at(Pyramid{LARGE, RED}) == 3));
      }
    }
  }
}

TEST_CASE("generating legal actions for a system") {
  Game g = Game(2);

  int main_system = g.create_system(
      {Pyramid{SMALL, YELLOW}, Pyramid{MEDIUM, BLUE}});
  g.add_ship(main_system, Ship{1, Pyramid{MEDIUM, GREEN}});
  g.add_ship(main_system, Ship{1, Pyramid{SMALL, BLUE}});
  g.add_ship(main_system, Ship{2, Pyramid{MEDIUM, RED}});

  std::vector<Action> actions;

  SECTION("attack actions only attack equal or smaller enemy ships") {
    g.add_ship(main_system, Ship{2, Pyramid{LARGE, BLUE}});
    g.legal_system_actions(actions, g.get_system(main_system), ATTACK);
    std::sort(actions.begin(), actions.end());

    REQUIRE(actions.size() == 1);
    REQUIRE((actions[0] == Action{1, ATTACK, main_system,
          Pyramid{MEDIUM, RED}}));

    SECTION("removing medium ship invalidates all attacks") {
      actions.clear();
      g.remove_ship(main_system, Ship{1, Pyramid{MEDIUM, GREEN}});
      g.legal_system_actions(actions, g.get_system(main_system), ATTACK);

      REQUIRE(actions.size() == 0);
    }
  }

  SECTION("discover actions only lead to connected systems") {
    g.remove_ship(main_system, Ship{1, Pyramid{MEDIUM, GREEN}});
    g.legal_system_actions(actions, g.get_system(main_system), DISCOVER);
    std::sort(actions.begin(), actions.end());

    REQUIRE(actions.size() == 4);
    REQUIRE((actions[0] == Action{1, DISCOVER, main_system,
          Pyramid{SMALL, BLUE}, 0, Pyramid{LARGE, RED}}));
    REQUIRE((actions[1] == Action{1, DISCOVER, main_system,
          Pyramid{SMALL, BLUE}, 0, Pyramid{LARGE, YELLOW}}));
    REQUIRE((actions[2] == Action{1, DISCOVER, main_system,
          Pyramid{SMALL, BLUE}, 0, Pyramid{LARGE, GREEN}}));
    REQUIRE((actions[3] == Action{1, DISCOVER, main_system,
          Pyramid{SMALL, BLUE}, 0, Pyramid{LARGE, BLUE}}));
  }

  SECTION("travel actions only lead to connected systems") {
    int alpha = g.create_system({Pyramid{LARGE, RED}});
    int beta = g.create_system({Pyramid{LARGE, RED}, Pyramid{LARGE, GREEN}});
    int gamma = g.create_system({Pyramid{SMALL, RED}});
    g.legal_system_actions(actions, g.get_system(main_system), TRAVEL);
    std::sort(actions.begin(), actions.end());

    REQUIRE(actions.size() == 4);
    REQUIRE((actions[0] == Action{1, TRAVEL, main_system,
          Pyramid{SMALL, BLUE}, alpha}));
    REQUIRE((actions[1] == Action{1, TRAVEL, main_system,
          Pyramid{SMALL, BLUE}, beta}));
    REQUIRE((actions[2] == Action{1, TRAVEL, main_system,
          Pyramid{MEDIUM, GREEN}, alpha}));
    REQUIRE((actions[3] == Action{1, TRAVEL, main_system,
          Pyramid{MEDIUM, GREEN}, beta}));
  }

  SECTION("build actions build smallest pyramid of colours available") {
    // Use remaining small blue pyramids
    g.add_ship(main_system, Ship{1, Pyramid{SMALL, BLUE}});
    g.add_ship(main_system, Ship{1, Pyramid{SMALL, BLUE}});
    g.legal_system_actions(actions, g.get_system(main_system), BUILD);
    std::sort(actions.begin(), actions.end());

    REQUIRE(actions.size() == 2);
    REQUIRE((actions[0] == Action{1, BUILD, main_system,
          Pyramid{SMALL, GREEN}}));
    REQUIRE((actions[1] == Action{1, BUILD, main_system,
          Pyramid{MEDIUM, BLUE}}));

    SECTION("can't build anything if no pyramids of that colour remain") {
      actions.clear();
      g.add_ship(main_system, Ship{1, Pyramid{MEDIUM, BLUE}});
      g.add_ship(main_system, Ship{1, Pyramid{MEDIUM, BLUE}});
      g.add_ship(main_system, Ship{1, Pyramid{LARGE, BLUE}});
      g.add_ship(main_system, Ship{1, Pyramid{LARGE, BLUE}});
      g.add_ship(main_system, Ship{1, Pyramid{LARGE, BLUE}});
      g.legal_system_actions(actions, g.get_system(main_system), BUILD);

      REQUIRE(actions.size() == 1);
      REQUIRE((actions[0] == Action{1, BUILD, main_system,
            Pyramid{SMALL, GREEN}}));
    }
  }

  SECTION("trade actions are to the same size and different colours") {
    g.legal_system_actions(actions, g.get_system(main_system), TRADE);

    REQUIRE(actions.size() == 6);
    for (const Action& action : actions) {
      REQUIRE(action.ship.size == action.target.size);
      REQUIRE(action.ship.colour != action.target.colour);
    }
  }

  SECTION("sacrifice action for each ship in the system") {
    g.legal_system_actions(actions, g.get_system(main_system), SACRIFICE);
    std::sort(actions.begin(), actions.end());

    REQUIRE(actions.size() == 2);
    REQUIRE((actions[0] == Action{1, SACRIFICE, main_system,
          Pyramid{SMALL, BLUE}}));
    REQUIRE((actions[1] == Action{1, SACRIFICE, main_system,
          Pyramid{MEDIUM, GREEN}}));
  }

  SECTION("catastrophe action count includes star") {
    g.add_ship(main_system, Ship{1, Pyramid{SMALL, BLUE}});
    g.add_ship(main_system, Ship{1, Pyramid{SMALL, BLUE}});
    g.legal_system_actions(actions, g.get_system(main_system), CATASTROPHE);

    REQUIRE(actions.size() == 1);
    REQUIRE(actions[0].ship.colour == BLUE);
  }
}

TEST_CASE("computing winner") {
  Game g = Game(2);
  
  REQUIRE(g.homeworlds_built() == 0);

  SECTION("no winner declared before homeworlds are done being built") {
    int player1 = g.create_system({Pyramid{SMALL, BLUE}, Pyramid{MEDIUM, YELLOW}}, 1);
    g.add_ship(player1, Ship{1, Pyramid{LARGE, GREEN}});
    
    REQUIRE(g.homeworlds_built() == 1);
    REQUIRE(g.winner() == 0);

    SECTION("game still in progress after both homeworlds exist") {
      int player2 = g.create_system({Pyramid{SMALL, BLUE}, Pyramid{MEDIUM, YELLOW}}, 2);
      g.add_ship(player2, Ship{2, Pyramid{LARGE, GREEN}});

      REQUIRE(g.homeworlds_built() == 2);
      REQUIRE(g.winner() == 0);

      SECTION("destroying a homeworld is a win for the other player") {
        g.destroy_system(player1);

        REQUIRE(g.winner() == 2);

        SECTION("destroying both homeworlds is a tie") {
          g.destroy_system(player2);

          REQUIRE(g.winner() == -1);
        }
      }

      SECTION("destroying all allied ships in a homeworld"
              "is a win for the other player") {
        g.add_ship(player2, Ship{1, Pyramid{LARGE, RED}});
        g.remove_ship(player2, Ship{2, Pyramid{LARGE, GREEN}});

        REQUIRE(g.winner() == 1);
      }
    }
  }
}

TEST_CASE("generating all legal actions") {
  Game g = Game(2);

  int main_system = g.create_system(
      {Pyramid{SMALL, YELLOW}, Pyramid{MEDIUM, RED}});
  g.add_ship(main_system, Ship{1, Pyramid{MEDIUM, GREEN}});
  g.add_ship(main_system, Ship{1, Pyramid{SMALL, BLUE}});
  g.add_ship(main_system, Ship{2, Pyramid{MEDIUM, BLUE}});

  int other_system = g.create_system({Pyramid{LARGE, GREEN}});
  int unreachable_system = g.create_system({Pyramid{SMALL, GREEN}});

  std::vector<Action> actions;

  SECTION("actions with all colours available and no catastrophes") {
    g.legal_actions(actions);
    std::sort(actions.begin(), actions.end());

    REQUIRE(actions.size() == 21);
    REQUIRE((actions[0] == Action{1, ATTACK, main_system, Pyramid{MEDIUM, BLUE}}));
    REQUIRE((actions[1] == Action{1, DISCOVER, main_system,
          Pyramid{SMALL, BLUE}, 0, Pyramid{LARGE, RED}}));
    for (int i = 2; i < 9; i++) {
      REQUIRE(actions[i].type == DISCOVER);
    }
    REQUIRE((actions[9] == Action{1, TRAVEL, main_system,
          Pyramid{SMALL, BLUE}, other_system}));
    REQUIRE((actions[10] == Action{1, TRAVEL, main_system,
          Pyramid{MEDIUM, GREEN}, other_system}));
    REQUIRE((actions[11] == Action{1, BUILD, main_system, Pyramid{SMALL, GREEN}}));
    REQUIRE((actions[12] == Action{1, BUILD, main_system, Pyramid{SMALL, BLUE}}));
    REQUIRE((actions[13] == Action{1, TRADE, main_system,
          Pyramid{SMALL, BLUE}, 0, Pyramid{SMALL, RED}}));
    for (int i = 14; i < 19; i++) {
      REQUIRE(actions[i].type == TRADE);
    }
    REQUIRE((actions[19] == Action{1, SACRIFICE, main_system, Pyramid{SMALL, BLUE}}));
    REQUIRE((actions[20] == Action{1, SACRIFICE, main_system, Pyramid{MEDIUM, GREEN}}));
  }

  SECTION("actions with a colour unavailable") {
    g.remove_ship(main_system, Ship{1, Pyramid{SMALL, BLUE}});
    g.legal_actions(actions);

    REQUIRE(find_if(actions.begin(), actions.end(), [](const Action& action) {
            return action.type == TRADE;
          }) == actions.end());

    SECTION("actions with a colour unavailable but with a sacrifice performed") {
      actions.clear();
      g.set_done_main_action(true);
      g.set_sacrifice_actions(1);
      g.set_sacrifice_colour(BLUE);
      g.legal_actions(actions);

      REQUIRE(actions.size() == 4);
      REQUIRE((actions[0] == Action{1, PASS}));
      REQUIRE((actions[1] == Action{1, TRADE, main_system,
            Pyramid{MEDIUM, GREEN}, 0, Pyramid{MEDIUM, RED}}));
      REQUIRE((actions[2] == Action{1, TRADE, main_system,
            Pyramid{MEDIUM, GREEN}, 0, Pyramid{MEDIUM, YELLOW}}));
      REQUIRE((actions[3] == Action{1, TRADE, main_system,
            Pyramid{MEDIUM, GREEN}, 0, Pyramid{MEDIUM, BLUE}}));
    }
  }

  SECTION("actions after performing a main action") {
    g.set_done_main_action(true);
    g.legal_actions(actions);

    REQUIRE(actions.size() == 1);
    REQUIRE((actions[0] == Action{1, PASS}));
  }

  SECTION("actions with a catastrophe") {
    g.add_ship(main_system, Ship{1, Pyramid{SMALL, BLUE}});
    g.add_ship(main_system, Ship{1, Pyramid{SMALL, BLUE}});
    g.legal_actions(actions);
    std::sort(actions.begin(), actions.end());

    REQUIRE(actions.size() == 22);
    REQUIRE(actions[21].type == CATASTROPHE);
    REQUIRE(actions[21].ship.colour == BLUE);

    SECTION("actions with a catastrophe after performing a main action") {
      actions.clear();
      g.set_done_main_action(true);
      g.legal_actions(actions);
      std::sort(actions.begin(), actions.end());

      REQUIRE(actions.size() == 2);
      REQUIRE((actions[0] == Action{1, PASS}));
      REQUIRE(actions[1].type == CATASTROPHE);
      REQUIRE(actions[1].ship.colour == BLUE);
    }
  }
}

TEST_CASE("sanity checking getters and setters") {
  Game g = Game(2);

  g.set_num_players(7);
  g.set_cur_player(6);
  g.set_done_main_action(true);
  g.set_homeworlds_built(5);
  g.set_sacrifice_actions(0);
  g.set_sacrifice_colour(BLUE);
  g.create_system({Pyramid{SMALL, BLUE}});

  REQUIRE(g.num_players() == 7);
  REQUIRE(g.cur_player() == 6);
  REQUIRE(g.done_main_action() == true);
  REQUIRE(g.homeworlds_built() == 5);
  REQUIRE(g.next_system() == 2);
  REQUIRE(g.sacrifice_actions() == 0);
  REQUIRE(g.sacrifice_colour() == BLUE);
  REQUIRE(g.systems().size() == 1);
}

TEST_CASE("hash strings of equivalent Games should be equal") {
  Game g1 = Game(2);
  Game g2 = Game(2);

  REQUIRE(g1.hash_string() == g2.hash_string());

  SECTION("adding systems in different orders should not affect hash strings") {
    int g1s1 = g1.create_system({Pyramid{SMALL, BLUE}, Pyramid{LARGE, YELLOW}}, 1);
    int g1s2 = g1.create_system({Pyramid{MEDIUM, GREEN}, Pyramid{LARGE, RED}}, 2);

    REQUIRE(g1.hash_string() != g2.hash_string());

    int g2s2 = g2.create_system({Pyramid{MEDIUM, GREEN}, Pyramid{LARGE, RED}}, 2);
    int g2s1 = g2.create_system({Pyramid{SMALL, BLUE}, Pyramid{LARGE, YELLOW}}, 1);

    REQUIRE(g1.hash_string() == g2.hash_string());

    SECTION("adding ships in different orders should not affect hash strings") {
      g1.add_ship(g1s1, Ship{1, Pyramid{LARGE, GREEN}});
      g1.add_ship(g1s1, Ship{2, Pyramid{SMALL, YELLOW}});

      REQUIRE(g1.hash_string() != g2.hash_string());

      g2.add_ship(g2s1, Ship{2, Pyramid{SMALL, YELLOW}});
      g2.add_ship(g2s1, Ship{1, Pyramid{LARGE, GREEN}});

      REQUIRE(g1.hash_string() == g2.hash_string());
    }
  }
}

TEST_CASE("hash codes of equivalent Games should be equal") {
  Game g1 = Game(2);
  Game g2 = Game(2);

  REQUIRE(g1.hash() == g2.hash());

  SECTION("adding systems in different orders should not affect hash strings") {
    int g1s1 = g1.create_system({Pyramid{SMALL, BLUE}, Pyramid{LARGE, YELLOW}}, 1);
    int g1s2 = g1.create_system({Pyramid{MEDIUM, GREEN}, Pyramid{LARGE, RED}}, 2);

    REQUIRE(g1.hash() != g2.hash());

    int g2s2 = g2.create_system({Pyramid{MEDIUM, GREEN}, Pyramid{LARGE, RED}}, 2);
    int g2s1 = g2.create_system({Pyramid{SMALL, BLUE}, Pyramid{LARGE, YELLOW}}, 1);

    REQUIRE(g1.hash() == g2.hash());

    SECTION("adding ships in different orders should not affect hash strings") {
      g1.add_ship(g1s1, Ship{1, Pyramid{LARGE, GREEN}});
      g1.add_ship(g1s1, Ship{2, Pyramid{SMALL, YELLOW}});

      REQUIRE(g1.hash() != g2.hash());

      g2.add_ship(g2s1, Ship{2, Pyramid{SMALL, YELLOW}});
      g2.add_ship(g2s1, Ship{1, Pyramid{LARGE, GREEN}});

      REQUIRE(g1.hash() == g2.hash());
    }
  }
}
