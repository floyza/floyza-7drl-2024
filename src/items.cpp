#include "items.hpp"

#include "colors.hpp"
#include "map.hpp"

bool is_smite(const ItemFV& f) { return std::holds_alternative<ItemF<EmptyPos>>(f); }

void healing(Map& map) {
  Actor& player = map.get_player();
  int nhp = std::min(player.hp + 10, player.max_hp);
  map.add_message("You heal " + std::to_string(nhp) + " hp.");
  player.hp = nhp;
}

void aoe_hit(Map& map) {
  int effected = 0;
  for (auto& [_, a] : map.actors) {
    if (!a.is_player && map.in_fov(a.p)) {
      a.hp -= 5;
      effected++;
    }
  }
  if (effected == 0) {
    map.add_message("There are no nearby enemies to blast. Wasted :(");
  } else if (effected == 1) {
    map.add_message("You blast an enemy for 5 damage.");
  } else {
    int marks = 1;
    if (effected > 3) {
      marks++;
    }
    if (effected > 6) {
      marks++;
    }
    if (effected > 9) {
      marks++;
    }
    map.add_message("You blast " + std::to_string(effected) + " enemies for 5 damage each" + std::string(marks, '!'));
  }
}

void shoot(Map& map, ActorSS i) {
  auto actor = map.get_actor(i.id);
  if (!actor) {
    return;
  }
  (*actor)->hp -= 15;
  map.add_message("You hit the " + (*actor)->mon->breed.name + " for 15 damage.");
}

void fireball(Map& map, PosSS i) {
  for (auto& [_, a] : map.actors) {
    Pos dist = a.p - i.pt;
    if (abs(dist.x) <= 1 && abs(dist.y) <= 1) {
      a.hp -= 15;
      if (a.is_player) {
        map.add_message("You get blased by your own fireball for 15 damage!");
      }
    }
  }
}

void controlled_blink(Map& map, EmptyPos pos) { map.teleport_player(pos.pt); }

const std::vector<Item> items{
    Item{.name = "HPPot", .desc = "Heal 10 hp", .chr = '!', .color = col::BLUE_BR, .action = healing},
    Item{.name = "Shoot", .desc = "Shoot an enemy for 20 damage", .chr = '/', .color = col::WHITE_BR, .action = shoot},
    Item{
        .name = "Incin",
        .desc = "Hit all enemies in fov for 5 damage",
        .chr = '=',
        .color = col::RED_BR,
        .action = aoe_hit},
    Item{
        .name = "FireB",
        .desc = "3x3 fireball dealing 15 damage to ALL units in area",
        .chr = '*',
        .color = col::RED_BR,
        .action = fireball,
        .drawing_flags = FIREBALL_DRAW},
    Item{
        .name = "CBlnk",
        .desc = "Controlled teleport to any unoccupied square",
        .chr = '?',
        .color = col::YELLOW_BR,
        .action = controlled_blink},
};
