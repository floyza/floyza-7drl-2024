#include "actor.hpp"

#include "colors.hpp"

Actor create_player(int id, Pos p) {
  return Actor{
      .id = id, .p = p, .chr = '@', .color = col::WHITE_BR, .is_player = true, .hp = 20, .max_hp = 20, .atk = 3};
}

Actor generate_monster(int id, Breed b, Pos p) {
  MonsterData mon = {.breed = b};
  return Actor{
      .id = id, .p = p, .chr = b.chr, .color = b.color, .mon = mon, .hp = b.max_hp, .max_hp = b.max_hp, .atk = b.atk};
}
