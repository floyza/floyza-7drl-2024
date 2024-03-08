#include "map.hpp"

void healing(Map& map) {
  Actor& player = *map.get_player();
  player.hp = std::min(player.hp + 10, player.max_hp);
}

void aoe_hit(Map& map) {
  for (auto& [_, a] : map.actors) {
    if (!a.is_player) {
      a.hp -= 5;
    }
  }
}

// need to have targeting for this
// void shoot
