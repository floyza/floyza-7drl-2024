#ifndef ACTOR_AI_H_
#define ACTOR_AI_H_

#include "pos.hpp"

class Map;
struct Actor;

bool move_actor(Map& map, Actor& actor, Pos offset);

void monster_act(Map& map, Actor& actor);

#endif
