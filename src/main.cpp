#include "colors.hpp"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif  // __EMSCRIPTEN__
#include <SDL.h>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <libtcod.hpp>

#include "factory.hpp"
#include "map.hpp"

using std::chrono::duration_cast;
using std::chrono::steady_clock;
using std::chrono::time_point;

using seconds = std::chrono::duration<double>;

#if defined(_MSC_VER)
#pragma warning(disable : 4297)  // Allow "throw" in main().  Letting the
                                 // compiler handle termination.
#endif

/// Return the data directory.
auto get_data_dir() -> std::filesystem::path {
  static auto root_directory = std::filesystem::path{"."};  // Begin at the working directory.
  while (!std::filesystem::exists(root_directory / "data")) {
    // If the current working directory is missing the data dir then it will
    // assume it exists in any parent directory.
    root_directory /= "..";
    if (!std::filesystem::exists(root_directory)) {
      throw std::runtime_error("Could not find the data directory.");
    }
  }
  return root_directory / "data";
};

static tcod::Console g_console;  // The global console object.
static tcod::Context g_context;  // The global libtcod context.
static std::unique_ptr<GNode> g_root;
static std::optional<time_point<steady_clock>> frame_time;

/// Game loop.
void main_loop() {
  // Rendering.
  g_console.clear();
  g_root->draw(g_console, 0, 0, g_console.get_width(), g_console.get_height());
  g_context.present(g_console);

  // Handle input.
  SDL_Event event;
#ifndef __EMSCRIPTEN__
  // Block until events exist.  This conserves resources well but isn't
  // compatible with animations or Emscripten.
  // SDL_WaitEvent(nullptr);
#endif
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        std::exit(EXIT_SUCCESS);
        break;
      case SDL_KEYDOWN: {
        int ev = event.key.keysym.sym;
        if (event.key.keysym.mod & KMOD_SHIFT) {
          ev = std::toupper(ev);
        }
        g_root->process_input(ev);
        break;
      }
    }
  }
  auto now = steady_clock::now();
  if (!frame_time) {
    *frame_time = now;
  }
  g_root->tick(duration_cast<seconds>(now - *frame_time).count());
  frame_time = steady_clock::now();
}

/// Main program entry point.
int main(int argc, char** argv) {
  {
    auto params = TCOD_ContextParams{};
    params.tcod_version = TCOD_COMPILEDVERSION;
    params.argc = argc;
    params.argv = argv;
    params.renderer_type = TCOD_RENDERER_SDL2;
    params.vsync = 1;
    params.sdl_window_flags = SDL_WINDOW_RESIZABLE;
    params.window_title = "Dimensional Dungeons Inc";

    auto tileset = tcod::load_tilesheet(get_data_dir() / "cp866_8x12.png", {16, 16}, tcod::CHARMAP_CP437);
    params.tileset = tileset.get();

    g_console = tcod::Console{90, 40};
    params.console = g_console.get();

    g_context = tcod::Context(params);

    std::unique_ptr<GNode> map = std::make_unique<Map>(80, 40);

    std::vector<std::pair<std::string, std::unique_ptr<GNode>>> tabs;
    tabs.push_back(std::make_pair(std::string("Factory"), std::make_unique<Factory>()));
    tabs.push_back(std::make_pair(std::string("Dungeon"), std::move(map)));
    std::unique_ptr<GNode> root = std::make_unique<GTabs>(std::move(tabs), 1);
    g_root = std::move(root);

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, 0);
#else
    while (true) main_loop();
#endif
  }
  // catch (const std::exception& exc) {
  //   std::cerr << exc.what() << "\n";
  //   throw;
  // }
}
