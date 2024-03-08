#ifndef MAP_H_
#define MAP_H_

#include <libtcod.hpp>
#include <unordered_map>
#include <vector>

#include "actor.hpp"
#include "gui.hpp"
#include "pos.hpp"

struct Examining {
  Pos pos;
};

class Map : public GNode {
  mutable TCODMap map;
  mutable std::vector<std::vector<bool>> discovered;  // discovered[x][y]
  mutable bool fov_dirty =
      true;  // currently assumes walls and stuff don't change, the only invalidation is the player moving
  friend bool move_actor(Map&, Actor&, Pos);
  void recompute_fov() const;

  Pos exit_;
  Pos entrance_;

  void generate();
  void gen_rand_walk();
  bool soft_edge_limit_dir(const Pos& pos, Pos& d, int limit);
  int in_soft_limit(const Pos& pos, int limit);
  bool can_sober(const Pos& pos, const Pos& dir, int hall_len, int limit, double density_allowed);
  bool in_level(const Pos& pos) const;

  int actor_id_ctr = 0;
  int new_actor_id();

  int player_id;
  Actor* player;

  void draw_level(tcod::Console& console, int x, int y, int w, int h) const;
  void draw_virt(tcod::Console& console, int x, int y, int w, int h) const override;
  void process_input_virt(int c) override;

  void monsters_act();
  void check_dead();

  std::optional<Examining> examining;
  void draw_desc(tcod::Console& console, Pos tile, Pos disp) const;

 public:
  Map(int w, int h);
  Pos entrance() const { return entrance_; }
  Pos exit() const { return exit_; }

  void set_walkable(const Pos& pos, bool walkable);

  int get_width() const { return map.getWidth(); }
  int get_height() const { return map.getHeight(); }
  bool is_walkable(const Pos& pos) const { return map.isWalkable(pos.x, pos.y); }
  std::optional<const Actor*> actor_at_pos(Pos pos) const;
  std::optional<Actor*> actor_at_pos(Pos pos) {
    auto val = const_cast<const Map*>(this)->actor_at_pos(pos);
    if (val) {
      return const_cast<Actor*>(*val);
    }
    return std::nullopt;
  }
  std::optional<Actor*> get_actor(int id);
  std::unordered_map<int, Actor> actors;

  Actor* get_player() { return player; }

  bool in_fov(Pos pos) const;
};

#endif  // MAP_H_
