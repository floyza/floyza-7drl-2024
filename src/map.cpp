#include "map.hpp"

#include <SDL.h>

#include <map>
#include <string>

#include "colors.hpp"
#include "dir.hpp"
#include "drawing.hpp"
#include "dungeons.hpp"
#include "items.hpp"
#include "los_path.hpp"
#include "rand.hpp"
#include "util.hpp"

Map::Map(Dungeons* parent, int w, int h)
    : map(w, h),
      discovered(w, std::vector<bool>(h, true)),
      floor_items(),
      item_quantities(items.size(), 0),
      messages(),
      parent(parent) {
  generate();
  player_id = new_actor_id();
  actors[player_id] = create_player(player_id, entrance_);

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
          int id = new_actor_id();
          actors[id] = generate_monster(id, breeds[rand_int(0, breeds.size() - 1)], loc);
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

  floor_items = {};
  int item_count = rand_int(4, 6);
  while (item_count > 0) {
    Pos p{rand_int(0, width - 1), rand_int(0, height - 1)};
    if (is_walkable(p) && !floor_items.contains(p)) {
      floor_items[p] = FloorItem{.item = 0, .amount = rand_int(1, 2) * 5};
      --item_count;
    }
  }
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

std::optional<const Actor*> Map::actor_at_pos(Pos pos) const {
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
  map.computeFov(player->p.x, player->p.y, 8, true, FOV_PERMISSIVE_4);
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
      actor.mon->breed.ai(*this, actor);
    }
  }
}

void Map::process_input_virt(int c, uint16_t mods) {
  bool used_action = false;
  if (target_selecting) {
    switch (c) {
      case 'h':
      case SDLK_KP_4:
      case SDLK_LEFT:
        target_selecting->pos += Pos{-1, 0};
        break;
      case 'j':
      case SDLK_DOWN:
      case SDLK_KP_2:
        target_selecting->pos += Pos{0, 1};
        break;
      case 'k':
      case SDLK_UP:
      case SDLK_KP_8:
        target_selecting->pos += Pos{0, -1};
        break;
      case 'l':
      case SDLK_RIGHT:
      case SDLK_KP_6:
        target_selecting->pos += Pos{1, 0};
        break;
      case 'y':
      case SDLK_KP_7:
        target_selecting->pos += Pos{-1, -1};
        break;
      case 'u':
      case SDLK_KP_9:
        target_selecting->pos += Pos{1, -1};
        break;
      case 'b':
      case SDLK_KP_1:
        target_selecting->pos += Pos{-1, 1};
        break;
      case 'n':
      case SDLK_KP_3:
        target_selecting->pos += Pos{1, 1};
        break;
      case SDLK_KP_ENTER:
      case SDLK_RETURN:
      case SDLK_RETURN2:  // what the heck is this?
        if (attempt_target_select()) {
          used_action = true;
          target_selecting = std::nullopt;
        }
        break;
      case SDLK_ESCAPE:
      case SDLK_BACKSPACE:
        target_selecting = std::nullopt;
    }
  } else if (examining) {
    switch (c) {
      case 'h':
      case SDLK_KP_4:
      case SDLK_LEFT:
        examining->pos += Pos{-1, 0};
        break;
      case 'j':
      case SDLK_DOWN:
      case SDLK_KP_2:
        examining->pos += Pos{0, 1};
        break;
      case 'k':
      case SDLK_UP:
      case SDLK_KP_8:
        examining->pos += Pos{0, -1};
        break;
      case 'l':
      case SDLK_RIGHT:
      case SDLK_KP_6:
        examining->pos += Pos{1, 0};
        break;
      case 'y':
      case SDLK_KP_7:
        examining->pos += Pos{-1, -1};
        break;
      case 'u':
      case SDLK_KP_9:
        examining->pos += Pos{1, -1};
        break;
      case 'b':
      case SDLK_KP_1:
        examining->pos += Pos{-1, 1};
        break;
      case 'n':
      case SDLK_KP_3:
        examining->pos += Pos{1, 1};
        break;
      case SDLK_ESCAPE:
      case SDLK_BACKSPACE:
        examining = std::nullopt;
    }
  } else {
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
      case 'x':
      case '/':
        examining = Examining{.pos = player->p};
        break;
      case '1':
      case '2':
      case '3':
      case '4':
      case '5': {
        int ability = std::stoi(std::string(1, static_cast<char>(c))) - 1;
        if (mods & SHIFT) {
          add_message("DESC: " + items[ability].desc);
          messages.next_turn();
        } else {
          used_action = use_ability(ability);
        }
        break;
      }
      case 's':
      case SDLK_KP_5:
      case SDLK_KP_PERIOD:
        // handle period lower down
        used_action = true;
        break;
      case 'g':
      case ',':
        used_action = pickup_item();
        break;
      case '.':
        if (!(mods & SHIFT)) {
          // not '>'
          used_action = true;
          break;
        }
        // fallthrough
      case '>':
        if (player->p == exit_) {
          parent->left_dungeon = true;
        }
        break;
    }
  }
  if (used_action) {
    ++turn_count;
    check_dead();
    monsters_act();
    check_dead();
    messages.next_turn();
  }
}

bool Map::pickup_item() {
  auto mbitem = floor_items.find(player->p);
  if (mbitem != floor_items.end()) {
    FloorItem item = mbitem->second;
    item_quantities[item.item] += item.amount;
    floor_items.erase(mbitem);
    messages.add_message("Picked up " + items[item.item].name + " x" + std::to_string(item.amount));
    return true;
  }
  return false;
}

bool Map::attempt_target_select() {
  assert(target_selecting);

  if (in_fov(target_selecting->pos) && is_walkable(target_selecting->pos)) {
    TCODPath path = get_los_path(*this, player->p, target_selecting->pos);
    bool self_target = path.isEmpty();
    bool blocked = false;
    for (int i = 0; i < path.size() - 1; ++i) {
      int lx, ly;
      path.get(i, &lx, &ly);
      if (actor_at_pos({lx, ly})) {
        blocked = true;
        break;
      }
    }
    bool used = std::visit(
        [this, blocked, self_target](auto& action) -> bool {
          using T = std::decay_t<decltype(action)>;
          if constexpr (std::is_same_v<T, ItemF<ActorSS>>) {
            if (blocked || self_target) {
              return false;
            }
            auto mbactor = actor_at_pos(target_selecting->pos);
            if (!mbactor) {
              return false;
            }
            Actor& actor = **mbactor;
            action(*this, ActorSS{actor.id});
            return true;
          } else if constexpr (std::is_same_v<T, ItemF<PosSS>>) {
            if (blocked) {
              return false;
            }
            action(*this, PosSS(target_selecting->pos));
            return true;
          } else if constexpr (std::is_same_v<T, ItemF<EmptyPos>>) {
            if (actor_at_pos(target_selecting->pos).has_value()) {
              return false;
            }
            action(*this, EmptyPos(target_selecting->pos));
            return true;
          }
          panic("std::visit for itemfv: missing case");
        },
        target_selecting->callback);

    if (used) {
      item_quantities[target_selecting->item_to_consume] -= 1;
    }
    return used;
  }
  return false;
}

bool Map::use_ability(int ability) {
  const Item& item = items[ability];
  bool used_action = false;
  std::visit(
      [this, &used_action, ability, item](auto&& action) {
        using T = std::decay_t<decltype(action)>;
        if constexpr (std::is_same_v<T, ItemF<>>) {
          action(*this);
          item_quantities[ability] -= 1;
          used_action = true;
        } else {
          target_selecting = {
              .pos = player->p, .item_to_consume = ability, .callback = action, .drawing_flags = item.drawing_flags};
        }
      },
      item.action);
  return used_action;
}

void Map::check_dead() {
  for (auto it = actors.begin(); it != actors.end();) {
    if (it->second.hp <= 0) {
      if (it->second.is_player) {
        // you die
        it->second.hp = 10;
      } else {
        it = actors.erase(it);
        continue;
      }
    }
    ++it;
  }
}

void Map::draw_virt(tcod::Console& console, int x, int y, int w, int h) const {
  int sbw = 14;
  int sb2w = 14;

  draw_level(console, x, y, w - sbw - sb2w - 2, h);

  draw_vline(console, x + w - sbw - sb2w - 2, y, h);
  console[{x + w - sb2w - sbw - 2, y - 1}].ch = L'┬';
  draw_usables(console, x + w - sbw - sb2w - 1, y, sb2w, h);

  draw_vline(console, x + w - sbw - 1, y, h);
  console[{x + w - sbw - 1, y - 1}].ch = L'┬';
  double health_rat = std::max(0.0, static_cast<double>(player->hp) / player->max_hp);
  TCOD_ColorRGB health_color = col::GREEN;
  if (health_rat < .6) {
    health_color = col::YELLOW;
  }
  if (health_rat < .3) {
    health_color = col::RED;
  }
  tcod::print(console, {x + w - sbw + 1, y}, "HP:", col::WHITE_BR, std::nullopt);
  tcod::print(console, {x + w - sbw + 4, y}, std::to_string(player->hp), health_color, std::nullopt);
  tcod::print(
      console,
      {x + w - sbw + 4 + static_cast<int>(std::to_string(player->hp).length()), y},
      "/" + std::to_string(player->max_hp),
      col::WHITE_BR,
      std::nullopt);
  int healthbar_len = sbw - 2;
  int removed = (1 - health_rat) * healthbar_len;
  tcod::print(console, {x + w - sbw + 1, y + 1}, std::string(healthbar_len - removed, '*'), health_color, std::nullopt);
  tcod::print(
      console,
      {x + w - sbw + 1 + (healthbar_len - removed), y + 1},
      std::string(removed, '-'),
      col::WHITE,
      std::nullopt);
  tcod::print(console, {x + w - sbw + 1, y + 2}, "ATK:" + std::to_string(player->atk), col::WHITE_BR, std::nullopt);

  // summarize seen monsters
  std::multimap<std::string, const Actor*> sorted_monsters;
  for (const auto& [i, a] : actors) {
    if (in_fov(a.p) && a.mon) {
      sorted_monsters.insert(std::make_pair(a.mon->breed.name, &a));
    }
  }
  int col = y + 4;
  for (auto it = sorted_monsters.begin(); it != sorted_monsters.end();) {
    std::string key = it->first;
    const Actor* mon = it->second;
    int count = 0;
    do {
      ++count;
      ++it;
    } while (it != sorted_monsters.end() && key == it->first);
    auto& num_tile = console[{x + w - sbw, col}];
    if (count > 9) {
      num_tile.fg = col::CYAN_BR;
      num_tile.ch = '#';
    } else if (count > 1) {
      num_tile.fg = col::WHITE_BR;
      num_tile.ch = std::to_string(count)[0];
    } else {
      num_tile.ch = '*';
      num_tile.fg = col::GREEN;
      health_rat = static_cast<double>(mon->hp) / mon->max_hp;
      if (health_rat < .6) {
        num_tile.fg = col::YELLOW;
      }
      if (health_rat < .3) {
        num_tile.fg = col::RED;
      }
    }
    auto& spr_tile = console[{x + w - sbw + 1, col}];
    spr_tile.fg = mon->color;
    spr_tile.ch = mon->chr;
    tcod::print(console, {x + w - sbw + 3, col}, mon->mon->breed.name, col::WHITE_BR, std::nullopt);
    ++col;
  }
}

void Map::draw_level(tcod::Console& console, int x, int y, int w, int h) const {
  auto wall = col::WHITE;
  auto floor = col::YELLOW;
  int basex = player->p.x - w / 2;
  int basey = player->p.y - h / 2;
  if (examining) {
    basex = examining->pos.x - w / 2;
    basey = examining->pos.y - h / 2;
  }
  if (target_selecting) {
    basex = target_selecting->pos.x - w / 2;
    basey = target_selecting->pos.y - h / 2;
  }
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
  for (auto [p, i] : floor_items) {
    if (in_fov(p)) {
      int console_x = p.x - basex + x;
      int console_y = p.y - basey + y;
      const auto& item = items[i.item];
      if (in_rect(console_x, console_y, x, y, w, h)) {
        auto& tile = console.at(console_x, console_y);
        tile.fg = item.color;
        tile.ch = item.chr;
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
  int bottom_offset = 0;
  std::vector<std::string> ms = messages.current_messages();
  for (auto it = ms.rbegin(); it != ms.rend(); ++it) {
    tcod::print(console, {x + 1, y + h - 1 + (bottom_offset--)}, *it, col::WHITE, std::nullopt);
  }
  if (examining) {
    std::swap(console[{x + w / 2, y + h / 2}].bg, console[{x + w / 2, y + h / 2}].fg);
    draw_desc(console, examining->pos, {x + 1, y + h - 1 + bottom_offset});
  }
  if (target_selecting) {
    bool good_path = false;
    if (in_fov(target_selecting->pos) && is_walkable(target_selecting->pos)) {
      if (is_smite(target_selecting->callback)) {
        good_path = true;
      } else {
        good_path = true;
        TCODPath path = get_los_path(*this, player->p, target_selecting->pos);
        for (int i = 0; i < path.size() - 1; ++i) {
          int lx, ly;
          path.get(i, &lx, &ly);
          if (actor_at_pos({lx, ly})) {
            good_path = false;
            break;
          }
        }
        for (int i = 0; i < path.size() - 1; ++i) {
          int lx, ly;
          path.get(i, &lx, &ly);
          auto& tile = console[{lx - basex + x, ly - basey + y}];
          if (actor_at_pos({lx, ly})) {
            tile.fg = col::BLACK;
            tile.bg = col::RED;
          } else {
            tile.ch = '*';
            if (good_path) {
              tile.fg = col::MAGENTA;
            } else {
              tile.fg = col::BLACK_BR;
            }
          }
        }
      }
    }
    Pos center(x + w / 2, y + h / 2);
    if (good_path && (target_selecting->drawing_flags & FIREBALL_DRAW)) {
      std::vector targets = {
          Pos(-1, -1), Pos(-1, 0), Pos(-1, 1), Pos(0, -1), Pos(0, 1), Pos(1, -1), Pos(1, 0), Pos(1, 1)};
      for (Pos d : targets) {
        Pos t = center + d;
        Pos treal = target_selecting->pos + d;
        if (!in_fov(treal) || !is_walkable(treal)) {
          continue;
        }
        if (!actor_at_pos(treal)) {
          console[t].ch = '*';
        }
        console[t].fg = col::BLACK;
        console[t].bg = col::MAGENTA;
      }
    }

    if (!actor_at_pos(target_selecting->pos)) {
      console[center].ch = '*';
    }
    console[center].fg = col::BLACK;
    if (good_path) {
      console[center].bg = col::RED_BR;
    } else {
      console[center].bg = col::RED;
    }
    draw_desc(console, target_selecting->pos, {x + 1, y + h - 1 + bottom_offset});
  }
}

void Map::draw_usables(tcod::Console& console, int x, int y, int w, int h) const {
  int starting_y = y;
  tcod::print(console, {x + 1, y++}, "Abilities", col::WHITE_BR, std::nullopt);
  tcod::print(console, {x + 1, y++}, std::string(w - 2, '-'), col::WHITE_BR, std::nullopt);
  for (int i = 0; i < static_cast<int>(items.size()); ++i) {
    std::string desc = std::to_string(i + 1) + ")" + items[i].name;
    tcod::print(console, {x + 1, y}, desc, col::WHITE_BR, std::nullopt);
    draw_d(console, x + static_cast<int>(desc.length()) + 2, y, item_quantities[i]);
    ++y;
  }
  tcod::print(console, {x + 1, starting_y + h - 2}, "TC: " + std::to_string(turn_count), col::WHITE_BR, std::nullopt);
}

// assumes there's some room above
void Map::draw_desc(tcod::Console& console, Pos tile, Pos disp) const {
  bool visible = in_fov(tile);
  bool seen_before = discovered[tile.x][tile.y];
  auto mbactor = actor_at_pos(tile);
  std::optional<std::pair<TCOD_ColorRGB, std::string>> hpdata;
  std::string actormsg;
  if (mbactor) {
    const auto& actor = **mbactor;
    if (actor.mon) {
      const auto& name = actor.mon->breed.name;
      if (name.length() > 0 && is_vowel(name[0])) {
        actormsg = "An ";
      } else {
        actormsg = "A ";
      }
      actormsg += name;
    } else {
      actormsg = "You";
    }
    std::string msg =
        "HP:" + std::to_string(actor.hp) + "/" + std::to_string(actor.max_hp) + ", ATK:" + std::to_string(actor.atk);
    hpdata = std::make_pair(col::GREEN, msg);
    double health_rat = static_cast<double>(actor.hp) / actor.max_hp;
    if (health_rat < .6) {
      hpdata->first = col::YELLOW;
    }
    if (health_rat < .3) {
      hpdata->first = col::RED;
    }
  }
  std::string floormsg;
  if (tile == exit_) {
    floormsg = "A portal out of here";
  } else if (is_walkable(tile)) {
    floormsg = "The floor";
  } else {
    floormsg = "A wall";
  }

  int y_from_bottom = 0;
  if (seen_before) {
    if (floormsg != "") {
      tcod::print(console, {disp.x, disp.y - y_from_bottom}, floormsg, col::WHITE_BR, std::nullopt);
      ++y_from_bottom;
    }
  } else {
    tcod::print(
        console, {disp.x, disp.y - y_from_bottom}, "You've never been there before.", col::WHITE_BR, std::nullopt);
  }
  if (visible && actormsg != "") {
    tcod::print(console, {disp.x, disp.y - y_from_bottom}, actormsg, col::WHITE_BR, std::nullopt);
    if (hpdata) {
      int x = disp.x + actormsg.length();
      int y = disp.y - y_from_bottom;
      console[{x + 1, y}].ch = ' ';
      console[{x + 1, y}].ch = '(';
      console[{x + 1, y}].fg = col::WHITE_BR;
      console[{x + 2, y}].ch = '*';
      console[{x + 2, y}].fg = hpdata->first;
      console[{x + 3, y}].ch = ')';
      console[{x + 3, y}].fg = col::WHITE_BR;
      console[{x + 3, y}].ch = ')';
      console[{x + 4, y}].ch = ' ';
      tcod::print(console, {x + 5, y}, hpdata->second, col::WHITE_BR, std::nullopt);
      ++y_from_bottom;
    }
  }
  auto mbitem = floor_items.find(tile);
  if (visible && mbitem != floor_items.end()) {
    const auto& [_, floor_item] = *mbitem;
    const auto& item = items[floor_item.item];
    tcod::print(
        console,
        {disp.x, disp.y - y_from_bottom},
        "Item on ground: " + item.name + " x" + std::to_string(floor_item.amount),
        col::WHITE_BR,
        std::nullopt);
  }
}
