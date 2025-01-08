#include "gui.hpp"

#include "drawing.hpp"

void GNode::draw(tcod::Console& console, int x, int y, int w, int h) const {
  if (w > min_width() && h > min_height()) {
    draw_virt(console, x, y, w, h);
  }
}

void GBoxed::draw_virt(tcod::Console& console, int x, int y, int w, int h) const {
  draw_box(console, {x, y, w, h});
  child->draw(console, x + 1, y + 1, w - 2, h - 2);
}

void GTabs::draw_virt(tcod::Console& console, int x, int y, int w, int h) const {
  draw_hline(console, x, y + 3, w);
  for (size_t i = 0; i < tabs.size(); ++i) {
    int tabwidth = w / tabs.size();
    int basex = tabwidth * i;
    draw_vline(console, basex, y, 4);
    console[{x + basex, y + 3}].ch = L'┴';
    if (static_cast<int>(i) == active_tab) {
      tcod::draw_rect(console, {x + basex + 1, y, tabwidth - 1, 3}, '%', col::GREEN, std::nullopt);
    }
    tcod::print(console, {x + basex + 2, y + 1}, tabs[i].first, col::WHITE_BR, std::nullopt);
  }
  tabs[active_tab].second->draw(console, x, y + 4, w, h - 4);
}

void GTabs::process_input_virt(int c, uint16_t mods) {
  if (c == '\t') {
    active_tab += 1;
    active_tab %= tabs.size();
  } else {
    tabs[active_tab].second->process_input(c, mods);
  }
}

void GTabs::tick_virt(double seconds) {
  for (auto& [_, node] : tabs) {
    node->tick(seconds);
  }
}
