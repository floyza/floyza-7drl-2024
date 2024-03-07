#include "rand.hpp"

#include <random>
#include <stdexcept>

#include "dir.hpp"

std::random_device dev{};
std::mt19937 rng{dev()};

int rand_int(int low, int high) {
  std::uniform_int_distribution<std::mt19937::result_type> dist(low, high);
  return dist(rng);
}

bool percent_chance(double chance) {
  std::binomial_distribution<std::mt19937::result_type> dist(1, chance / 100);
  return dist(rng);
}

Pos rand_dir_orth() {
  switch (rand_int(0, 3)) {
    case 0:
      return Dir::n;
    case 1:
      return Dir::s;
    case 2:
      return Dir::w;
    case 3:
      return Dir::e;
  }
  throw std::logic_error("idk man");
}

Pos rand_dir() {
  switch (rand_int(0, 7)) {
    case 0:
      return Dir::n;
    case 1:
      return Dir::s;
    case 2:
      return Dir::w;
    case 3:
      return Dir::e;
    case 4:
      return Dir::nw;
    case 5:
      return Dir::ne;
    case 6:
      return Dir::sw;
    case 7:
      return Dir::se;
  }
  throw std::logic_error("idk man");
}
