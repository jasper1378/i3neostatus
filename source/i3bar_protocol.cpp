#include "i3bar_protocol.hpp"

#include "hide_block.hpp"
#include "i3bar_data.hpp"
#include "i3bar_data_conversions.hpp"
#include "misc.hpp"
#include "module_id.hpp"

#include "bits-and-bytes/constexpr_min_max.hpp"
#include "bits-and-bytes/stream_append.hpp"
#include "libconfigfile/color.hpp"

#include <cassert>
#include <charconv>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace bits_and_bytes::stream_append;

void i3neostatus::i3bar_protocol::print_header(
    const i3bar_data::header &value, std::ostream &stream /*= std::cout */) {
  impl::serialize_header(stream, value);
  stream << json_constants::k_newline << std::flush;
}

void i3neostatus::i3bar_protocol::init_statusline(
    std::ostream &stream /*= std::cout*/) {
  stream << json_constants::k_array_opening_delimiter
         << json_constants::k_newline;
  stream << json_constants::k_array_opening_delimiter
         << json_constants::k_array_closing_delimiter
         << json_constants::k_newline;
  stream << std::flush;
}

void i3neostatus::i3bar_protocol::print_statusline(
    const std::vector<struct i3bar_data::block> &content, const bool hide_empty,
    std::ostream &stream /*= std::cout*/) {
  impl::print_statusline(impl::serialize_blocks(content, hide_empty),
                         hide_empty, stream);
}

void i3neostatus::i3bar_protocol::print_statusline(
    const std::vector<struct i3bar_data::block> &content,
    const std::vector<i3bar_data::block> &separators, const bool hide_empty,
    std::ostream &stream /*= std::cout*/) {
  impl::print_statusline(impl::serialize_blocks(content, hide_empty),
                         impl::serialize_blocks(separators, hide_empty),
                         hide_empty, stream);
}

void i3neostatus::i3bar_protocol::print_statusline(
    const struct i3bar_data::block &content,
    const module_id::type content_index,
    std::vector<std::string> &content_cache, const bool hide_empty,
    std::ostream &stream /*= std::cout*/) {
  content_cache[content_index].clear();
  impl::serialize_block(content_cache[content_index], content, hide_empty);
  impl::print_statusline(content_cache, hide_empty, stream);
}

void i3neostatus::i3bar_protocol::print_statusline(
    const struct i3bar_data::block &content,
    const module_id::type content_index,
    std::vector<std::string> &content_cache,
    const i3bar_data::block &separator_left,
    const module_id::type separator_left_index,
    const i3bar_data::block &separator_right,
    const module_id::type separator_right_index,
    std::vector<std::string> &separator_cache, const bool hide_empty,
    std::ostream &stream) {
  content_cache[content_index].clear();
  impl::serialize_block(content_cache[content_index], content, hide_empty);
  separator_cache[separator_left_index].clear();
  impl::serialize_block(separator_cache[separator_left_index], separator_left,
                        hide_empty);
  separator_cache[separator_right_index].clear();
  impl::serialize_block(separator_cache[separator_right_index], separator_right,
                        hide_empty);
  impl::print_statusline(content_cache, separator_cache, hide_empty, stream);
}

void i3neostatus::i3bar_protocol::print_statusline(
    const std::vector<struct i3bar_data::block> &content,
    std::vector<std::string> &content_cache, const bool hide_empty,
    std::ostream &stream /*= std::cout*/) {
  content_cache = impl::serialize_blocks(content, hide_empty);
  impl::print_statusline(content_cache, hide_empty, stream);
}

void i3neostatus::i3bar_protocol::print_statusline(
    const std::vector<struct i3bar_data::block> &content,
    std::vector<std::string> &content_cache,
    const std::vector<i3bar_data::block> &separators,
    std::vector<std::string> &separator_cache, const bool hide_empty,
    std::ostream &stream /*= std::cout*/) {
  content_cache = impl::serialize_blocks(content, hide_empty);
  separator_cache = impl::serialize_blocks(separators, hide_empty);
  impl::print_statusline(content_cache, separator_cache, hide_empty, stream);
}

void i3neostatus::i3bar_protocol::init_click_event(
    std::istream &input_stream /*= std::cin*/) {
  input_stream.ignore(1, json_constants::k_array_opening_delimiter);
  input_stream.ignore(1, json_constants::k_newline);
}

i3neostatus::i3bar_data::click_event
i3neostatus::i3bar_protocol::read_click_event(
    std::istream &input_stream /*= std::cin*/) {
  std::string input_str{};
  std::getline(input_stream, input_str, json_constants::k_newline);

  return impl::parse_click_event(input_str);
}

void i3neostatus::i3bar_protocol::impl::print_statusline(
    const std::vector<std::string> &content, const bool hide_empty,
    std::ostream &stream /*= std::cout*/) {
  stream << json_constants::k_element_separator;
  serialize_array(stream, content, hide_empty);
  stream << json_constants::k_newline << std::flush;
}

void i3neostatus::i3bar_protocol::impl::print_statusline(
    const std::vector<std::string> &content,
    const std::vector<std::string> &separators, const bool hide_empty,
    std::ostream &stream /*= std::cout*/) {
  assert((content.size() + 1) == separators.size());
  stream << json_constants::k_element_separator;
  serialize_array_interleave(stream, separators, content, hide_empty);
  stream << json_constants::k_newline << std::flush;
}

template <typename t_output>
t_output &i3neostatus::i3bar_protocol::impl::serialize_header(
    t_output &output, const i3bar_data::header &header) {
  return serialize_object(
      output, [&header]() -> std::vector<std::pair<std::string, std::string>> {
        std::vector<std::pair<std::string, std::string>> ret_val;

        ret_val.emplace_back(json_strings::header::k_version, std::string{});
        serialize_number(ret_val.back().second, header.version);

        ret_val.emplace_back(json_strings::header::k_stop_signal,
                             std::string{});
        serialize_number(ret_val.back().second, header.stop_signal);

        ret_val.emplace_back(json_strings::header::k_cont_signal,
                             std::string{});
        serialize_number(ret_val.back().second, header.cont_signal);

        ret_val.emplace_back(json_strings::header::k_click_events,
                             std::string{});
        serialize_bool(ret_val.back().second, header.click_events);

        return ret_val;
      }());
}

template <typename t_output>
t_output &i3neostatus::i3bar_protocol::impl::serialize_block(
    t_output &output, const struct i3bar_data::block &block,
    const bool hide_empty) {
  if (hide_block::get(block) && hide_empty) {
    output += hide_block::set<std::string>();
  } else {
    serialize_object(
        output, [&block]() -> std::vector<std::pair<std::string, std::string>> {
          std::vector<std::pair<std::string, std::string>> ret_val;

          ret_val.emplace_back(json_strings::block::k_name, std::string{});
          serialize_string(ret_val.back().second, block.id.name);

          ret_val.emplace_back(json_strings::block::k_instance, std::string{});
          serialize_string(ret_val.back().second,
                           std::to_string(block.id.instance));

          ret_val.emplace_back(json_strings::block::k_separator, std::string{});
          serialize_bool(ret_val.back().second,
                         block.data.program.global.separator);

          ret_val.emplace_back(json_strings::block::k_separator_block_width,
                               std::string{});
          serialize_number(ret_val.back().second,
                           block.data.program.global.separator_block_width);

          ret_val.emplace_back(json_strings::block::k_color, std::string{});
          serialize_string(
              ret_val.back().second,
              libconfigfile::color::to_string(block.data.program.theme.color));

          ret_val.emplace_back(json_strings::block::k_background,
                               std::string{});
          serialize_string(ret_val.back().second,
                           libconfigfile::color::to_string(
                               block.data.program.theme.background));

          ret_val.emplace_back(json_strings::block::k_border, std::string{});
          serialize_string(
              ret_val.back().second,
              libconfigfile::color::to_string(block.data.program.theme.border));

          ret_val.emplace_back(json_strings::block::k_border_top,
                               std::string{});
          serialize_number(ret_val.back().second,
                           block.data.program.theme.border_top);

          ret_val.emplace_back(json_strings::block::k_border_right,
                               std::string{});
          serialize_number(ret_val.back().second,
                           block.data.program.theme.border_right);

          ret_val.emplace_back(json_strings::block::k_border_bottom,
                               std::string{});
          serialize_number(ret_val.back().second,
                           block.data.program.theme.border_bottom);

          ret_val.emplace_back(json_strings::block::k_border_left,
                               std::string{});
          serialize_number(ret_val.back().second,
                           block.data.program.theme.border_left);

          ret_val.emplace_back(json_strings::block::k_full_text, std::string{});
          serialize_string(ret_val.back().second, block.data.module.full_text);

          if (block.data.module.short_text.has_value()) {
            ret_val.emplace_back(json_strings::block::k_short_text,
                                 std::string{});
            serialize_string(ret_val.back().second,
                             *block.data.module.short_text);
          }

          if (block.data.module.min_width.has_value()) {
            ret_val.emplace_back(json_strings::block::k_min_width,
                                 std::string{});
            ((block.data.module.min_width->index() == 0)
                 ? (serialize_number(ret_val.back().second,
                                     std::get<0>(*block.data.module.min_width)))
                 : (serialize_string(
                       ret_val.back().second,
                       std::get<1>(*block.data.module.min_width))));
          }

          if (block.data.module.align.has_value()) {
            ret_val.emplace_back(json_strings::block::k_align, std::string{});
            serialize_string(
                ret_val.back().second,
                i3bar_data::types::to_string(*block.data.module.align));
          }

          if (block.data.module.urgent.has_value()) {
            ret_val.emplace_back(json_strings::block::k_urgent, std::string{});
            serialize_bool(ret_val.back().second, *block.data.module.urgent);
          }

          if (block.data.module.markup.has_value()) {
            ret_val.emplace_back(json_strings::block::k_markup, std::string{});
            serialize_string(
                ret_val.back().second,
                i3bar_data::types::to_string(*block.data.module.markup));
          }

          return ret_val;
        }());
  }
  return output;
}

std::vector<std::string> i3neostatus::i3bar_protocol::impl::serialize_blocks(
    const std::vector<struct i3bar_data::block> &blocks,
    const bool hide_empty) {
  std::vector<std::string> ret_val(blocks.size());
  for (std::size_t i{0}; i < ret_val.size(); ++i) {
    impl::serialize_block(ret_val[i], blocks[i], hide_empty);
  }
  return ret_val;
}

template <typename t_output>
t_output &i3neostatus::i3bar_protocol::impl::serialize_name_value(
    t_output &output, const std::string &name, const std::string &value) {

  output += json_constants::k_string_delimiter;
  output += name;
  output += json_constants::k_string_delimiter;
  output += json_constants::k_name_value_separator;
  output += value;
  return output;
}

template <typename t_output>
t_output &i3neostatus::i3bar_protocol::impl::serialize_object(
    t_output &output,
    const std::vector<std::pair<std::string, std::string>> &object) {
  output += json_constants::k_object_opening_delimiter;

  for (std::size_t i{0}; i < object.size(); ++i) {
    if (i != 0) {
      output += json_constants::k_element_separator;
    }
    serialize_name_value(output, object[i].first, object[i].second);
  }

  output += json_constants::k_object_closing_delimiter;

  return output;
}

template <typename t_output>
t_output &i3neostatus::i3bar_protocol::impl::serialize_array(
    t_output &output, const std::vector<std::string> &array,
    const bool hide_empty) {
  output += json_constants::k_array_opening_delimiter;

  bool first{true};
  for (std::size_t i{0}; i < array.size(); ++i) {
    if (!(hide_block::get(array[i]) && hide_empty)) {
      if (first) {
        first = false;
      } else {
        output += json_constants::k_element_separator;
      }
      output += array[i];
    }
  }

  output += json_constants::k_array_closing_delimiter;

  return output;
}

template <typename t_output>
t_output &i3neostatus::i3bar_protocol::impl::serialize_array_interleave(
    t_output &output, const std::vector<std::string> &array1,
    const std::vector<std::string> &array2, const bool hide_empty) {
  int remaining{2};
  bool first{true};
  std::size_t idx1{0};
  std::size_t idx2{0};

  const auto do_serialize{
      [hide_empty, &remaining, &first](const std::vector<std::string> &a,
                                       std::size_t &i) {
        for (;; ++i) {
          if (i == a.size()) {
            --remaining;
            break;
          } else if (i > a.size()) {
            break;
          } else if (hide_block::get(a[i]) && hide_empty) {
            continue;
          } else {
            if (first) {
              first = false;
            } else {
              std::cout << json_constants::k_element_separator;
            }
            std::cout << a[i];
            break;
          }
        }
        ++i;
      }};

  output += json_constants::k_array_opening_delimiter;
  while (remaining) {
    do_serialize(array1, idx1);
    do_serialize(array2, idx2);
  }
  output += json_constants::k_array_closing_delimiter;

  return output;
}

template <typename t_output>
t_output &i3neostatus::i3bar_protocol::impl::serialize_string(
    t_output &output, const std::string_view string) {
  static const std::string k_control_chars{
      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A,
      0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
      0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F};

  static const std::unordered_map<char, std::string> k_control_char_codes{
      {0x00, "u0000"}, {0x01, "u0001"}, {0x02, "u0002"}, {0x03, "u0003"},
      {0x04, "u0004"}, {0x05, "u0005"}, {0x06, "u0006"}, {0x07, "u0007"},
      {0x08, "u0008"}, {0x09, "u0009"}, {0x0A, "u000A"}, {0x0B, "u000B"},
      {0x0C, "u000C"}, {0x0D, "u000D"}, {0x0E, "u000E"}, {0x0F, "u000F"},
      {0x10, "u0010"}, {0x11, "u0011"}, {0x12, "u0012"}, {0x13, "u0013"},
      {0x14, "u0014"}, {0x15, "u0015"}, {0x16, "u0016"}, {0x17, "u0017"},
      {0x18, "u0018"}, {0x19, "u0019"}, {0x1A, "u001A"}, {0x1B, "u001B"},
      {0x1C, "u001C"}, {0x1D, "u001D"}, {0x1E, "u001E"}, {0x1F, "u001F"}};

  static const std::string k_need_to_replace{
      k_control_chars + json_constants::k_string_delimiter +
      json_constants::k_escape_leader};

  // ret_val.reserve(string.size() + 2);

  output += json_constants::k_string_delimiter;

  std::string::size_type pos{0};
  std::string::size_type pos_prev{0};
  while (true) {
    pos = string.find_first_of(k_need_to_replace, pos_prev);
    if (pos == std::string::npos) {
      break;
    } else {
      output += string.substr(pos_prev, (pos - pos_prev));
      output += json_constants::k_escape_leader;

      switch (string[pos]) {
      case json_constants::k_string_delimiter: {
        output += json_constants::k_string_delimiter;
      } break;
      case json_constants::k_escape_leader: {
        output += json_constants::k_escape_leader;
      } break;
      default: {
        output += k_control_char_codes.at(string[pos]);
      } break;
      }

      pos_prev = pos + 1;
    }
  }
  output += string.substr(pos_prev);

  output += json_constants::k_string_delimiter;

  return output;
}

template <typename t_output>
t_output &i3neostatus::i3bar_protocol::impl::serialize_bool(t_output &output,
                                                            const bool b) {
  output += ((b) ? ("true") : ("false"));
  return output;
}

i3neostatus::i3bar_data::click_event
i3neostatus::i3bar_protocol::impl::parse_click_event(
    const std::string_view click_event) {
  const auto substr{[](const std::string_view str,
                       const std::string::size_type begin,
                       const std::string::size_type end) -> std::string_view {
    return str.substr(begin, end - begin + 1);
  }};
  const auto find_first_digit{
      [](const std::string_view str,
         const std::string::size_type pos = 0) -> std::string::size_type {
        for (std::string::size_type i{pos}; i < str.size(); ++i) {
          if (str[i] >= '0' && str[i] <= '9') {
            return i;
          }
        }
        return std::string::npos;
      }};
  const auto find_first_not_digit{
      [](const std::string_view str,
         const std::string::size_type pos = 0) -> std::string::size_type {
        for (std::string::size_type i{pos}; i < str.size(); ++i) {
          if (!(str[i] >= '0' && str[i] <= '9')) {
            return i;
          }
        }
        return std::string::npos;
      }};
  const auto read_string_value{
      [&substr](const std::string_view str,
                const std::string::size_type name_end_pos,
                std::string::size_type &continue_from_pos) -> std::string {
        std::string::size_type value_begin_pos{
            str.find(json_constants::k_string_delimiter, name_end_pos + 2) + 1};
        std::string::size_type value_end_pos{
            str.find(json_constants::k_string_delimiter, value_begin_pos) - 1};
        continue_from_pos = value_end_pos + 2;
        return std::string{substr(str, value_begin_pos, value_end_pos)};
      }};
  const auto read_numeric_value{
      [&substr, &find_first_digit, &find_first_not_digit](
          const std::string_view str, const std::string::size_type name_end_pos,
          std::string::size_type &continue_from_pos) -> std::string_view {
        std::string::size_type value_begin_pos{
            find_first_digit(str, name_end_pos + 2)};
        std::string::size_type value_end_pos{
            find_first_not_digit(str, value_begin_pos)};
        continue_from_pos = value_end_pos + 1;
        return substr(str, value_begin_pos, value_end_pos);
      }};

  std::string name_buf;
  name_buf.resize(bits_and_bytes::constexpr_min_max::max(
      json_strings::click_event::k_name.size(),
      json_strings::click_event::k_instance.size(),
      json_strings::click_event::k_x.size(),
      json_strings::click_event::k_y.size(),
      json_strings::click_event::k_button.size(),
      json_strings::click_event::k_relative_x.size(),
      json_strings::click_event::k_relative_y.size(),
      json_strings::click_event::k_output_x.size(),
      json_strings::click_event::k_output_y.size(),
      json_strings::click_event::k_width.size(),
      json_strings::click_event::k_height.size(),
      json_strings::click_event::k_modifiers.size()));

  i3bar_data::click_event ret_val{};

  std::string::size_type continue_from_pos{0};

  while (true) {
    std::string::size_type name_begin_pos{click_event.find(
        json_constants::k_string_delimiter, continue_from_pos)};
    if (name_begin_pos == std::string::npos) {
      break;
    } else {
      ++name_begin_pos;
      std::string::size_type name_end_pos =
          (click_event.find(json_constants::k_string_delimiter,
                            name_begin_pos) -
           1);
      name_buf = substr(click_event, name_begin_pos, name_end_pos);

      switch (misc::constexpr_hash_string::hash(name_buf)) {
      case misc::constexpr_hash_string::hash(
          json_strings::click_event::k_name): {
        ret_val.id.name =
            read_string_value(click_event, name_end_pos, continue_from_pos);
      } break;
      case misc::constexpr_hash_string::hash(
          json_strings::click_event::k_instance): {
        ret_val.id.instance = module_id::from_string(
            read_string_value(click_event, name_end_pos, continue_from_pos));
      } break;
      case misc::constexpr_hash_string::hash(json_strings::click_event::k_x): {
        std::string_view value{
            read_numeric_value(click_event, name_end_pos, continue_from_pos)};
        std::from_chars(value.data(), value.data() + value.size(),
                        ret_val.data.x);
      } break;
      case misc::constexpr_hash_string::hash(json_strings::click_event::k_y): {
        std::string_view value{
            read_numeric_value(click_event, name_end_pos, continue_from_pos)};
        std::from_chars(value.data(), value.data() + value.size(),
                        ret_val.data.y);
      } break;
      case misc::constexpr_hash_string::hash(
          json_strings::click_event::k_button): {
        std::string_view value{
            read_numeric_value(click_event, name_end_pos, continue_from_pos)};
        std::from_chars(value.data(), value.data() + value.size(),
                        ret_val.data.button);
      } break;
      case misc::constexpr_hash_string::hash(
          json_strings::click_event::k_relative_x): {
        std::string_view value{
            read_numeric_value(click_event, name_end_pos, continue_from_pos)};
        std::from_chars(value.data(), value.data() + value.size(),
                        ret_val.data.relative_x);
      } break;
      case misc::constexpr_hash_string::hash(
          json_strings::click_event::k_relative_y): {
        std::string_view value{
            read_numeric_value(click_event, name_end_pos, continue_from_pos)};
        std::from_chars(value.data(), value.data() + value.size(),
                        ret_val.data.relative_y);
      } break;
      case misc::constexpr_hash_string::hash(
          json_strings::click_event::k_output_x): {
        std::string_view value{
            read_numeric_value(click_event, name_end_pos, continue_from_pos)};
        std::from_chars(value.data(), value.data() + value.size(),
                        ret_val.data.output_x);
      } break;
      case misc::constexpr_hash_string::hash(
          json_strings::click_event::k_output_y): {
        std::string_view value{
            read_numeric_value(click_event, name_end_pos, continue_from_pos)};
        std::from_chars(value.data(), value.data() + value.size(),
                        ret_val.data.output_y);
      } break;
      case misc::constexpr_hash_string::hash(
          json_strings::click_event::k_width): {
        std::string_view value{
            read_numeric_value(click_event, name_end_pos, continue_from_pos)};
        std::from_chars(value.data(), value.data() + value.size(),
                        ret_val.data.width);
      } break;
      case misc::constexpr_hash_string::hash(
          json_strings::click_event::k_height): {
        std::string_view value{
            read_numeric_value(click_event, name_end_pos, continue_from_pos)};
        std::from_chars(value.data(), value.data() + value.size(),
                        ret_val.data.height);
      } break;
      case misc::constexpr_hash_string::hash(
          json_strings::click_event::k_modifiers): {
        std::string::size_type array_begin_pos{click_event.find(
            json_constants::k_array_opening_delimiter, name_end_pos + 2)};
        std::string::size_type array_end_pos{click_event.find(
            json_constants::k_array_closing_delimiter, array_begin_pos)};
        if ((array_begin_pos + 1) != array_end_pos) {
          for (std::string::size_type i{array_begin_pos}; i < array_end_pos;) {
            std::string::size_type value_begin_pos{
                click_event.find(json_constants::k_string_delimiter, i) + 1};
            std::string::size_type value_end_pos{
                click_event.find(json_constants::k_string_delimiter,
                                 value_begin_pos) -
                1};
            ret_val.data.modifiers |= i3bar_data::types::from_string<
                i3bar_data::types::click_modifiers>(std::string{
                substr(click_event, value_begin_pos, value_end_pos)});
            i = value_end_pos + 2;
          }
        }
        continue_from_pos = array_end_pos + 1;
      } break;
      }
    }
  }

  return ret_val;
}
