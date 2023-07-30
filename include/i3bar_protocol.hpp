#ifndef I3BAR_PROTOCOL_HPP
#define I3BAR_PROTOCOL_HPP

#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace i3bar_protocol {
using pixel_count_t = long;

struct header {
  int version;
  std::optional<int> stop_signal;
  std::optional<int> count_signal;
  std::optional<bool> click_event_events;
};

struct block {
  std::string full_text;
  std::optional<std::string> short_text;
  std::optional<std::string> color;
  std::optional<std::string> background;
  std::optional<std::string> border;
  std::optional<pixel_count_t> border_top;
  std::optional<pixel_count_t> border_right;
  std::optional<pixel_count_t> border_bottom;
  std::optional<pixel_count_t> border_left;
  std::optional<std::variant<pixel_count_t, std::string>> min_width;
  std::optional<std::string> align;
  std::optional<std::string> name;
  std::optional<std::string> instance;
  std::optional<bool> urgent;
  std::optional<bool> separator;
  std::optional<pixel_count_t> separator_block_width;
  std::optional<std::string> markup;
};

struct click_event {
  std::string name;
  std::string instance;
  pixel_count_t x;
  pixel_count_t y;
  int button;
  pixel_count_t relative_x;
  pixel_count_t relative_y;
  pixel_count_t output_x;
  pixel_count_t output_y;
  pixel_count_t width;
  pixel_count_t height;
  std::vector<std::string> modifiers;
};

std::string generate_header(const header &output);
std::string generate_block(const block &output);
click_event parse_click_event(const std::string &input);

template <typename t_value>
std::string generate_json_key_value(std::pair<std::string, t_value> &output);
template <typename t_value>
std::pair<std::string, t_value> parse_json_key_value(const std::string &input);
}; // namespace i3bar_protocol

#endif
