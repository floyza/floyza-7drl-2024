#ifndef BREEDS_H_
#define BREEDS_H_

#include <functional>
#include <libtcod/color.hpp>
#include <string>

struct Actor;
class Map;

struct Breed {
  std::string name;
  int chr;
  tcod::ColorRGB color;
  int max_hp;
  int atk;
  std::function<void(Map&, Actor&)> ai;
};

extern const std::vector<Breed> breeds;

#endif  // BREEDS_H_
