#ifndef FACTORY_H_
#define FACTORY_H_

#include "gui.hpp"

class Factory : public GNode {
  void draw_items(tcod::Console& console, int x, int y, int w, int h) const;

  void draw_virt(tcod::Console& console, int x, int y, int w, int h) const override;
  void process_input_virt(int c, uint16_t mods) override;
  void tick_virt(double seconds) override;

  double counter = 0;
  std::vector<int> item_quantities;

 public:
  Factory();
  ~Factory() override{};
};

#endif
