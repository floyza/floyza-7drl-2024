#ifndef UTIL_H_
#define UTIL_H_

#include <cctype>
#include <string_view>

inline bool is_vowel(int c) {
  int lc = std::tolower(c);
  switch (lc) {
    case 'a':
    case 'e':
    case 'i':
    case 'o':
    case 'u':
      return true;
  }
  return false;
}

[[noreturn]] void panic(std::string_view message);

void dlog(std::string_view message);

#endif
