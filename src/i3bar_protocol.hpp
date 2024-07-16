#ifndef I3NEOSTATUS_I3BAR_PROTOCOL_HPP
#define I3NEOSTATUS_I3BAR_PROTOCOL_HPP

#include "i3bar_data.hpp"

#include "bits-and-bytes/stream_append.hpp"

#include <concepts>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace i3neostatus {

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
static constexpr char k_newline{'\n'};
static constexpr char k_space{' '};
static constexpr char k_escape_leader{'\\'};
static constexpr char k_name_value_separator{':'};
static constexpr char k_element_separator{','};
static constexpr char k_string_delimiter{'"'};
static constexpr char k_object_opening_delimiter{'{'};
static constexpr char k_object_closing_delimiter{'}'};
static constexpr char k_array_opening_delimiter{'['};
static constexpr char k_array_closing_delimiter{']'};
} // namespace json_constants

void print_header(const i3bar_data::header &value,
                  std::ostream &stream = std::cout);
void init_statusline(std::ostream &stream = std::cout);

void print_statusline(const std::vector<struct i3bar_data::block> &content,
                      const bool hide_empty, std::ostream &stream = std::cout);
void print_statusline(const std::vector<struct i3bar_data::block> &content,
                      const std::vector<i3bar_data::block> &separators,
                      const bool hide_empty, std::ostream &stream = std::cout);

void print_statusline(const struct i3bar_data::block &content,
                      const module_id::type content_index,
                      std::vector<std::string> &content_cache,
                      const bool hide_empty, std::ostream &stream = std::cout);
void print_statusline(const struct i3bar_data::block &content,
                      const module_id::type content_index,
                      std::vector<std::string> &content_cache,
                      const i3bar_data::block &separator_left,
                      const module_id::type separator_left_index,
                      const i3bar_data::block &separator_right,
                      const module_id::type separator_right_index,
                      std::vector<std::string> &separator_cache,
                      const bool hide_empty, std::ostream &stream = std::cout);

void print_statusline(const std::vector<struct i3bar_data::block> &content,
                      std::vector<std::string> &content_cache,
                      const bool hide_empty, std::ostream &stream = std::cout);
void print_statusline(const std::vector<struct i3bar_data::block> &content,
                      std::vector<std::string> &content_cache,
                      const std::vector<i3bar_data::block> &separators,
                      std::vector<std::string> &separator_cache,
                      const bool hide_empty, std::ostream &stream = std::cout);

void init_click_event(std::istream &input_stream = std::cin);
i3bar_data::click_event read_click_event(std::istream &input_stream = std::cin);

namespace impl {
void print_statusline(const std::vector<std::string> &content,
                      const bool hide_empty, std::ostream &stream = std::cout);
void print_statusline(const std::vector<std::string> &content,
                      const std::vector<std::string> &separators,
                      const bool hide_empty, std::ostream &stream = std::cout);

template <typename t_output>
t_output &serialize_header(t_output &output, const i3bar_data::header &header);
template <typename t_output>
t_output &serialize_block(t_output &output,
                          const struct i3bar_data::block &block,
                          const bool hide_empty);
std::vector<std::string>
serialize_blocks(const std::vector<struct i3bar_data::block> &blocks,
                 const bool hide_empty);

template <typename t_output>
t_output &serialize_name_value(t_output &output, const std::string &name,
                               const std::string &value);
template <typename t_output>
t_output &serialize_object(
    t_output &output,
    const std::vector<std::pair<std::string, std::string>> &object);
template <typename t_output>
t_output &serialize_array(t_output &output,
                          const std::vector<std::string> &array,
                          const bool hide_empty);
template <typename t_output>
t_output &serialize_array_interleave(t_output &output,
                                     const std::vector<std::string> &array1,
                                     const std::vector<std::string> &array2,
                                     const bool hide_empty);
template <typename t_output>
t_output &serialize_number(t_output &output, auto number)
  requires(std::integral<decltype(number)> ||
           std::floating_point<decltype(number)>)
{
  using namespace bits_and_bytes::stream_append;
  output += std::to_string(number);
  return output;
}
template <typename t_output>
t_output &serialize_string(t_output &output, const std::string_view string);
template <typename t_output>
t_output &serialize_bool(t_output &output, const bool b);

i3bar_data::click_event parse_click_event(const std::string_view click_event);
} // namespace impl
} // namespace i3bar_protocol
} // namespace i3neostatus
#endif
