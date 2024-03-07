#ifndef COLORS_H_
#define COLORS_H_

#include <libtcod.hpp>

namespace col {
static constexpr auto BLACK = tcod::ColorRGB{0, 0, 0};
static constexpr auto RED = tcod::ColorRGB{0xaa, 0, 0};
static constexpr auto GREEN = tcod::ColorRGB{0, 0xaa, 0};
static constexpr auto YELLOW = tcod::ColorRGB{0xaa, 0xaa, 0};
static constexpr auto BLUE = tcod::ColorRGB{0, 0, 0xaa};
static constexpr auto MAGENTA = tcod::ColorRGB{0xaa, 0, 0xaa};
static constexpr auto CYAN = tcod::ColorRGB{0, 0xaa, 0xaa};
static constexpr auto WHITE = tcod::ColorRGB{0xaa, 0xaa, 0xaa};
static constexpr auto BLACK_BR = tcod::ColorRGB{55, 55, 55};
static constexpr auto RED_BR = tcod::ColorRGB{0xff, 55, 55};
static constexpr auto GREEN_BR = tcod::ColorRGB{55, 0xff, 55};
static constexpr auto YELLOW_BR = tcod::ColorRGB{0xff, 0xff, 55};
static constexpr auto BLUE_BR = tcod::ColorRGB{55, 55, 0xff};
static constexpr auto MAGENTA_BR = tcod::ColorRGB{0xff, 55, 0xff};
static constexpr auto CYAN_BR = tcod::ColorRGB{55, 0xff, 0xff};
static constexpr auto WHITE_BR = tcod::ColorRGB{0xff, 0xff, 0xff};
};  // namespace col

#endif  // COLORS_H_
