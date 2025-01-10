#ifndef DUNGEONS_H_
#define DUNGEONS_H_

#include "gui.hpp"
#include "map.hpp"

class Dungeons : public GNode {
  std::unique_ptr<Map> map;

  void draw_virt(tcod::Console& console, int x, int y, int w, int h) const override;
  void process_input_virt(int c, uint16_t mods) override;

 public:
  Dungeons();
  ~Dungeons() override{};
};

#endif  // DUNGEONS_H_
