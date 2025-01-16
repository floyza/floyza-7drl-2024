#ifndef ITEMS_H_
#define ITEMS_H_

#include <libtcod/color.h>

#include <functional>
#include <libtcod/console_types.hpp>
#include <string>
#include <variant>

#include "pos.hpp"

class Map;

// actor which has a straight shot between it and the player
// cannot target self
struct ActorSS {
  int id;
};

// straight shot to empty square
struct PosSS {
  Pos pt;
};

// smite to empty spot
struct EmptyPos {
  Pos pt;
};

template <typename... Args>
using ItemF = std::function<void(Map&, Args...)>;

using ItemFV = std::variant<ItemF<>, ItemF<ActorSS>, ItemF<PosSS>, ItemF<EmptyPos>>;

bool is_smite(const ItemFV& f);

enum DrawingFlags { FIREBALL_DRAW = 1 };

struct Item {
  std::string name;
  std::string desc;
  int chr;
  TCOD_ColorRGB color;
  ItemFV action;
  unsigned drawing_flags = 0;
};

extern const std::vector<Item> items;  // length of 9

#endif
