#ifndef DUNGEONS_H_
#define DUNGEONS_H_

#include "gui.hpp"
#include "map.hpp"

struct MapResults {
  std::vector<int> item_quantities;
  int turn_amount;
};

class Dungeons : public GNode {
  void draw_virt(tcod::Console& console, int x, int y, int w, int h) const override;
  void process_input_virt(int c, uint16_t mods) override;

 public:
  Dungeons();
  ~Dungeons() override{};

  bool left_dungeon = false;  // flag for map to set

  std::variant<
      std::monostate,  // waiting to generate next map
      std::unique_ptr<Map>,  // in map
      MapResults  // waiting to apply results
      >
      state;
};

#endif  // DUNGEONS_H_
