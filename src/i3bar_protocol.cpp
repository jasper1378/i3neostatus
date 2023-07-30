#include "i3bar_protocol.hpp"

#include <optional>
#include <string>
#include <variant>
#include <vector>

std::string i3bar_protocol::generate_header(const header &output) {
  // TODO
}

std::string i3bar_protocol::generate_block(const block &output) {
  // TODO
}

i3bar_protocol::click_event parse_click_event(const std::string &input) {
  // TODO
}

template <typename t_value>
std::string generate_json_key_value(std::pair<std::string, t_value> &output) {
  // TODO
}

template <typename t_value>
std::pair<std::string, t_value> parse_json_key_value(const std::string &input) {
  // TODO
}
