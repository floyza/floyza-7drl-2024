#ifndef DRAWING_H_
#define DRAWING_H_

#include <libtcod.hpp>

#include "colors.hpp"

// inline void draw_box(
//     TCOD_Console& console,
//     const std::array<int, 4>& rect,
//     std::optional<TCOD_ColorRGB> fg,
//     std::optional<TCOD_ColorRGB> bg,
//     TCOD_bkgnd_flag_t flag = TCOD_BKGND_SET,
//     bool clear = true) {
//   tcod::draw_frame(console, rect, {L'┌', L'─', L'┐', L'│', ' ', L'│', L'└', L'─', L'┘'}, fg, bg, flag, clear);
// }

int wall_merge(int wall1, int wall2);

void draw_box(TCOD_Console& console, const std::array<int, 4>& rect, TCOD_ColorRGB fg = col::WHITE_BR);

#endif  // DRAWING_H_
