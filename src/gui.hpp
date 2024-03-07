#ifndef GUI_H_
#define GUI_H_

#include <libtcod/console_types.hpp>

#include "drawing.hpp"

class GNode {
  virtual void draw_virt(tcod::Console& console, int x, int y, int w, int h) const = 0;
  virtual void process_input_virt(int /*c*/){};
  virtual void tick_virt(double /*seconds*/){};
  virtual int min_width() const { return 0; }
  virtual int min_height() const { return 0; }

 public:
  virtual ~GNode() = 0;
  void draw(tcod::Console& console, int x, int y, int w, int h) const;
  void process_input(int c) { process_input_virt(c); }
  void tick(double seconds) { tick_virt(seconds); }
};

class GBoxed : public GNode {
  std::unique_ptr<GNode> child;
  void draw_virt(tcod::Console& console, int x, int y, int w, int h) const override;
  int min_width() const override { return 2; }
  int min_height() const override { return 2; }

  void process_input_virt(int c) override { child->process_input(c); }
  void tick_virt(double seconds) override { child->tick(seconds); }

 public:
  GBoxed(std::unique_ptr<GNode> child) : child(std::move(child)) {}
  ~GBoxed() override{};
};

#endif  // GUI_H_
