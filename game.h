#ifndef GAME_H
#define GAME_H

#include <iostream>
#include <map>
#include <string>
#include <vector>

enum Size { ZERO = 0, SMALL, MEDIUM, LARGE };
enum Colour { RED = 0, YELLOW, GREEN, BLUE };

struct Pyramid {
  Size size;
  Colour colour;
};

bool operator==(const Pyramid& lhs, const Pyramid& rhs);
bool operator<(const Pyramid& lhs, const Pyramid& rhs);

struct Ship {
  int player;
  Pyramid pyramid;
};

bool operator==(const Ship& lhs, const Ship& rhs);

struct System {
  int id;
  int player; // 0 if not homeworld
  std::vector<Pyramid> stars;
  std::vector<Ship> ships;
};

enum ActionType {
  PASS,
  ATTACK,
  DISCOVER,
  TRAVEL,
  BUILD,
  TRADE,
  SACRIFICE,
  CATASTROPHE
};

struct Action {
  int player;
  ActionType type;
  int system; // all
  Pyramid ship; // all
  // TODO: in games with more than two players, ship can be ambiguous in ATTACKs
  int system_target; // TRAVEL
  Pyramid target; // TRADE, DISCOVER
};

bool operator==(const Action& lhs, const Action& rhs);
bool operator<(const Action& lhs, const Action& rhs);

const static unsigned int SIZE_HASH[] = {0x96ef527d, 0xbf6ff0bb, 0x55742d1e, 0x12972d93};
const static unsigned int COLOUR_HASH[] = {0x7fd9fd6a, 0x8dd08654, 0x29f99b21, 0x3a707ca3};
const static unsigned int PLAYER_HASH[] = {0x3e624de3, 0x1a9e20e1, 0x52c25952};
const static unsigned int STARS_HASH = 0xd9a110c5;
const static unsigned int SHIPS_HASH = 0x12ec3a54;

class Game {
 public:
  // Constructors
  Game(int num_players);

  // Getters
  int num_players() const;
  int cur_player() const; // players start from 1
  bool done_main_action() const;
  int homeworlds_built() const;
  int next_system() const;
  int sacrifice_actions() const;
  Colour sacrifice_colour() const;
  const std::vector<System>& systems() const;
  const std::map<Pyramid, int>& stash() const;
  
  // Setters (testing only)
  void set_num_players(int num_players);
  void set_cur_player(int cur_player);
  void set_done_main_action(bool done_main_action);
  void set_homeworlds_built(int homeworlds_built);
  void set_sacrifice_actions(int sacrifice_actions);
  void set_sacrifice_colour(Colour sacrifice_colour);

  // Info
  int winner() const; // 0 if game not complete, -1 if tie
  const System& get_system(int id) const;
  Pyramid smallest_of_colour(Colour colour) const;
  int hash() const;
  std::string hash_string() const;

  void legal_actions(std::vector<Action>& result) const;
  void legal_system_actions(std::vector<Action>& result, const System& system,
      ActionType type) const;

  // Actions
  bool perform_action(Action& action); // returns success

  // Mutators
  int create_system(const std::vector<Pyramid>& stars, int player = 0);
  int add_system(System system);
  void destroy_system(int system_id);
  void add_ship(int system_id, const Ship& ship);
  Ship remove_ship(int system_id, const Ship& ship);
  void apply_catastrophe(int system_id, Colour colour);
  void advance_cur_player();

 private:
  int num_players_;
  int cur_player_;
  bool done_main_action_;
  int homeworlds_built_;
  int next_system_;
  int sacrifice_actions_;
  Colour sacrifice_colour_;
  std::vector<System> systems_;
  std::map<Pyramid, int> stash_;
};

// Utility

bool connected(const System& a, const System& b);
bool connected(const System& a, const Pyramid& b);
bool colour_available(const System& system, int player, Colour colour,
    bool include_stars = true);

int hash(const System& system);
int hash(const Ship& ship);
int hash(const Pyramid& pyramid);

std::string hash_string(const System& system);
char hash_string(const Ship& ship);
char hash_string(const Pyramid& pyramid);

#endif
