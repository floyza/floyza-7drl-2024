#include "items.hpp"

#include "map.hpp"

void healing(Map& map) {
  Actor& player = *map.get_player();
  player.hp = std::min(player.hp + 10, player.max_hp);
}

void aoe_hit(Map& map) {
  for (auto& [_, a] : map.actors) {
    if (!a.is_player && map.in_fov(a.p)) {
      a.hp -= 5;
    }
  }
}

void shoot(Map& map, ActorSS i) {
  auto actor = map.get_actor(i.id);
  if (!actor) {
    return;
  }
  (*actor)->hp -= 20;
}

const std::vector<Item> items{
    Item{.name = "HPPot", .desc = "Heal 10 hp", .chr = '!', .color = col::BLUE_BR, .action = healing},
    Item{.name = "Shoot", .desc = "Shoot an enemy for 20 damage", .chr = '/', .color = col::WHITE_BR, .action = shoot},
    Item{
        .name = "Incin",
        .desc = "Hit all enemies in fov for 5 damage",
        .chr = '=',
        .color = col::RED_BR,
        .action = aoe_hit},
};
