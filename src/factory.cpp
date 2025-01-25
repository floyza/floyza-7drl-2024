#include "factory.hpp"

#include "drawing.hpp"
#include "items.hpp"

using std::optional;

Factory::Factory() : item_quantities(items.size(), 0), lines(4, std::nullopt) {}

void Factory::draw_items(tcod::Console& console, int x, int y, int w, int h) const {
  tcod::print(console, {x + 1, y}, "Stock", col::WHITE_BR, std::nullopt);
  tcod::print(console, {x + 1, y + 1}, std::string(w - 2, '-'), col::WHITE_BR, std::nullopt);
  y += 2;
  for (size_t i = 0; i < items.size(); ++i) {
    const Item& item = items[i];
    tcod::print(
        console, {x + 1, y++}, item.name + " " + std::to_string(item_quantities[i]), col::WHITE_BR, std::nullopt);
  }
}
void Factory::draw_lines(tcod::Console& console, int x, int y, int w, int h) const {
  // probably 75 wide
  constexpr int entry_x = 11;
  int line_width = entry_x * 3 + 3;
  int line_height = 7;
  int line_i = 0;
  for (const optional<DungeonLine>& mbline : lines) {
    int lx = x + (line_i % 2) * (line_width + 1) + 1;
    int ly = y + line_i / 2 * line_height;
    ++line_i;
    draw_box(console, {lx, ly, line_width, line_height});
    if (!mbline) {
      // tcod::print(console, {lx + 2, ly}, "Empty", col::WHITE_BR, std::nullopt);
      continue;
    }
    const DungeonLine& line = *mbline;
    tcod::print(console, {lx + 2, ly}, "Assembly", col::WHITE_BR, std::nullopt);
    for (int iqx = 0; iqx < 3; ++iqx) {
      for (int iqy = 0; iqy < 3; ++iqy) {
        int i = iqx * 3 + iqy;
        int quantity = line.item_quantities[i];
        const Item& item = items[i];
        tcod::print(console, {lx + 2 + iqx * entry_x, ly + 1 + iqy}, item.name, col::WHITE_BR, std::nullopt);
        draw_d(console, lx + 2 + iqx * entry_x + 6, ly + 1 + iqy, quantity);
      }
    }
    tcod::print(
        console,
        {lx + line_width / 2, ly + 4},
        "PER " + std::to_string(line.turn_amount),
        col::WHITE,
        std::nullopt,
        TCOD_CENTER);
    double progress = static_cast<double>(line.progress) / static_cast<double>(line.turn_amount);
    int progress_filled = static_cast<double>(line_width - 4) * progress;
    tcod::print(console, {lx + 2, ly + 5}, std::string(progress_filled, '='), col::GREEN, std::nullopt);
    tcod::print(
        console,
        {lx + 2 + progress_filled, ly + 5},
        std::string(line_width - 4 - progress_filled, '-'),  // crashes here
        col::WHITE,
        std::nullopt);
  }
}

void Factory::draw_virt(tcod::Console& console, int x, int y, int w, int h) const {
  int sbw = 14;

  draw_vline(console, x + w - sbw - 1, y, h);
  console[{x + w - sbw - 1, y - 1}].ch = L'┬';
  draw_items(console, x + w - sbw, y, sbw, h);
  draw_lines(console, x, y, w - sbw - 1, h);
}

void Factory::process_input_virt(int c, uint16_t mods) {}

void Factory::tick_virt(double seconds) {
  for (optional<DungeonLine>& mbline : lines) {
    if (!mbline) {
      continue;
    }
    DungeonLine& line = *mbline;
    if (line.started) {
      line.progress++;
    } else {
      bool enough_stuff = true;
      for (size_t i = 0; i < item_quantities.size(); ++i) {
        if (line.item_quantities[i] < 0 && (item_quantities[i] + line.item_quantities[i]) < 0) {
          enough_stuff = false;
          break;
        }
      }
      if (enough_stuff) {
        line.started = true;
        line.progress++;
        for (size_t i = 0; i < item_quantities.size(); ++i) {
          if (line.item_quantities[i] < 0) {
            item_quantities[i] += line.item_quantities[i];
          }
        }
      }
    }
    if (line.progress >= line.turn_amount) {
      line.started = false;
      line.progress = 0;
      for (size_t i = 0; i < item_quantities.size(); ++i) {
        if (line.item_quantities[i] > 0) {
          item_quantities[i] += line.item_quantities[i];
        }
      }
    }
  }
  counter += seconds;
}
