#include "los_path.hpp"
#include "map.hpp"

class LosPathCost : public ITCODPathCallback {
 public:
  float getWalkCost(int, int, int x_dest, int y_dest, void* map_ptr) const override {
    Map& map = *static_cast<Map*>(map_ptr);
    if (map.is_walkable({x_dest, y_dest}) && map.in_fov({x_dest, y_dest})) {
      return 1;
    }
    return 0;
  }
};

TCODPath get_los_path(const Map &map, Pos start, Pos end) {
  LosPathCost fnc{};
  TCODPath path(map.get_width(), map.get_height(), &fnc, const_cast<Map*>(&map));
  path.compute(start.x, start.y, end.x, end.y);
  return path;
}
