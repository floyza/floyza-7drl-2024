#ifndef GUI_H_
#define GUI_H_

#include <libtcod/console_types.hpp>

#include "drawing.hpp"

enum Modifiers {
  SHIFT = 1,
  CTRL = 2,
};

class GNode {
  virtual void draw_virt(tcod::Console& /*console*/, int /*x*/, int /*y*/, int /*w*/, int /*h*/) const {};
  virtual void process_input_virt(int /*c*/, uint16_t /*mods*/){};
  virtual void tick_virt(double /*seconds*/){};
  virtual int min_width() const { return 0; }
  virtual int min_height() const { return 0; }

 public:
  virtual ~GNode(){};
  void draw(tcod::Console& console, int x, int y, int w, int h) const;
  void process_input(int c, uint16_t mods) { process_input_virt(c, mods); }
  void tick(double seconds) { tick_virt(seconds); }
};

class GBoxed : public GNode {
  std::unique_ptr<GNode> child;
  void draw_virt(tcod::Console& console, int x, int y, int w, int h) const override;
  int min_width() const override { return 2; }
  int min_height() const override { return 2; }

  void process_input_virt(int c, uint16_t mods) override { child->process_input(c, mods); }
  void tick_virt(double seconds) override { child->tick(seconds); }

 public:
  GBoxed(std::unique_ptr<GNode> child) : child(std::move(child)) {}
  ~GBoxed() override{};
};

class GTabs : public GNode {
  std::vector<std::pair<std::string, std::unique_ptr<GNode>>> tabs;
  int active_tab;

  void draw_virt(tcod::Console& console, int x, int y, int w, int h) const override;
  void process_input_virt(int c, uint16_t mods) override;
  void tick_virt(double seconds) override;

 public:
  GTabs(std::vector<std::pair<std::string, std::unique_ptr<GNode>>> tabs, int active)
      : tabs(std::move(tabs)), active_tab(active) {}
  ~GTabs() override{};
};

#endif  // GUI_H_
