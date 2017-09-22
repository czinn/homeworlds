#include <sstream>

#include "game_io.h"

std::ostream& operator<<(std::ostream &os, const Pyramid& pyramid) {
  os << "rygb"[pyramid.colour] << pyramid.size;
  return os;
}

std::ostream& operator<<(std::ostream &os, const Ship& ship) {
  os << ship.player << ship.pyramid;
  return os;
}

std::ostream& operator<<(std::ostream &os, const System& system) {
  os << "(";
  if (system.player != 0) {
    os << system.player << ", ";
  }
  for (const Pyramid& pyramid : system.stars) {
    os << pyramid;
  }
  os << ")";
  for (const Ship& ship : system.ships) {
    os << " " << ship;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const Action& action) {
  switch(action.type) {
    case PASS:
      os << "PASS";
      break;

    case ATTACK:
      os << "ATTACK " << action.ship << " " << action.system;
      break;

    case DISCOVER:
      os << "DISCOVER " << action.ship << " " << action.system;
      os << " " << action.target;
      break;

    case TRAVEL:
      os << "MOVE " << action.ship << " " << action.system << " ";
      os << action.system_target;
      break;

    case BUILD:
      os << "BUILD " << action.ship << " " << action.system;
      break;

    case TRADE:
      os << "TRADE " << action.ship << " " << action.target;
      os << " " << action.system;
      break;

    case SACRIFICE:
      os << "SACRIFICE " << action.ship << " " << action.system;
      break;

    case CATASTROPHE:
      os << "CATASTROPHE " << action.system;
      os << " " << "rygb"[action.ship.colour];
      break;
  }
  return os;
}


void print_action(std::ostream& os, const Action& action,
                  const std::map<int, std::string>& system_names,
                  std::vector<std::string>::iterator& new_names) {
  switch (action.type) {
    case PASS:
      os << "PASS";
      break;

    case ATTACK:
      os << "ATTACK " << action.ship << " " << system_names.at(action.system);
      break;

    case DISCOVER:
      os << "DISCOVER " << action.ship << " " << system_names.at(action.system);
      os << " " << action.target << " " << *new_names++;
      break;

    case TRAVEL:
      os << "MOVE " << action.ship << " " << system_names.at(action.system) << " ";
      os << system_names.at(action.system_target);
      break;

    case BUILD:
      os << "BUILD " << action.ship << " " << system_names.at(action.system);
      break;

    case TRADE:
      os << "TRADE " << action.ship << " " << action.target;
      os << " " << system_names.at(action.system);
      break;

    case SACRIFICE:
      os << "SACRIFICE " << action.ship << " " << system_names.at(action.system);
      break;

    case CATASTROPHE:
      os << "CATASTROPHE " << system_names.at(action.system);
      os << " " << "rygb"[action.ship.colour];
      break;
  }
}

void print_game(std::ostream& os, const Game& game,
                const std::map<int, std::string>& system_names) {
  os << game.num_players() << " ";
  os << game.homeworlds_built() << " ";
  os << game.cur_player() << std::endl;
  for (const System& system : game.systems()) {
    os << system_names.at(system.id) << " " << system << std::endl;
  }
  os << std::endl;
}

std::istream& operator>>(std::istream& is, Pyramid& pyramid) {
  is >> std::ws;
  char c;
  is.get(c);
  pyramid.colour =
    c == 'r' || c == 'R' ? RED :
    c == 'y' || c == 'Y' ? YELLOW :
    c == 'g' || c == 'G' ? GREEN : BLUE;
  int size;
  is >> size;
  pyramid.size = (Size)size;
  return is;
}

std::istream& operator>>(std::istream& is, Ship& ship) {
  is >> std::ws;
  char c;
  is.get(c);
  ship.player = (int)(c - '0');
  is >> ship.pyramid;
  return is;
}

std::istream& operator>>(std::istream& is, System& system) {
  system.player = 0;
  system.id = 0;
  is >> std::ws;
  is.ignore(); // (
  if (std::isdigit(is.peek())) {
    is >> system.player;
    is.ignore(256, ' ');
  }
  while (is.peek() != ')') {
    Pyramid star;
    is >> star;
    system.stars.push_back(star);
  }
  is.ignore(); // )
  while (!is.eof()) {
    Ship ship;
    is >> ship;
    system.ships.push_back(ship);
  }
  return is;
}

Game* read_game(std::istream& is, std::map<int, std::string>& system_names) {
  int num_players, homeworlds_built, cur_player;
  std::string line;
  is >> num_players >> homeworlds_built >> cur_player;
  std::getline(is, line);
  Game *g = new Game(num_players);
  g->set_homeworlds_built(homeworlds_built);
  g->set_cur_player(cur_player);
  while (true) {
    std::getline(is, line);
    if (line.length() == 0) {
      break;
    }
    System system;
    std::istringstream sis = std::istringstream(line);
    std::string system_name;
    sis >> system_name;
    sis >> system;
    int system_id = g->add_system(system);
    system_names.emplace(system_id, system_name);
  }

  return g;
}

std::string read_action(std::istream& is, Action& action,
                        std::map<int, std::string>& system_names) {
  std::map<std::string, int> name_lookup;
  for (const auto& it : system_names) {
    name_lookup.emplace(it.second, it.first);
  }

  std::string type, system_name, colour;
  is >> type;

  if (type == "PASS") {
    action.type = PASS;
  } else if (type == "ATTACK") {
    action.type = ATTACK;
    is >> action.ship;
    is >> system_name;
    action.system = name_lookup[system_name];
  } else if (type == "DISCOVER") {
    action.type = DISCOVER;
    is >> action.ship;
    is >> system_name;
    action.system = name_lookup[system_name];
    is >> action.target;
    is >> system_name;
    return system_name;
  } else if (type == "MOVE") {
    action.type = TRAVEL;
    is >> action.ship;
    is >> system_name;
    action.system = name_lookup[system_name];
    is >> system_name;
    action.system_target = name_lookup[system_name];
  } else if (type == "BUILD") {
    action.type = BUILD;
    is >> action.ship;
    is >> system_name;
    action.system = name_lookup[system_name];
  } else if (type == "TRADE") {
    action.type = TRADE;
    is >> action.ship;
    is >> action.target;
    is >> system_name;
    action.system = name_lookup[system_name];
  } else if (type == "SACRIFICE") {
    action.type = SACRIFICE;
    is >> action.ship;
    is >> system_name;
    action.system = name_lookup[system_name];
  } else if (type == "CATASTROPHE") {
    action.type = CATASTROPHE;
    is >> system_name;
    action.system = name_lookup[system_name];
    is >> std::ws;
    is >> colour;
    if (colour == "r" || colour == "R") {
      action.ship = Pyramid{ZERO, RED};
    } else if (colour == "y" || colour == "Y") {
      action.ship = Pyramid{ZERO, YELLOW};
    } else if (colour == "g" || colour == "G") {
      action.ship = Pyramid{ZERO, GREEN};
    } else {
      action.ship = Pyramid{ZERO, BLUE};
    }
  }

  return "";
}
