#include "drawing.hpp"
#include <cassert>

struct Wall_tile {
  bool n, s, w, e;
  bool operator==(const Wall_tile &o) const {
    return n == o.n && s == o.s && e == o.e && w == o.w;
  }
  Wall_tile &operator+=(const Wall_tile &o) {
    n += o.n;
    s += o.s;
    e += o.e;
    w += o.w;
    return *this;
  }
};

int wall_merge(int wall1, int wall2) {
  std::array<int, 12> wall_chars{
      ' ',
      L'─',
      L'│',
      L'┐',
      L'┌',
      L'┘',
      L'└',
      L'┤',
      L'├',
      L'┴',
      L'┬',
      L'┼',
  };
  std::array<Wall_tile, 12> wall_data{
      Wall_tile{0, 0, 0, 0}, Wall_tile{0, 0, 1, 1}, Wall_tile{1, 1, 0, 0},
      Wall_tile{0, 1, 1, 0}, Wall_tile{0, 1, 0, 1}, Wall_tile{1, 0, 1, 0},
      Wall_tile{1, 0, 0, 1}, Wall_tile{1, 1, 1, 0}, Wall_tile{1, 1, 0, 1},
      Wall_tile{1, 0, 1, 1}, Wall_tile{0, 1, 1, 1}, Wall_tile{1, 1, 1, 1}};
  auto wall1_it = std::find(wall_chars.begin(), wall_chars.end(), wall1);
  auto wall2_it = std::find(wall_chars.begin(), wall_chars.end(), wall2);
  assert(wall1_it != wall_chars.end() ||
         wall2_it != wall_chars.end()); // why did you even call it?
  if (wall1_it == wall_chars.end())
    return wall2;
  if (wall2_it == wall_chars.end())
    return wall1;
  Wall_tile result = wall_data[wall1_it - wall_chars.begin()];
  result += wall_data[wall2_it - wall_chars.begin()];
  return wall_chars[std::find(wall_data.data(), wall_data.end(), result) -
                    wall_data.begin()];
}

void draw_box(TCOD_Console &console, const std::array<int, 4> &rect, TCOD_ColorRGB fg) {
  const int width = rect[2];
  const int height = rect[3];
  const int x = rect[0];
  const int y = rect[1];

  for (int xo = 1; xo < width - 1; ++xo) {
    console[{x+xo,y}].ch = wall_merge(L'─', console[{x+xo,y}].ch);
    console[{x+xo,y}].fg = fg;
    console[{x+xo,y+height-1}].ch = wall_merge(L'─', console[{x+xo,y+height-1}].ch);
    console[{x+xo,y+height-1}].fg = fg;
  }
  for (int yo = 1; yo < height - 1; ++yo) {
    console[{x,y+yo}].ch = wall_merge(L'│', console[{x,y+yo}].ch);
    console[{x,y+yo}].fg = fg;
    console[{x+width-1,y+yo}].ch = wall_merge(L'│', console[{x+width-1,y+yo}].ch);
    console[{x+width-1,y+yo}].fg = fg;
  }
  console[{x,y}].ch = wall_merge(L'┌', console[{x,y}].ch);
  console[{x,y}].fg = fg;
  console[{x+width-1,y}].ch = wall_merge(L'┐', console[{x+width-1,y}].ch);
  console[{x+width-1,y}].fg = fg;
  console[{x,y+height-1}].ch = wall_merge(L'└', console[{x,y+height-1}].ch);
  console[{x,y+height-1}].fg = fg;
  console[{x+width-1,y+height-1}].ch = wall_merge(L'┘', console[{x+width-1,y+height-1}].ch);
  console[{x+width-1,y+height-1}].fg = fg;
}
