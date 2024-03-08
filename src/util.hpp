#ifndef UTIL_H_
#define UTIL_H_

#include <cctype>

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

#endif
