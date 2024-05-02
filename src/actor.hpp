#ifndef ACTOR_H_
#define ACTOR_H_

#include <libtcod.hpp>

#include "breeds.hpp"
#include "pos.hpp"
class Map;

struct MonsterData {
  Breed breed;
  std::optional<Pos> target = std::nullopt;
};

struct Actor {
  int id;
  Pos p;
  int chr;
  tcod::ColorRGB color;
  std::optional<MonsterData> mon = std::nullopt;
  bool is_player = false;

  int hp;
  int max_hp;
  int atk;
};

Actor create_player(int id, Pos p);

Actor generate_monster(int id, Breed b, Pos p);

#endif  // ACTOR_H_
