#include "items.hpp"

#include "map.hpp"

void healing(Map& map) {
  Actor& player = *map.get_player();
  int nhp = std::min(player.hp + 10, player.max_hp);
  map.add_message("You heal " + std::to_string(nhp) + " hp.");
  player.hp = nhp;
}

void aoe_hit(Map& map) {
  for (auto& [_, a] : map.actors) {
    if (!a.is_player && map.in_fov(a.p)) {
      a.hp -= 5;
    }
  }
  map.add_message("You blast all nearby enemies for 5 damage.");
}

void shoot(Map& map, ActorSS i) {
  auto actor = map.get_actor(i.id);
  if (!actor) {
    return;
  }
  (*actor)->hp -= 15;
  map.add_message("You hit the " + (*actor)->mon->breed.name + " for 15 damage.");
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
