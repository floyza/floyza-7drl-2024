#ifndef DIR_H_
#define DIR_H_

#include "pos.hpp"

namespace Dir {
constexpr Pos n{0, -1};
constexpr Pos s{0, 1};
constexpr Pos w{-1, 0};
constexpr Pos e{1, 0};
constexpr Pos nw{-1, -1};
constexpr Pos ne{1, -1};
constexpr Pos sw{-1, 1};
constexpr Pos se{1, 1};
}; // namespace Dir

#endif // DIR_H_
