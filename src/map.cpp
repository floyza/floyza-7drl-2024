#include "map.hpp"

#include <SDL.h>

#include "colors.hpp"
#include "dir.hpp"
#include "rand.hpp"

Map::Map(int w, int h) : map(w, h), discovered(w, std::vector<bool>(h, false)) {
  generate();
  player_id = new_actor_id();
  actors[player_id] = create_player(entrance_);

  player = &actors[player_id];  // is never invalidated unless it is erased, which it isn't
}

int Map::new_actor_id() { return ++actor_id_ctr; }

void Map::generate() {
  constexpr double grain_count = 5;
  constexpr double filled_percent = 75;

  const int granularity = std::min(std::ceil(get_width() / grain_count), std::ceil(get_height() / grain_count));

  int tries = 0;

  bool generating = true;
  while (generating) {
    ++tries;
    map.clear();
    gen_rand_walk();

    int total_grids = 0;
    int filled_grids = 0;

    for (int x = 0; x < get_width(); x += granularity) {
      for (int y = 0; y < get_height(); y += granularity) {
        ++total_grids;
        for (int x_fine = x; x_fine < x + granularity && x_fine < get_width(); ++x_fine) {
          for (int y_fine = y; y_fine < y + granularity && y_fine < get_height(); ++y_fine) {
            if (is_walkable(Pos{x_fine, y_fine})) {
              ++filled_grids;
              // break out of x_fine and y_fine loops
              x_fine = get_width();
              break;
            }
          }
        }
      }
    }

    if (total_grids * (filled_percent / 100) < filled_grids) generating = false;
  }
}
void Map::gen_rand_walk() {
  // Map generation done with modified 'drunkard's walk' algorithm
  // Basic algorithm described here:
  // http://www.roguebasin.com/index.php?title=Random_Walk_Cave_Generation With
  // some modifications described here:
  // https://forums.roguetemple.com//index.php?topic=4128.0

  // tweakable variables:
  constexpr double percentage = 30;
  constexpr double sober_chance = 30;
  constexpr double sober_density_allowed = 35;
  constexpr double soft_edge_limit_percent = 20;
  constexpr int hall_min = 5;
  constexpr int hall_max = 10;
  constexpr int cave_min = 15;
  constexpr int cave_max = 25;
  constexpr int start_variation = 5;

  constexpr double monster_chance = 6;
  actors = {};

  const int width = get_width();
  const int height = get_height();
  const int soft_edge_limit = std::min(width, height) * (soft_edge_limit_percent / 100);
  const int req_tiles = (width * height) * (percentage / 100);
  const int max_steps = req_tiles * 500;

  int done_tiles = rand_int(cave_min, cave_max);
  int total_steps = 0;

  Pos loc;
  loc.x = width / 2 + rand_int(-start_variation, start_variation);
  loc.y = height / 2 + rand_int(-start_variation, start_variation);
  entrance_ = loc;

  bool sober = false;  // true if we are digging a corridor
  int steps = rand_int(cave_min, cave_max);  // steps until change sober state
  Pos dir = rand_dir_orth();

  bool last_room = false;

  while (true) {
    if (done_tiles >= req_tiles && !sober) {
      if (last_room)
        break;
      else
        last_room = true;
    }
    if (!is_walkable(loc)) {
      set_walkable(loc, true);
      ++done_tiles;
    }

    if (steps-- == 0) {
      int hall_len = rand_int(hall_min, hall_max);
      if (percent_chance(sober_chance) && can_sober(loc, dir, hall_len, soft_edge_limit, sober_density_allowed) &&
          !sober) {
        sober = true;
        steps = hall_len;
      } else {
        if (percent_chance(monster_chance)) {
          actors[new_actor_id()] = generate_monster(loc);
        }
        sober = false;
        steps = rand_int(cave_min, cave_max);
      }
    }

    if (sober && in_soft_limit(loc, soft_edge_limit)) {
      sober = false;
      steps = rand_int(cave_min, cave_max);
    }

    if (!sober) {
      // turn
      Pos old_dir = dir;
      while (old_dir == dir) {
        dir = rand_dir_orth();
      }
      int limit_tier = in_soft_limit(loc, soft_edge_limit);
      if (rand_int(1, soft_edge_limit * soft_edge_limit) <= limit_tier * limit_tier)
        soft_edge_limit_dir(loc, dir, soft_edge_limit);
    }

    loc += dir;
    if (loc.x == 0)
      loc.x = 1;
    else if (loc.x == width - 1)
      loc.x = get_width() - 2;
    if (loc.y == 0)
      loc.y = 1;
    else if (loc.y == height - 1)
      loc.y = height - 2;

    if (++total_steps > max_steps) {
      // std::cerr << "map generation exceeded max_steps\n";
      break;
    }
  }
  exit_ = loc;
  set_walkable(loc, true);
}

bool Map::in_level(const Pos& pos) const {
  return !(pos.x < 0 || pos.x > get_width() - 1 || pos.y < 0 || pos.y > get_height() - 1);
}

bool Map::soft_edge_limit_dir(const Pos& pos, Pos& d, int limit) {
  bool changed = false;
  if (pos.x < limit) {
    d = Dir::e;
    changed = true;
  } else if (pos.x > get_width() - 1 - limit) {
    d = Dir::w;
    changed = true;
  } else if (pos.y < limit) {
    d = Dir::s;
    changed = true;
  } else if (pos.y > get_height() - 1 - limit) {
    d = Dir::n;
    changed = true;
  }
  return changed;
}

int Map::in_soft_limit(const Pos& pos, int limit) {
  using std::max;
  int depth = 0;
  if (pos.x < limit)
    depth = max(depth, limit - pos.x);
  else if (pos.x > get_width() - 1 - limit)
    depth = max(depth, limit - (get_width() - pos.x - 1));
  if (pos.y < limit)
    depth = max(depth, limit - pos.y);
  else if (pos.y > get_height() - 1 - limit)
    depth = max(depth, limit - (get_height() - pos.y - 1));
  return depth;
}

bool Map::can_sober(const Pos& pos, const Pos& dir, int hall_len, int limit, double density_allowed) {
  if (in_soft_limit(pos, limit)) return false;

  // increase hall_len to make it odd
  const int rect_size = hall_len + !(hall_len % 2);
  Pos rect = {(pos.x - hall_len / 2) + dir.x * (hall_len / 2), (pos.y - hall_len / 2) + dir.y * (hall_len / 2)};

  // max 25% of tiles can be floor tiles
  const int max_amount = rect_size * rect_size * (density_allowed / 100);
  int floor_count = 0;

  Pos curr = rect;

  for (; curr.x < rect.x + rect_size; ++curr.x) {
    for (; curr.y < rect.y + rect_size; ++curr.y) {
      if (!in_level(curr)) return false;
      if (is_walkable(curr)) ++floor_count;
      if (floor_count > max_amount) return false;
    }
  }
  return true;
}

void Map::set_walkable(const Pos& pos, bool walkable) { map.setProperties(pos.x, pos.y, walkable, walkable); }

std::optional<Actor*> Map::actor_at_pos(Pos pos) {
  for (auto& [_, actor] : actors) {
    if (actor.p == pos) {
      return &actor;
    }
  }
  return std::nullopt;
}

std::optional<Actor*> Map::get_actor(int id) {
  auto a = actors.find(id);
  if (a != actors.end()) {
    return &a->second;
  }
  return std::nullopt;
}

void Map::recompute_fov() const {
  map.computeFov(player->p.x, player->p.y, 8, true, FOV_BASIC);
  fov_dirty = false;
  for (int x = 0; x < get_width(); ++x) {
    for (int y = 0; y < get_height(); ++y) {
      if (in_fov(Pos{x, y})) {
        discovered[x][y] = true;
      }
    }
  }
}

bool Map::in_fov(Pos pos) const {
  if (fov_dirty) {
    recompute_fov();
  }
  return map.isInFov(pos.x, pos.y);
}

void Map::monsters_act() {
  for (auto& [_, actor] : actors) {
    if (actor.mon) {
      monster_act(*this, actor);
    }
  }
}

void Map::process_input_virt(int c) {
  bool used_action = false;
  switch (c) {
    case 'h':
    case SDLK_KP_4:
    case SDLK_LEFT:
      used_action = move_actor(*this, *player, Pos{-1, 0});
      break;
    case 'j':
    case SDLK_DOWN:
    case SDLK_KP_2:
      used_action = move_actor(*this, *player, Pos{0, 1});
      break;
    case 'k':
    case SDLK_UP:
    case SDLK_KP_8:
      used_action = move_actor(*this, *player, Pos{0, -1});
      break;
    case 'l':
    case SDLK_RIGHT:
    case SDLK_KP_6:
      used_action = move_actor(*this, *player, Pos{1, 0});
      break;
    case 'y':
    case SDLK_KP_7:
      used_action = move_actor(*this, *player, Pos{-1, -1});
      break;
    case 'u':
    case SDLK_KP_9:
      used_action = move_actor(*this, *player, Pos{1, -1});
      break;
    case 'b':
    case SDLK_KP_1:
      used_action = move_actor(*this, *player, Pos{-1, 1});
      break;
    case 'n':
    case SDLK_KP_3:
      used_action = move_actor(*this, *player, Pos{1, 1});
      break;
    case 's':
    case SDLK_KP_5:
    case SDLK_PERIOD:
    case SDLK_KP_PERIOD:
      used_action = true;
      break;
  }
  if (used_action) {
    monsters_act();
  }
}

void Map::draw_virt(tcod::Console& console, int x, int y, int w, int h) const {
  auto wall = col::WHITE;
  auto floor = col::YELLOW;
  int basex = player->p.x - w / 2;
  int basey = player->p.y - h / 2;
  for (int tx = basex; tx < basex + w; ++tx) {
    for (int ty = basey; ty < basey + h; ++ty) {
      if (in_level(Pos{tx, ty})) {
        auto& tile = console.at(tx - basex + x, ty - basey + y);
        bool seeing = in_fov({tx, ty});
        if (seeing || discovered[tx][ty]) {
          if (Pos{tx, ty} == exit_) {
            tile.fg = col::WHITE_BR;
            tile.ch = '>';
          } else if (is_walkable(Pos{tx, ty})) {
            tile.fg = floor;
            tile.ch = '.';
          } else {
            tile.fg = wall;
            tile.ch = '#';
          }
        }
        if (!seeing) {
          tile.fg = col::BLACK_BR;
        }
      }
    }
  }
  for (const auto& elem : actors) {
    const Actor& actor = elem.second;
    if (in_fov(actor.p)) {
      int console_x = actor.p.x - basex + x;
      int console_y = actor.p.y - basey + y;
      if (in_rect(console_x, console_y, x, y, w, h)) {
        auto& tile = console.at(console_x, console_y);
        tile.fg = actor.color;
        tile.ch = actor.chr;
      }
    }
  }
}
