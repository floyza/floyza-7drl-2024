#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <libtcod/console_types.hpp>
#include <vector>

class Messages {
  // list of (turn number, message) pairs
  std::vector<std::pair<int, std::string>> messages;
  int turn_number = 0;

 public:
  Messages();
  void add_message(std::string message);
  std::vector<std::string> current_messages() const;
  void next_turn();
};

#endif  // MESSAGE_H_
