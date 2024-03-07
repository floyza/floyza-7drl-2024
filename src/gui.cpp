#include "gui.hpp"

#include "drawing.hpp"

GNode::~GNode() {}

void GNode::draw(tcod::Console& console, int x, int y, int w, int h) const {
  if (w > min_width() && h > min_height()) {
    draw_virt(console, x, y, w, h);
  }
}

void GBoxed::draw_virt(tcod::Console& console, int x, int y, int w, int h) const {
  draw_box(console, {x, y, w, h});
  child->draw(console, x + 1, y + 1, w - 2, h - 2);
}
