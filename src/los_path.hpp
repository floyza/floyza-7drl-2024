#ifndef LOS_PATH_H_
#define LOS_PATH_H_

#include <libtcod.hpp>

#include "pos.hpp"

class Map;

TCODPath get_los_path(const Map& map, Pos start, Pos end);

#endif  // LOS_PATH_H_
