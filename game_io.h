#ifndef GAME_IO_H
#define GAME_IO_H

#include <map>
#include <string>
#include <vector>

#include "game.h"

std::ostream& operator<<(std::ostream& os, const Pyramid& pyramid);
std::ostream& operator<<(std::ostream& os, const Ship& ship);
std::ostream& operator<<(std::ostream& os, const System& system);
std::ostream& operator<<(std::ostream& os, const Action& action);
void print_action(std::ostream& os, const Action& action,
                  const std::map<int, std::string>& system_names,
                  std::vector<std::string>::iterator& new_names);
void print_game(std::ostream& os, const Game& game,
                const std::map<int, std::string>& system_names);

std::istream& operator>>(std::istream& is, Ship& ship);
std::istream& operator>>(std::istream& is, System& system);
Game* read_game(std::istream& is, std::map<int, std::string>& system_names);
std::string read_action(std::istream& is, Action& action,
                        std::map<int, std::string>& system_names);

#endif
