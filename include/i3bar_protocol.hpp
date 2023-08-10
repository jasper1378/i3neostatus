#ifndef I3BAR_PROTOCOL_HPP
#define I3BAR_PROTOCOL_HPP

#include <boost/json.hpp>

#include <iostream>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace i3bar_protocol {
using pixel_count_t = long;

struct header {
  int version;
  std::optional<int> stop_signal;
  std::optional<int> cont_signal;
  std::optional<bool> click_events;

  static constexpr std::string k_version_str{"version"};
  static constexpr std::string k_stop_signal_str{"stop_signal"};
  static constexpr std::string k_cont_signal_str{"cont_signal"};
  static constexpr std::string k_click_events_str{"click_events"};
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

  static constexpr std::string k_full_text_str{"full_text"};
  static constexpr std::string k_short_text_str{"short_text"};
  static constexpr std::string k_color_str{"color"};
  static constexpr std::string k_background_str{"background"};
  static constexpr std::string k_border_str{"border"};
  static constexpr std::string k_border_top_str{"border_top"};
  static constexpr std::string k_border_right_str{"border_right"};
  static constexpr std::string k_border_bottom_str{"border_bottom"};
  static constexpr std::string k_border_left_str{"border_left"};
  static constexpr std::string k_min_width_str{"min_width"};
  static constexpr std::string k_align_str{"align"};
  static constexpr std::string k_name_str{"name"};
  static constexpr std::string k_instance_str{"instance"};
  static constexpr std::string k_urgent_str{"urgent"};
  static constexpr std::string k_separator_str{"separator"};
  static const std::string k_separator_block_width_str;
  static constexpr std::string k_markup_str{"markup"};
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

  static constexpr std::string k_name_str{"name"};
  static constexpr std::string k_instance_str{"instance"};
  static constexpr std::string k_x_str{"x"};
  static constexpr std::string k_y_str{"y"};
  static constexpr std::string k_button_str{"button"};
  static constexpr std::string k_relative_x_str{"relative_x"};
  static constexpr std::string k_relative_y_str{"relative_y"};
  static constexpr std::string k_output_x_str{"output_x"};
  static constexpr std::string k_output_y_str{"output_y"};
  static constexpr std::string k_width_str{"width"};
  static constexpr std::string k_height_str{"height"};
  static constexpr std::string k_modifiers_str{"modifiers"};
};

static constexpr char g_k_newline_char{'\n'};
static constexpr char g_k_space_char{' '};
static constexpr char g_k_json_array_opening_delimiter{'['};
static constexpr char g_k_json_array_closing_delimiter{']'};
static constexpr char g_k_json_array_element_separator{','};

void print_header(const header &output_value,
                  std::ostream &ouput_stream = std::cout);
void print_array_start(std::ostream &output_stream = std::cout);
void print_statusline(const std::vector<block> &output_value,
                      std::ostream &output_stream = std::cout);
void read_array_start(std::istream &input_stream = std::cin);
click_event read_click_event(std::istream &input_stream = std::cin);
}; // namespace i3bar_protocol

namespace boost {
namespace json {
void tag_invoke(const value_from_tag &, value &bj_value,
                const i3bar_protocol::header &header);
void tag_invoke(const value_from_tag &, value &bj_value,
                i3bar_protocol::header &&header);
void tag_invoke(const value_from_tag &, value &bj_value,
                const i3bar_protocol::block &block);
void tag_invoke(const value_from_tag &, value &bj_value,
                i3bar_protocol::block &&block);
i3bar_protocol::click_event
tag_invoke(const value_to_tag<i3bar_protocol::click_event> &,
           const value &bj_value);
}; // namespace json
}; // namespace boost

#endif
