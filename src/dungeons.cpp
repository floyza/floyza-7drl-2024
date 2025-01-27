#include "dungeons.hpp"

#include <SDL_keycode.h>

#include "colors.hpp"
#include "drawing.hpp"

template <class... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

Dungeons::Dungeons() {}

void Dungeons::draw_virt(tcod::Console& console, int x, int y, int w, int h) const {
  std::visit(
      overloaded{
          [&](std::monostate) {
            tcod::print(console, {x + 1, y}, "Press enter to start dungeon", col::WHITE_BR, std::nullopt);
          },
          [&](const std::unique_ptr<Map>& map) { map->draw(console, x, y, w, h); },
          [&](const MapResults& results) {
            tcod::print(
                console,
                {x + 1, y},
                "Go over to the Factory to apply, or press (D) to discard.",
                col::WHITE_BR,
                std::nullopt);
            // copied from draw_usables with modifications
            int starting_y = y;
            y += 2;
            tcod::print(console, {x + 1, y++}, "Returns", col::WHITE_BR, std::nullopt);
            tcod::print(console, {x + 1, y++}, std::string(12, '-'), col::WHITE_BR, std::nullopt);
            for (int i = 0; i < static_cast<int>(items.size()); ++i) {
              std::string desc = std::to_string(i + 1) + ")" + items[i].name;
              tcod::print(console, {x + 1, y}, desc, col::WHITE_BR, std::nullopt);
              draw_d(console, x + static_cast<int>(desc.length()) + 2, y, results.item_quantities[i]);
              ++y;
            }
            tcod::print(
                console,
                {x + 1, starting_y + h - 2},
                "TC: " + std::to_string(results.turn_amount),
                col::WHITE_BR,
                std::nullopt);
          }},
      state);
}

void Dungeons::process_input_virt(int c, uint16_t mods) {
  std::visit(
      overloaded{
          [&](std::monostate) {
            if (c == SDLK_RETURN) {
              state = std::make_unique<Map>(this, 80, 40);
              left_dungeon = false;
            }
          },
          [&](std::unique_ptr<Map>& map) {
            map->process_input(c, mods);
            if (left_dungeon) {
              state = MapResults{.item_quantities = map->get_item_quantities(), .turn_amount = map->get_turn_count()};
            }
          },
          [&](const MapResults&) {
            if (c == 'D') {
              state = std::monostate();
            }
          }},
      state);
}
