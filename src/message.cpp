#include "message.hpp"

#include <algorithm>

Messages::Messages() {}

void Messages::add_message(std::string message) { messages.push_back(std::make_pair(turn_number, std::move(message))); }

std::vector<std::string> Messages::current_messages() const {
  auto not_our_turn = std::find_if_not(
      messages.rbegin(), messages.rend(), [=, this](auto& msg) { return msg.first == turn_number - 1; });
  std::vector<std::string> ret;
  for (auto it = not_our_turn.base(); it != messages.end(); ++it) {
    ret.push_back(it->second);
  }
  return ret;
}

void Messages::next_turn() { ++turn_number; }
