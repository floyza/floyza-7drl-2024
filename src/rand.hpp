#ifndef RAND_H_
#define RAND_H_

#include "pos.hpp"

// inclusive
int rand_int(int low, int high);

bool percent_chance(double chance);

Pos rand_dir_orth();

Pos rand_dir();

#endif  // RAND_H_
