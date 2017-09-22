#include <algorithm>

#include "negamax.h"
#include "game_io.h"

Turn::~Turn() {
  delete game;
}

Negamax::Negamax(const Game *game) : root_game(game) {}

std::vector<Action> Negamax::get_actions(int depth) {
  if (depth == 0 || root_game->winner() != 0) {
    return std::vector<Action>({Action{PASS}});
  }

  int best = -10000000;
  const Turn *best_turn;
  std::vector<Turn*> turns = get_turns(root_game);
  std::sort(turns.begin(), turns.end(),
      [this](const Turn *a, const Turn *b) {
        int ha = a->game->hash();
        int hb = b->game->hash();
        int sa = this->transpositions.count(ha) > 0 ? this->transpositions[ha].value : 0;
        int sb = this->transpositions.count(hb) > 0 ? this->transpositions[hb].value : 0;
        return sa < sb;
      });
  int a = -10000000;
  int b = 10000000;
  for (const Turn *turn : turns) {
    int value = -negamax(turn->game, depth - 1, -b, -a);
    if (value > best) {
      best = value;
      best_turn = turn;
    }
    /*
    std::cout << "Value: " << value << std::endl;
    for (const Action& action : turn->actions) {
      std::cout << action << std::endl;
    }
    std::cout << std::endl;
    */
    a = std::max(a, value);
    if (a >= b) {
      break;
    }
  }

  std::vector<Action> actions(best_turn->actions.begin(), best_turn->actions.end());
  for (Turn *turn : turns) {
    delete turn;
  }
  return actions;
}

int Negamax::negamax(const Game *game, int depth, int a, int b) {
  int olda = a;

  int h = game->hash();
  if (transpositions.count(h) > 0) {
    Transposition t = transpositions[h];
    if (t.depth >= depth) {
      if (t.flag == EXACT) {
        return t.value;
      } else if (t.flag == LOWERBOUND) {
        a = std::max(a, t.value);
      } else if (t.flag == UPPERBOUND) {
        a = std::min(b, t.value);
      }
      if (a >= b) {
        return t.value;
      }
    }
  }

  if (depth == 0 || game->winner() != 0) {
    return heuristic(game);
  }

  int best = -10000000;
  std::vector<Turn*> turns = get_turns(game);
  std::sort(turns.begin(), turns.end(),
      [this](const Turn *a, const Turn *b) {
        int ha = a->game->hash();
        int hb = b->game->hash();
        int sa = this->transpositions.count(ha) > 0 ? this->transpositions[ha].value : 0;
        int sb = this->transpositions.count(hb) > 0 ? this->transpositions[hb].value : 0;
        return sa > sb;
      });
  int num = 0;
  for (const Turn *turn : turns) {
    int value = -negamax(turn->game, depth - 1, -b, -a);
    best = std::max(best, value);
    a = std::max(a, value);
    num++;
    if (a >= b) {
      break;
    }
  }
  for (Turn *turn : turns) {
    delete turn;
  }

  Transposition t{best, depth};
  if (best <= olda) {
    t.flag = UPPERBOUND;
  } else if (best >= b) {
    t.flag = LOWERBOUND;
  } else {
    t.flag = EXACT;
  }
  transpositions[h] = t;

  return best;
}

int Negamax::heuristic(const Game *game) {
  // Negamax only works for two players
  return half_heuristic(game, game->cur_player()) -
      half_heuristic(game, 3 - game->cur_player());
}

int Negamax::half_heuristic(const Game *game, int player) {
  if (game->winner() == player) {
    return 1000000;
  } else if (game->winner() != 0) {
    return 0;
  }

  int total = 0;
  for (const System& system : game->systems()) {
    for (const Ship& ship : system.ships) {
      if (ship.player == player) {
        total += 100 * ship.pyramid.size * ship.pyramid.size;
      }
    }
  }

  return total;
}


std::vector<Turn*> Negamax::get_turns(const Game *game) {
  std::unordered_map<int, Turn*> result;
  std::vector<Action> actions;
  game->legal_actions(actions);

  for (Action& action : actions) {
    if (action.type == PASS) {
      Game *g = new Game(*game);
      g->perform_action(action);
      result.emplace(g->hash(), new Turn{g, std::deque<Action>({action})});
      continue;
    }

    Game *g = new Game(*game);
    g->perform_action(action);
    for (Turn *turn : get_turns(g)) {
      if (result.count(turn->game->hash()) == 0) {
        turn->actions.push_front(action);
        result.emplace(turn->game->hash(), turn);
      } else {
        delete turn;
      }
    }
    delete g;
  }

  std::vector<Turn*> real_result;
  for (const auto& p : result) {
    real_result.push_back(p.second);
  }
  return real_result;
}
