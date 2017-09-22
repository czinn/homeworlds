#ifndef NEGAMAX_H
#define NEGAMAX_H

#include <deque>
#include <string>
#include <unordered_map>
#include <vector>

#include "game.h"

struct Turn {
  Game *game;
  std::deque<Action> actions;

  ~Turn();
};

enum TranspositionFlag { EXACT, LOWERBOUND, UPPERBOUND };

struct Transposition {
  int value;
  int depth;
  TranspositionFlag flag;
};

class Negamax {
  public:
    Negamax(const Game *game);

    std::vector<Action> get_actions(int depth);
    int negamax(const Game *game, int depth, int a, int b);
    int heuristic(const Game *game);
    int half_heuristic(const Game *game, int player);

  private:
    const Game *root_game;

    std::vector<Turn*> get_turns(const Game *game);
    std::unordered_map<int, Transposition> transpositions;
};

#endif
