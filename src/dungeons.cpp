#include "dungeons.hpp"

#include <SDL_keycode.h>

#include "colors.hpp"

Dungeons::Dungeons() {}

void Dungeons::draw_virt(tcod::Console& console, int x, int y, int w, int h) const {
  if (map) {
    map->draw(console, x, y, w, h);
  } else {
    tcod::print(console, {x, y}, "Press enter to start dungeon", col::WHITE_BR, std::nullopt);
  }
}

void Dungeons::process_input_virt(int c, uint16_t mods) {
  if (map) {
    map->process_input(c, mods);
  } else {
    if (c == SDLK_RETURN) {
      map = std::make_unique<Map>(this, 80, 40);
    }
  }
}
