#ifndef I3NEOSTATUS_I3BAR_PROTOCOL_HPP
#define I3NEOSTATUS_I3BAR_PROTOCOL_HPP

#include "i3bar_data.hpp"

#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace i3bar_protocol {

namespace json_strings {
namespace header {
static constexpr std::string k_version{"version"};
static constexpr std::string k_stop_signal{"stop_signal"};
static constexpr std::string k_cont_signal{"cont_signal"};
static constexpr std::string k_click_events{"click_events"};
} // namespace header

namespace block {
static constexpr std::string k_name{"name"};
static constexpr std::string k_instance{"instance"};
static constexpr std::string k_full_text{"full_text"};
static constexpr std::string k_short_text{"short_text"};
static constexpr std::string k_color{"color"};
static constexpr std::string k_background{"background"};
static constexpr std::string k_border{"border"};
static constexpr std::string k_border_top{"border_top"};
static constexpr std::string k_border_right{"border_right"};
static constexpr std::string k_border_bottom{"border_bottom"};
static constexpr std::string k_border_left{"border_left"};
static constexpr std::string k_min_width{"min_width"};
static constexpr std::string k_align{"align"};
static constexpr std::string k_urgent{"urgent"};
static constexpr std::string k_separator{"separator"};
static const std::string k_separator_block_width{"separator_block_width"};
static constexpr std::string k_markup{"markup"};
} // namespace block

namespace click_event {
static constexpr std::string k_name{"name"};
static constexpr std::string k_instance{"instance"};
static constexpr std::string k_x{"x"};
static constexpr std::string k_y{"y"};
static constexpr std::string k_button{"button"};
static constexpr std::string k_relative_x{"relative_x"};
static constexpr std::string k_relative_y{"relative_y"};
static constexpr std::string k_output_x{"output_x"};
static constexpr std::string k_output_y{"output_y"};
static constexpr std::string k_width{"width"};
static constexpr std::string k_height{"height"};
static constexpr std::string k_modifiers{"modifiers"};
} // namespace click_event
} // namespace json_strings

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

void print_header(const i3bar_data::header &value,
                  std::ostream &stream = std::cout);
void init_statusline(std::ostream &stream = std::cout);
void print_statusline(const std::vector<i3bar_data::block> &value,
                      bool hide_empty = true, std::ostream &stream = std::cout);
void print_statusline(const std::pair<i3bar_data::block, std::size_t> &value,
                      std::vector<std::string> &cache, bool hide_emtpy = true,
                      std::ostream &stream = std::cout);
void print_statusline(
    const std::vector<std::pair<i3bar_data::block, std::size_t>> &value,
    std::vector<std::string> &cache, bool hide_empty = true,
    std::ostream &stream = std::cout);
void init_click_event(std::istream &input_stream = std::cin);
i3bar_data::click_event read_click_event(std::istream &input_stream = std::cin);

namespace impl {
void print_statusline(const std::vector<std::string> &value,
                      std::ostream &stream = std::cout);

std::string serialize_header(const i3bar_data::header &header);
std::string serialize_block(const i3bar_data::block &block);

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

i3bar_data::click_event parse_click_event(const std::string &click_event);
} // namespace impl
} // namespace i3bar_protocol
#endif
