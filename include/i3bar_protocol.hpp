#ifndef I3BAR_PROTOCOL_HPP
#define I3BAR_PROTOCOL_HPP

#include "module_id.hpp"

#include <concepts>
#include <iostream>
#include <optional>
#include <string>
#include <utility>
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
  struct id {
    std::optional<std::string> name;
    std::optional<module_id::type> instance;

    static constexpr std::string k_name_str{"name"};
    static constexpr std::string k_instance_str{"instance"};
  };
  using struct_id = struct id;

  struct content {
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
    static constexpr std::string k_urgent_str{"urgent"};
    static constexpr std::string k_separator_str{"separator"};
    static const std::string k_separator_block_width_str;
    static constexpr std::string k_markup_str{"markup"};
  };
  using struct_content = struct content;

  id id;
  content content;
};

struct click_event {
  struct id {
    std::string name;
    module_id::type instance;

    static constexpr std::string k_name_str{"name"};
    static constexpr std::string k_instance_str{"instance"};
  };
  using struct_id = struct id;

  struct content {
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
  using struct_content = struct content;

  struct id id;
  struct content content;
};

namespace json_constants {
static constexpr char g_k_newline{'\n'};
static constexpr char g_k_space{' '};
static constexpr char g_k_escape_leader{'\\'};
static constexpr char g_k_name_value_separator{':'};
static constexpr char g_k_element_separator{','};
static constexpr char g_k_string_delimiter{'"'};
static constexpr char g_k_object_opening_delimiter{'{'};
static constexpr char g_k_object_closing_delimiter{'}'};
static constexpr char g_k_array_opening_delimiter{'['};
static constexpr char g_k_array_closing_delimiter{']'};
} // namespace json_constants

void print_header(const header &value, std::ostream &stream = std::cout);
void init_statusline(std::ostream &stream = std::cout);
void print_statusline(const std::vector<block> &value, bool hide_empty = true,
                      std::ostream &stream = std::cout);
void print_statusline(const std::pair<block, std::size_t> &value,
                      std::vector<std::string> &cache, bool hide_emtpy = true,
                      std::ostream &stream = std::cout);
void print_statusline(const std::vector<std::pair<block, std::size_t>> &value,
                      std::vector<std::string> &cache, bool hide_empty = true,
                      std::ostream &stream = std::cout);
void init_click_event(std::istream &input_stream = std::cin);
click_event read_click_event(std::istream &input_stream = std::cin);

namespace impl {
void print_statusline(const std::vector<std::string> &value,
                      std::ostream &stream = std::cout);

std::string serialize_header(const header &header);
std::string serialize_block(const block &block);

std::string
serialize_name_value(const std::pair<std::string, std::string> &name_value);
std::string serialize_object(
    const std::vector<std::pair<std::string, std::string>> &object);
std::string serialize_array(const std::vector<std::string> &array);
std::string serialize_number(auto number)
  requires(std::integral<decltype(number)> ||
           std::floating_point<decltype(number)>)
{
  return std::to_string(number);
}
std::string serialize_string(const std::string &string);
std::string serialize_bool(const bool b);

click_event parse_click_event(const std::string &click_event);
} // namespace impl
} // namespace i3bar_protocol
#endif
