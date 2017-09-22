#include <set>

#include "game.h"

// Utility

bool operator==(const Pyramid& lhs, const Pyramid& rhs) {
  return lhs.size == rhs.size && lhs.colour == rhs.colour;
}

bool operator<(const Pyramid& lhs, const Pyramid& rhs) {
  return lhs.size < rhs.size ||
    (lhs.size == rhs.size && lhs.colour < rhs.colour);
}

bool operator==(const Ship& lhs, const Ship& rhs) {
  return lhs.player == rhs.player && lhs.pyramid == rhs.pyramid;
}

bool operator==(const Action& lhs, const Action& rhs) {
  return lhs.type == rhs.type &&
    lhs.system == rhs.system &&
    lhs.ship == rhs.ship &&
    lhs.system_target == rhs.system_target &&
    lhs.target == rhs.target;
}

bool operator<(const Action& lhs, const Action& rhs) {
  if (lhs.type < rhs.type) return true;
  if (lhs.type > rhs.type) return false;
  if (lhs.system < rhs.system) return true;
  if (lhs.system > rhs.system) return false;
  if (lhs.ship < rhs.ship) return true;
  if (!(lhs.ship == rhs.ship)) return false;
  if (lhs.system_target < rhs.system_target) return true;
  if (lhs.system_target > rhs.system_target) return false;
  if (lhs.target < rhs.target) return true;
  return false;
}

// Constructors

Game::Game(int num_players) :
    num_players_(num_players),
    cur_player_(1),
    done_main_action_(false),
    homeworlds_built_(0),
    next_system_(1),
    sacrifice_actions_(0),
    sacrifice_colour_(RED) {
  for (const auto colour : { RED, YELLOW, GREEN, BLUE }) {
    for (const auto size : { SMALL, MEDIUM, LARGE }) {
      Pyramid p { size, colour };
      stash_[p] = 1 + num_players;
    }
  }
}

// Getters

int Game::num_players() const {
  return num_players_;
}

int Game::cur_player() const {
  return cur_player_;
}

bool Game::done_main_action() const {
  return done_main_action_;
}

int Game::homeworlds_built() const {
  return homeworlds_built_;
}

int Game::next_system() const {
  return next_system_;
}

int Game::sacrifice_actions() const {
  return sacrifice_actions_;
}

Colour Game::sacrifice_colour() const {
  return sacrifice_colour_;
}

const std::vector<System>& Game::systems() const {
  return systems_;
}

const std::map<Pyramid, int>& Game::stash() const {
  return stash_;
}

// Setters

void Game::set_num_players(int num_players) {
  num_players_ = num_players;
}

void Game::set_cur_player(int cur_player) {
  cur_player_ = cur_player;
}

void Game::set_done_main_action(bool done_main_action) {
  done_main_action_ = done_main_action;
}

void Game::set_homeworlds_built(int homeworlds_built) {
  homeworlds_built_ = homeworlds_built;
}

void Game::set_sacrifice_actions(int sacrifice_actions) {
  sacrifice_actions_ = sacrifice_actions;
}

void Game::set_sacrifice_colour(Colour sacrifice_colour) {
  sacrifice_colour_ = sacrifice_colour;
}

// Info

int Game::winner() const {
  if (homeworlds_built_ < num_players_ || sacrifice_actions_ > 0) {
    return 0;
  }
  int w = -1;
  for (const System& s : systems_) {
    if (s.player != 0 && find_if(s.ships.begin(), s.ships.end(), [&s](const Ship& ship) {
            return ship.player == s.player; 
          }) != s.ships.end()) {
      if (w == -1) {
        w = s.player;
      } else {
        return 0;
      }
    }
  }
  return w;
}

const System& Game::get_system(int id) const {
  for (const System& system : systems_) {
    if (system.id == id) {
      return system;
    }
  }
  throw "system not found";
}

Pyramid Game::smallest_of_colour(Colour colour) const {
  for (const auto& it : stash_) {
    if (it.first.colour == colour && it.second > 0) {
      return it.first;
    }
  }
  return Pyramid{ZERO, colour};
}

int Game::hash() const {
  int systems = 0;
  for (const System& system : systems_) {
    systems += ::hash(system);
  }
  return systems ^ (PLAYER_HASH[num_players_] + PLAYER_HASH[cur_player_]) ^
    SIZE_HASH[sacrifice_actions_] ^ COLOUR_HASH[sacrifice_colour_] ^
    (31 * PLAYER_HASH[homeworlds_built_]) ^ (done_main_action_ ? 0xd7067b50 : 0);
}

std::string Game::hash_string() const {
  std::string h = "";
  // maximum compression! fails for more than 7 players
  // TODO: adapt for more than 7 players, or forbid games from having more than
  // 7 players
  h += (char)(num_players_ << 4 | cur_player_);
  h += (char)(done_main_action_ << 7 | homeworlds_built_ << 4 |
              sacrifice_actions_ << 2 | sacrifice_colour_);
  std::set<std::string> system_hashes;
  for (const System& system : systems_) {
    system_hashes.insert(::hash_string(system));
  }
  for (std::string str : system_hashes) {
    h += str;
  }
  return h;
}

void Game::legal_actions(std::vector<Action>& result) const {
  if (done_main_action_) {
    result.push_back(Action{cur_player_, PASS});
  }

  for (const System& system : systems_) {
    legal_system_actions(result, system, CATASTROPHE);

    if (!done_main_action_) {
      legal_system_actions(result, system, SACRIFICE);
    }

    if ((!done_main_action_ && colour_available(system, cur_player_, RED)) ||
        (sacrifice_actions_ > 0 && sacrifice_colour_ == RED)) {
      legal_system_actions(result, system, ATTACK);
    }

    if ((!done_main_action_ && colour_available(system, cur_player_, YELLOW)) ||
        (sacrifice_actions_ > 0 && sacrifice_colour_ == YELLOW)) {
      legal_system_actions(result, system, DISCOVER);
      legal_system_actions(result, system, TRAVEL);
    }

    if ((!done_main_action_ && colour_available(system, cur_player_, GREEN)) ||
        (sacrifice_actions_ > 0 && sacrifice_colour_ == GREEN)) {
      legal_system_actions(result, system, BUILD);
    }

    if ((!done_main_action_ && colour_available(system, cur_player_, BLUE)) ||
        (sacrifice_actions_ > 0 && sacrifice_colour_ == BLUE)) {
      legal_system_actions(result, system, TRADE);
    }
  }
}

void Game::legal_system_actions(std::vector<Action>& result,
                                const System& system, ActionType type) const {
  std::set<Action> actions;

  switch(type) {
    case PASS:
      break;

    case ATTACK: {
        Size largest = ZERO;
        for (const Ship& ship : system.ships) {
          if (ship.player == cur_player_ && ship.pyramid.size > largest) {
            largest = ship.pyramid.size;
          }
        }
        if (largest == ZERO) {
          break;

        }
        for (const Ship& ship : system.ships) {
          if (ship.player != cur_player_ && ship.pyramid.size <= largest) {
            actions.insert(
                Action{cur_player_, ATTACK, system.id, ship.pyramid});
          }
        }
      }
      break;

    case DISCOVER:
      for (const auto& it : stash_) {
        if (connected(system, it.first) && it.second > 0) {
          for (const Ship& ship : system.ships) {
            if (ship.player == cur_player_) {
              actions.insert(Action{cur_player_, DISCOVER,
                  system.id, ship.pyramid, 0, it.first});
            }
          }
        }
      }
      break;

    case TRAVEL:
      for (const auto& to_system : systems_) {
        if (connected(system, to_system)) {
          for (const Ship& ship : system.ships) {
            if (ship.player == cur_player_) {
              actions.insert(Action{cur_player_, TRAVEL,
                  system.id, ship.pyramid, to_system.id});
            }
          }
        }
      }
      break;

    case BUILD:
      for (Colour colour : { RED, YELLOW, GREEN, BLUE }) {
        if (colour_available(system, cur_player_, colour, false)) {
          Pyramid p = smallest_of_colour(colour);
          if (p.size != ZERO && stash_.at(p) > 0) {
            actions.insert(Action{cur_player_, BUILD, system.id, p});
          }
        }
      }
      break;

    case TRADE:
      for (const Ship& ship : system.ships) {
        if (ship.player == cur_player_) {
          for (Colour colour : { RED, YELLOW, GREEN, BLUE }) {
            Pyramid trade_pyramid{ship.pyramid.size, colour};
            if (colour != ship.pyramid.colour && stash_.at(trade_pyramid) > 0) {
              actions.insert(Action{cur_player_, TRADE,
                  system.id, ship.pyramid, 0, trade_pyramid});
            }
          }
        }
      }
      break;

    case SACRIFICE:
      for (const Ship& ship : system.ships) {
        if (ship.player == cur_player_) {
          actions.insert(Action{cur_player_, SACRIFICE,
              system.id, ship.pyramid});
        }
      }
      break;

    case CATASTROPHE: {
        int colour_counts[4] = {0};
        for (const Pyramid& star : system.stars) {
          colour_counts[star.colour]++;
        }
        for (const Ship& ship : system.ships) {
          if (++colour_counts[ship.pyramid.colour] == 4) {
            actions.insert(Action{cur_player_, CATASTROPHE,
                system.id, ship.pyramid});
          }
        }
      }
      break;
  }

  result.insert(result.end(), actions.begin(), actions.end());
}

// Actions

bool Game::perform_action(Action& action) {
  if (action.player != cur_player_) {
    return false;
  }

  if (sacrifice_actions_ > 0) {
    sacrifice_actions_--;
  }

  switch(action.type) {
    case PASS:
      advance_cur_player();
      break;

    case ATTACK: {
        // Need to find player of attacked ship
        const System& system = get_system(action.system);
        const auto& ship = find_if(system.ships.begin(), system.ships.end(),
            [&action](const Ship& ship) {
              return ship.pyramid == action.ship && ship.player != action.player;
            });
        if (ship == system.ships.end()) {
          return false;
        }
        remove_ship(action.system, *ship);
        add_ship(action.system, Ship{action.player, ship->pyramid});
        done_main_action_ = true;
      }
      break;

    case DISCOVER: {
        int system_id = create_system({action.target});
        action.system_target = system_id;
        add_ship(system_id,
            remove_ship(action.system, Ship{action.player, action.ship}));
        if (get_system(action.system).ships.size() == 0) {
          destroy_system(action.system);
        }
        done_main_action_ = true;
      }
      break;

    case TRAVEL:
      add_ship(action.system_target,
          remove_ship(action.system, Ship{action.player, action.ship}));
      if (get_system(action.system).ships.size() == 0) {
        destroy_system(action.system);
      }
      done_main_action_ = true;
      break;

    case BUILD:
      add_ship(action.system, Ship{action.player, action.ship});
      done_main_action_ = true;
      break;

    case TRADE:
      remove_ship(action.system, Ship{action.player, action.ship});
      add_ship(action.system, Ship{action.player, action.target});
      done_main_action_ = true;
      break;

    case SACRIFICE:
      remove_ship(action.system, Ship{action.player, action.ship});
      if (get_system(action.system).ships.size() == 0) {
        destroy_system(action.system);
      }
      sacrifice_colour_ = action.ship.colour;
      sacrifice_actions_ = action.ship.size;
      done_main_action_ = true;
      break;

    case CATASTROPHE:
      apply_catastrophe(action.system, action.ship.colour);
      break;

  }

  return true;
}

// Mutators

int Game::create_system(const std::vector<Pyramid>& stars, int player) {
  for (const Pyramid& star : stars) {
    stash_[star]--;
  }
  if (player != 0) {
    homeworlds_built_++;
  }
  return add_system(System{0, player, stars});
}

int Game::add_system(System system) {
  system.id = next_system_;
  systems_.push_back(system);
  return next_system_++;
}

void Game::destroy_system(int system_id) {
  const auto& it = find_if(systems_.begin(), systems_.end(),
      [system_id](const System& s) {
        return s.id == system_id;
      });
  for (const Pyramid& star : it->stars) {
    stash_[star]++;
  }
  for (const Ship& ship : it->ships) {
    stash_[ship.pyramid]++;
  }
  systems_.erase(it);
}

void Game::add_ship(int system_id, const Ship& ship) {
  auto it = find_if(systems_.begin(), systems_.end(),
      [system_id](const System& s) {
        return s.id == system_id;
      });
  it->ships.push_back(ship);
  stash_[ship.pyramid]--;
}

Ship Game::remove_ship(int system_id, const Ship& ship) {
  auto system = find_if(systems_.begin(), systems_.end(),
      [system_id](const System& s) {
        return s.id == system_id;
      });
  const auto& it = find_if(system->ships.begin(), system->ships.end(),
      [&ship](const Ship& ship2) {
        return ship == ship2;
      });
  if (it != system->ships.end()) {
    stash_[ship.pyramid]++;
    system->ships.erase(it);
  }
  return ship;
}

void Game::apply_catastrophe(int system_id, Colour colour) {
  auto it = find_if(systems_.begin(), systems_.end(),
      [system_id](const System& system) {
        return system.id == system_id;
      });
  const auto& stars_end = std::remove_if(it->stars.begin(), it->stars.end(),
        [colour](const Pyramid& pyramid) {
          return pyramid.colour == colour;
        });
  std::for_each(stars_end, it->stars.end(), [this](const Pyramid& pyramid) {
        this->stash_[pyramid]++;
      });
  it->stars.erase(stars_end, it->stars.end());
  const auto& ships_end = std::remove_if(it->ships.begin(), it->ships.end(),
        [colour](const Ship& ship) {
          return ship.pyramid.colour == colour;
        });
  std::for_each(ships_end, it->ships.end(), [this](const Ship& ship) {
        this->stash_[ship.pyramid]++;
      });
  it->ships.erase(ships_end, it->ships.end());

  if (it->stars.size() == 0 || it->ships.size() == 0) {
    destroy_system(system_id);
  }
}

void Game::advance_cur_player() {
  cur_player_ = (cur_player_ % num_players_) + 1;
  if (winner() == 0) {
    done_main_action_ = false;
    sacrifice_actions_ = 0;
    // TODO: handle eliminated players in games with more than two players
  }
}

// Utility

bool connected(const System& a, const System& b) {
  for (const Pyramid& star : a.stars) {
    for (const Pyramid& star2 : b.stars) {
      if (star.size == star2.size) {
        return false;
      }
    }
  }
  return true;
}

bool connected(const System& a, const Pyramid& b) {
  for (const Pyramid& star : a.stars) {
    if (star.size == b.size) {
      return false;
    }
  }
  return true;
}

bool colour_available(const System& system, int player, Colour colour,
    bool include_stars) {
  if (include_stars) {
    for (const Pyramid& star : system.stars) {
      if (star.colour == colour) {
        return true;
      }
    }
  }

  for (const Ship& ship : system.ships) {
    if (ship.player == player && ship.pyramid.colour == colour) {
      return true;
    }
  }

  return false;
}

int hash(const System& system) {
  int ships = SHIPS_HASH;
  int stars = STARS_HASH;
  for (const Pyramid& star : system.stars) {
    stars += hash(star);
  }
  for (const Ship& ship : system.ships) {
    ships += hash(ship);
  }
  return ships ^ stars ^ PLAYER_HASH[system.player];
}

int hash(const Ship& ship) {
  return hash(ship.pyramid) ^ PLAYER_HASH[ship.player];
}

int hash(const Pyramid& pyramid) {
  return SIZE_HASH[pyramid.size] ^ COLOUR_HASH[pyramid.colour];
}

std::string hash_string(const System& system) {
  std::set<char> chars;
  for (const Pyramid& pyramid : system.stars) {
    chars.insert(hash_string(Ship{system.player, pyramid}));
  }
  for (const Ship& ship : system.ships) {
    chars.insert(0x80 | hash_string(ship));
  }
  return std::string(chars.begin(), chars.end());
}

char hash_string(const Ship& ship) {
  return (char)(ship.player << 4 | hash_string(ship.pyramid));
}

char hash_string(const Pyramid& pyramid) {
  return (char)(pyramid.size << 2 | pyramid.colour);
}
