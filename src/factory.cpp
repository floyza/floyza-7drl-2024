#include "factory.hpp"

#include "items.hpp"

Factory::Factory() : item_quantities(items.size(), 0) {}

void Factory::draw_items(tcod::Console& console, int x, int y, int w, int h) const {}
void Factory::draw_virt(tcod::Console& console, int x, int y, int w, int h) const {
  tcod::print(console, {x, y}, std::to_string(counter), col::WHITE_BR, std::nullopt);
  int sbw = 14;

  draw_vline(console, x + w - sbw - 1, y, h);
  console[{x + w - sbw - 1, y - 1}].ch = L'┬';
  draw_items(console, x + w - sbw, y, sbw, h);
}

void Factory::process_input_virt(int c) {}

void Factory::tick_virt(double seconds) { counter += seconds; }
