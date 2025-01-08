#ifndef ITEMS_H_
#define ITEMS_H_

#include <libtcod/color.h>

#include <functional>
#include <string>
#include <variant>

class Map;

// actor which has a straight shot between it and the player
// cannot target self
struct ActorSS {
  int id;
};

template <typename... Args>
using ItemF = std::function<void(Map&, Args...)>;

struct Item {
  std::string name;
  std::string desc;
  int chr;
  TCOD_ColorRGB color;
  std::variant<ItemF<>, ItemF<ActorSS>> action;
};

extern const std::vector<Item> items;

#endif
