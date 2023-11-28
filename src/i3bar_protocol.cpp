#include "i3bar_protocol.hpp"

#include <boost/json.hpp>

#include <iostream>
#include <limits>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

const std::string i3bar_protocol::block::content::k_separator_block_width_str{
    "separator_block_width"};

void i3bar_protocol::print_header(
    const header &output_value, std::ostream &output_stream /*= std::cout */) {
  output_stream << boost::json::serialize(boost::json::value_from(output_value))
                << g_k_newline_char << std::flush;
}

void i3bar_protocol::print_array_start(
    std::ostream &output_stream /*= std::cout*/) {
  output_stream << g_k_json_array_opening_delimiter << g_k_newline_char;
  output_stream << g_k_json_array_opening_delimiter
                << g_k_json_array_closing_delimiter << g_k_newline_char;
  output_stream << std::flush;
}

void i3bar_protocol::print_statusline(
    const std::vector<block> &output_value,
    std::ostream &output_stream /*= std::cout*/) {
  output_stream << g_k_json_array_element_separator
                << boost::json::serialize(boost::json::value_from(output_value))
                << g_k_newline_char << std::flush;
}

void i3bar_protocol::read_array_start(
    std::istream &input_stream /*=std::cin*/) {
  input_stream.ignore(std::numeric_limits<std::streamsize>::max(),
                      g_k_json_array_opening_delimiter);
}

i3bar_protocol::click_event
i3bar_protocol::read_click_event(std::istream &input_stream /*= std::cin*/) {
  std::string input_str{};
  std::getline(input_stream, input_str, g_k_newline_char);

  if (input_str.front() == g_k_json_array_element_separator) {
    input_str.front() = g_k_space_char;
  }

  boost::json::value bj_value{boost::json::parse(std::move(input_str))};

  return boost::json::value_to<click_event>(std::move(bj_value));
}

void boost::json::tag_invoke(const boost::json::value_from_tag &,
                             boost::json::value &bj_value,
                             const i3bar_protocol::header &header) {
  boost::json::object bj_object{};

  bj_object.insert(std::make_pair(i3bar_protocol::header::k_version_str,
                                  std::int64_t{header.version}));
  if (header.stop_signal.has_value() == true) {
    bj_object.insert(std::make_pair(i3bar_protocol::header::k_stop_signal_str,
                                    std::int64_t{header.stop_signal.value()}));
  }
  if (header.cont_signal.has_value() == true) {
    bj_object.insert(std::make_pair(i3bar_protocol::header::k_cont_signal_str,
                                    std::int64_t{header.cont_signal.value()}));
  }
  if (header.click_events.has_value() == true) {
    bj_object.insert(std::make_pair(i3bar_protocol::header::k_click_events_str,
                                    bool{header.click_events.value()}));
  }

  bj_value = std::move(bj_object);
};

void boost::json::tag_invoke(const boost::json::value_from_tag &,
                             boost::json::value &bj_value,
                             i3bar_protocol::header &&header) {
  boost::json::object bj_object{};

  bj_object.insert(std::make_pair(i3bar_protocol::header::k_version_str,
                                  std::int64_t{std::move(header.version)}));
  if (header.stop_signal.has_value() == true) {
    bj_object.insert(
        std::make_pair(i3bar_protocol::header::k_stop_signal_str,
                       std::int64_t{std::move(header.stop_signal.value())}));
  }
  if (header.cont_signal.has_value() == true) {
    bj_object.insert(
        std::make_pair(i3bar_protocol::header::k_cont_signal_str,
                       std::int64_t{std::move(header.cont_signal.value())}));
  }
  if (header.click_events.has_value() == true) {
    bj_object.insert(
        std::make_pair(i3bar_protocol::header::k_click_events_str,
                       bool{std::move(header.click_events.value())}));
  }

  bj_value = std::move(bj_object);
}

void boost::json::tag_invoke(const boost::json::value_from_tag &,
                             boost::json::value &bj_value,
                             const i3bar_protocol::block &block) {
  boost::json::object bj_object{};

  bj_object.insert(
      std::make_pair(i3bar_protocol::block::content::k_full_text_str,
                     boost::json::string{block.content.full_text}));

  if (block.id.name.has_value() == true) {
    bj_object.insert(
        std::make_pair(i3bar_protocol::block::id::k_name_str,
                       boost::json::string{block.id.name.value()}));
  }
  if (block.id.instance.has_value() == true) {
    bj_object.insert(std::make_pair(
        i3bar_protocol::block::id::k_instance_str,
        boost::json::string{std::to_string(block.id.instance.value())}));
  }
  if (block.content.short_text.has_value() == true) {
    bj_object.insert(
        std::make_pair(i3bar_protocol::block::content::k_short_text_str,
                       boost::json::string{block.content.short_text.value()}));
  }
  if (block.content.color.has_value() == true) {
    bj_object.insert(
        std::make_pair(i3bar_protocol::block::content::k_color_str,
                       boost::json::string{block.content.color.value()}));
  }
  if (block.content.background.has_value() == true) {
    bj_object.insert(
        std::make_pair(i3bar_protocol::block::content::k_background_str,
                       boost::json::string{block.content.background.value()}));
  }
  if (block.content.border.has_value() == true) {
    bj_object.insert(
        std::make_pair(i3bar_protocol::block::content::k_border_str,
                       boost::json::string{block.content.border.value()}));
  }
  if (block.content.border_top.has_value() == true) {
    bj_object.insert(
        std::make_pair(i3bar_protocol::block::content::k_border_top_str,
                       std::int64_t{block.content.border_top.value()}));
  }
  if (block.content.border_right.has_value() == true) {
    bj_object.insert(
        std::make_pair(i3bar_protocol::block::content::k_border_right_str,
                       std::int64_t{block.content.border_right.value()}));
  }
  if (block.content.border_bottom.has_value() == true) {
    bj_object.insert(
        std::make_pair(i3bar_protocol::block::content::k_border_bottom_str,
                       std::int64_t{block.content.border_bottom.value()}));
  }
  if (block.content.border_left.has_value() == true) {
    bj_object.insert(
        std::make_pair(i3bar_protocol::block::content::k_border_left_str,
                       std::int64_t{block.content.border_left.value()}));
  }
  if (block.content.min_width.has_value() == true) {
    decltype(block.content.min_width.value()) min_width_value{
        block.content.min_width.value()};
    switch (min_width_value.index()) {
    case 0: {
      bj_object.insert(std::make_pair(
          i3bar_protocol::block::content::k_min_width_str,
          std::int64_t{std::get<0>(std::move(min_width_value))}));
    } break;
    case 1: {
      bj_object.insert(std::make_pair(
          i3bar_protocol::block::content::k_min_width_str,
          boost::json::string{std::get<1>(std::move(min_width_value))}));
    } break;
    }
  }
  if (block.content.align.has_value() == true) {
    bj_object.insert(
        std::make_pair(i3bar_protocol::block::content::k_align_str,
                       boost::json::string{block.content.align.value()}));
  }
  if (block.content.urgent.has_value() == true) {
    bj_object.insert(
        std::make_pair(i3bar_protocol::block::content::k_urgent_str,
                       bool{block.content.urgent.value()}));
  }
  if (block.content.separator.has_value() == true) {
    bj_object.insert(
        std::make_pair(i3bar_protocol::block::content::k_separator_str,
                       bool{block.content.separator.value()}));
  }
  if (block.content.separator_block_width.has_value() == true) {
    bj_object.insert(std::make_pair(
        i3bar_protocol::block::content::k_separator_block_width_str,
        std::int64_t{block.content.separator_block_width.value()}));
  }
  if (block.content.markup.has_value() == true) {
    bj_object.insert(
        std::make_pair(i3bar_protocol::block::content::k_markup_str,
                       boost::json::string{block.content.markup.value()}));
  }

  bj_value = std::move(bj_object);
}

void boost::json::tag_invoke(const value_from_tag &, value &bj_value,
                             i3bar_protocol::block &&block) {
  boost::json::object bj_object{};

  bj_object.insert(
      std::make_pair(i3bar_protocol::block::content::k_full_text_str,
                     boost::json::string{std::move(block.content.full_text)}));

  if (block.id.name.has_value() == true) {
    bj_object.insert(
        std::make_pair(i3bar_protocol::block::id::k_name_str,
                       boost::json::string{std::move(block.id.name.value())}));
  }
  if (block.id.instance.has_value() == true) {
    bj_object.insert(std::make_pair(
        i3bar_protocol::block::id::k_instance_str,
        boost::json::string{std::to_string(block.id.instance.value())}));
  }
  if (block.content.short_text.has_value() == true) {
    bj_object.insert(std::make_pair(
        i3bar_protocol::block::content::k_short_text_str,
        boost::json::string{std::move(block.content.short_text.value())}));
  }
  if (block.content.color.has_value() == true) {
    bj_object.insert(std::make_pair(
        i3bar_protocol::block::content::k_color_str,
        boost::json::string{std::move(block.content.color.value())}));
  }
  if (block.content.background.has_value() == true) {
    bj_object.insert(std::make_pair(
        i3bar_protocol::block::content::k_background_str,
        boost::json::string{std::move(block.content.background.value())}));
  }
  if (block.content.border.has_value() == true) {
    bj_object.insert(std::make_pair(
        i3bar_protocol::block::content::k_border_str,
        boost::json::string{std::move(block.content.border.value())}));
  }
  if (block.content.border_top.has_value() == true) {
    bj_object.insert(std::make_pair(
        i3bar_protocol::block::content::k_border_top_str,
        std::int64_t{std::move(block.content.border_top.value())}));
  }
  if (block.content.border_right.has_value() == true) {
    bj_object.insert(std::make_pair(
        i3bar_protocol::block::content::k_border_right_str,
        std::int64_t{std::move(block.content.border_right.value())}));
  }
  if (block.content.border_bottom.has_value() == true) {
    bj_object.insert(std::make_pair(
        i3bar_protocol::block::content::k_border_bottom_str,
        std::int64_t{std::move(block.content.border_bottom.value())}));
  }
  if (block.content.border_left.has_value() == true) {
    bj_object.insert(std::make_pair(
        i3bar_protocol::block::content::k_border_left_str,
        std::int64_t{std::move(block.content.border_left.value())}));
  }
  if (block.content.min_width.has_value() == true) {
    std::variant<i3bar_protocol::pixel_count_t, std::string> min_width_value{
        std::move(block.content.min_width.value())};
    switch (min_width_value.index()) {
    case 0: {
      bj_object.insert(std::make_pair(
          i3bar_protocol::block::content::k_min_width_str,
          std::int64_t{std::get<0>(std::move(min_width_value))}));
    } break;
    case 1: {
      bj_object.insert(std::make_pair(
          i3bar_protocol::block::content::k_min_width_str,
          boost::json::string{std::get<1>(std::move(min_width_value))}));
    } break;
    }
  }
  if (block.content.align.has_value() == true) {
    bj_object.insert(std::make_pair(
        i3bar_protocol::block::content::k_align_str,
        boost::json::string{std::move(block.content.align.value())}));
  }
  if (block.content.urgent.has_value() == true) {
    bj_object.insert(
        std::make_pair(i3bar_protocol::block::content::k_urgent_str,
                       bool{std::move(block.content.urgent.value())}));
  }
  if (block.content.separator.has_value() == true) {
    bj_object.insert(
        std::make_pair(i3bar_protocol::block::content::k_separator_str,
                       bool{std::move(block.content.separator.value())}));
  }
  if (block.content.separator_block_width.has_value() == true) {
    bj_object.insert(std::make_pair(
        i3bar_protocol::block::content::k_separator_block_width_str,
        std::int64_t{std::move(block.content.separator_block_width.value())}));
  }
  if (block.content.markup.has_value() == true) {
    bj_object.insert(std::make_pair(
        i3bar_protocol::block::content::k_markup_str,
        boost::json::string{std::move(block.content.markup.value())}));
  }

  bj_value = std::move(bj_object);
}

i3bar_protocol::click_event boost::json::tag_invoke(
    const boost::json::value_to_tag<i3bar_protocol::click_event> &,
    const boost::json::value &bj_value) {

  const auto boost_json_string_to_std_string{
      [](const boost::json::string &bj_string) -> std::string {
        return std::string{bj_string.begin(), bj_string.end()};
      }};

  const boost::json::object &bj_object{bj_value.as_object()};

  i3bar_protocol::click_event click_event{
      .id{.name{boost_json_string_to_std_string(
              bj_object.at(i3bar_protocol::click_event::id::k_name_str)
                  .as_string())},
          .instance{std::stoul(boost_json_string_to_std_string(
              bj_object.at(i3bar_protocol::click_event::id::k_instance_str)
                  .as_string()))}},
      .content{
          .x{bj_object.at(i3bar_protocol::click_event::content::k_x_str)
                 .as_int64()},
          .y{bj_object.at(i3bar_protocol::click_event::content::k_y_str)
                 .as_int64()},
          .button{static_cast<decltype(click_event.content.button)>(
              bj_object.at(i3bar_protocol::click_event::content::k_button_str)
                  .as_int64())},
          .relative_x{
              bj_object
                  .at(i3bar_protocol::click_event::content::k_relative_x_str)
                  .as_int64()},
          .relative_y{
              bj_object
                  .at(i3bar_protocol::click_event::content::k_relative_y_str)
                  .as_int64()},
          .output_x{
              bj_object.at(i3bar_protocol::click_event::content::k_output_x_str)
                  .as_int64()},
          .output_y{
              bj_object.at(i3bar_protocol::click_event::content::k_output_y_str)
                  .as_int64()},
          .width{bj_object.at(i3bar_protocol::click_event::content::k_width_str)
                     .as_int64()},
          .height{
              bj_object.at(i3bar_protocol::click_event::content::k_height_str)
                  .as_int64()},
          .modifiers{[&boost_json_string_to_std_string](
                         const boost::json::array &bj_array) {
            std::vector<std::string> ret_val{};
            ret_val.reserve(bj_array.size());
            for (auto p{bj_array.begin()}; p != bj_array.end(); ++p) {
              ret_val.emplace_back(
                  boost_json_string_to_std_string(p->as_string()));
            }
            return ret_val;
          }(bj_object.at(i3bar_protocol::click_event::content::k_modifiers_str)
                         .as_array())}}};

  return click_event;
}
