#include "breeds.hpp"

#include "actor_ai.hpp"
#include "colors.hpp"

const std::vector<Breed> breeds{
    Breed{.name = "goblin", .chr = 'g', .color = col::BLUE_BR, .max_hp = 5, .atk = 3, .ai = monster_act},
    Breed{.name = "kobold", .chr = 'k', .color = col::GREEN, .max_hp = 4, .atk = 2, .ai = monster_act}};
