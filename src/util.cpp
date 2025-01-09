#include "util.hpp"

#include <iostream>

[[noreturn]] void panic(std::string_view message) {
  std::cerr << "PANIC!!! " << message << std::endl;
  std::exit(-1);
}

void dlog(std::string_view message) { std::cerr << "LOG: " << message << std::endl; }
