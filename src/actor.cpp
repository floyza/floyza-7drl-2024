#include "actor.hpp"

#include "colors.hpp"
#include "map.hpp"
#include "rand.hpp"

Actor create_player(Pos p) { return Actor{.p = p, .chr = '@', .color = col::WHITE_BR, .is_player = true}; }

Actor generate_monster(Pos p) {
  MonsterData mon = {};
  return Actor{.p = p, .chr = 'g', .color = col::BLUE_BR, .mon = mon};
}

bool move_actor(Map& map, Actor& actor, Pos offset) {
  if (actor.is_player) {
    map.fov_dirty = true;
  }
  Pos new_pos = actor.p + offset;
  if (map.is_walkable(new_pos)) {
    if (map.actor_at_pos(new_pos)) {
      // bump attack
    } else {
      actor.p = new_pos;
      return true;
    }
  }
  return false;
}

class MonsterPathCost : public ITCODPathCallback {
 public:
  float getWalkCost(int, int, int x_dest, int y_dest, void* map_ptr) const override {
    Map& map = *static_cast<Map*>(map_ptr);
    if (map.is_walkable({x_dest, y_dest})) {
      auto mb_actor = map.actor_at_pos({x_dest, y_dest});
      if (mb_actor and (*mb_actor)->mon) {
        // we want monsters to crowd around a full chokepoint
        return 4;
      } else {
        return 1;
      }
    } else {
      return 0;
    }
  }
};

void monster_act(Map& map, Actor& actor) {
  if (map.in_fov(actor.p)) {
    actor.mon->target = map.get_player()->p;
  } else if (percent_chance(30)) {
    actor.mon->target = std::nullopt;
  }
  if (actor.mon->target) {
    // we create a new path each turn, alternatively we could use the builtin `TCODPath::walk'
    MonsterPathCost fnc{};
    TCODPath path(map.get_width(), map.get_height(), &fnc, &map);
    path.compute(actor.p.x, actor.p.y, actor.mon->target->x, actor.mon->target->y);
    if (path.isEmpty()) {
      actor.mon->target = std::nullopt;
    } else {
      Pos p;
      path.get(0, &p.x, &p.y);
      move_actor(map, actor, p - actor.p);
    }
  } else {
    // wander randomly
    if (percent_chance(50)) {
      move_actor(map, actor, rand_dir());
    }
  }
}
