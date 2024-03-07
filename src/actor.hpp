#ifndef ACTOR_H_
#define ACTOR_H_

#include <libtcod.hpp>

#include "pos.hpp"
class Map;

struct MonsterData {
  std::optional<Pos> target = std::nullopt;
};

struct Actor {
  Pos p;
  int chr;
  tcod::ColorRGB color;
  std::optional<MonsterData> mon = std::nullopt;
  bool is_player = false;
};

Actor create_player(Pos p);

Actor generate_monster(Pos p);

bool move_actor(Map& map, Actor& actor, Pos offset);

void monster_act(Map& map, Actor& actor);

#endif  // ACTOR_H_
