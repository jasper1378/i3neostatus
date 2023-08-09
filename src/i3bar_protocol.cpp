#include "i3bar_protocol.hpp"

#include <boost/json.hpp>

#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

const std::string i3bar_protocol::block::k_separator_block_width_str{
    "separator_block_width"};

void i3bar_protocol::output_header(
    const header &output_value, std::ostream &output_stream /*= std::cout */) {
  output_stream << boost::json::serialize(boost::json::value_from(output_value))
                << g_k_newline_char << std::flush;
}

void i3bar_protocol::output_infinite_array_start(
    std::ostream &output_stream /*= std::cout*/) {
  output_stream << g_k_json_array_opening_delimiter << g_k_newline_char;
  output_stream << g_k_json_array_opening_delimiter
                << g_k_json_array_closing_delimiter << g_k_newline_char;
  output_stream << std::flush;
}

void i3bar_protocol::output_statusline(
    const std::vector<block> &output_value,
    std::ostream &output_stream /*= std::cout*/) {
  output_stream << g_k_json_array_element_separator
                << boost::json::serialize(boost::json::value_from(output_value))
                << g_k_newline_char << std::flush;
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

  bj_object.insert(std::make_pair(i3bar_protocol::block::k_full_text_str,
                                  boost::json::string{block.full_text}));

  if (block.short_text.has_value() == true) {
    bj_object.insert(
        std::make_pair(i3bar_protocol::block::k_short_text_str,
                       boost::json::string{block.short_text.value()}));
  }
  if (block.color.has_value() == true) {
    bj_object.insert(std::make_pair(i3bar_protocol::block::k_color_str,
                                    boost::json::string{block.color.value()}));
  }
  if (block.background.has_value() == true) {
    bj_object.insert(
        std::make_pair(i3bar_protocol::block::k_background_str,
                       boost::json::string{block.background.value()}));
  }
  if (block.border.has_value() == true) {
    bj_object.insert(std::make_pair(i3bar_protocol::block::k_border_str,
                                    boost::json::string{block.border.value()}));
  }
  if (block.border_top.has_value() == true) {
    bj_object.insert(std::make_pair(i3bar_protocol::block::k_border_top_str,
                                    std::int64_t{block.border_top.value()}));
  }
  if (block.border_right.has_value() == true) {
    bj_object.insert(std::make_pair(i3bar_protocol::block::k_border_right_str,
                                    std::int64_t{block.border_right.value()}));
  }
  if (block.border_bottom.has_value() == true) {
    bj_object.insert(std::make_pair(i3bar_protocol::block::k_border_bottom_str,
                                    std::int64_t{block.border_bottom.value()}));
  }
  if (block.border_left.has_value() == true) {
    bj_object.insert(std::make_pair(i3bar_protocol::block::k_border_left_str,
                                    std::int64_t{block.border_left.value()}));
  }
  if (block.min_width.has_value() == true) {
    decltype(block.min_width.value()) min_width_value{block.min_width.value()};
    switch (min_width_value.index()) {
    case 0: {
      bj_object.insert(std::make_pair(
          i3bar_protocol::block::k_min_width_str,
          std::int64_t{std::get<0>(std::move(min_width_value))}));
    } break;
    case 1: {
      bj_object.insert(std::make_pair(
          i3bar_protocol::block::k_min_width_str,
          boost::json::string{std::get<1>(std::move(min_width_value))}));
    } break;
    }
  }
  if (block.align.has_value() == true) {
    bj_object.insert(std::make_pair(i3bar_protocol::block::k_align_str,
                                    boost::json::string{block.align.value()}));
  }
  if (block.name.has_value() == true) {
    bj_object.insert(std::make_pair(i3bar_protocol::block::k_name_str,
                                    boost::json::string{block.name.value()}));
  }
  if (block.instance.has_value() == true) {
    bj_object.insert(
        std::make_pair(i3bar_protocol::block::k_instance_str,
                       boost::json::string{block.instance.value()}));
  }
  if (block.urgent.has_value() == true) {
    bj_object.insert(std::make_pair(i3bar_protocol::block::k_urgent_str,
                                    bool{block.urgent.value()}));
  }
  if (block.separator.has_value() == true) {
    bj_object.insert(std::make_pair(i3bar_protocol::block::k_separator_str,
                                    bool{block.separator.value()}));
  }
  if (block.separator_block_width.has_value() == true) {
    bj_object.insert(
        std::make_pair(i3bar_protocol::block::k_separator_block_width_str,
                       std::int64_t{block.separator_block_width.value()}));
  }
  if (block.markup.has_value() == true) {
    bj_object.insert(std::make_pair(i3bar_protocol::block::k_markup_str,
                                    boost::json::string{block.markup.value()}));
  }

  bj_value = std::move(bj_object);
}

void boost::json::tag_invoke(const value_from_tag &, value &bj_value,
                             i3bar_protocol::block &&block) {
  boost::json::object bj_object{};

  bj_object.insert(
      std::make_pair(i3bar_protocol::block::k_full_text_str,
                     boost::json::string{std::move(block.full_text)}));

  if (block.short_text.has_value() == true) {
    bj_object.insert(std::make_pair(
        i3bar_protocol::block::k_short_text_str,
        boost::json::string{std::move(block.short_text.value())}));
  }
  if (block.color.has_value() == true) {
    bj_object.insert(
        std::make_pair(i3bar_protocol::block::k_color_str,
                       boost::json::string{std::move(block.color.value())}));
  }
  if (block.background.has_value() == true) {
    bj_object.insert(std::make_pair(
        i3bar_protocol::block::k_background_str,
        boost::json::string{std::move(block.background.value())}));
  }
  if (block.border.has_value() == true) {
    bj_object.insert(
        std::make_pair(i3bar_protocol::block::k_border_str,
                       boost::json::string{std::move(block.border.value())}));
  }
  if (block.border_top.has_value() == true) {
    bj_object.insert(
        std::make_pair(i3bar_protocol::block::k_border_top_str,
                       std::int64_t{std::move(block.border_top.value())}));
  }
  if (block.border_right.has_value() == true) {
    bj_object.insert(
        std::make_pair(i3bar_protocol::block::k_border_right_str,
                       std::int64_t{std::move(block.border_right.value())}));
  }
  if (block.border_bottom.has_value() == true) {
    bj_object.insert(
        std::make_pair(i3bar_protocol::block::k_border_bottom_str,
                       std::int64_t{std::move(block.border_bottom.value())}));
  }
  if (block.border_left.has_value() == true) {
    bj_object.insert(
        std::make_pair(i3bar_protocol::block::k_border_left_str,
                       std::int64_t{std::move(block.border_left.value())}));
  }
  if (block.min_width.has_value() == true) {
    std::variant<i3bar_protocol::pixel_count_t, std::string> min_width_value{
        std::move(block.min_width.value())};
    switch (min_width_value.index()) {
    case 0: {
      bj_object.insert(std::make_pair(
          i3bar_protocol::block::k_min_width_str,
          std::int64_t{std::get<0>(std::move(min_width_value))}));
    } break;
    case 1: {
      bj_object.insert(std::make_pair(
          i3bar_protocol::block::k_min_width_str,
          boost::json::string{std::get<1>(std::move(min_width_value))}));
    } break;
    }
  }
  if (block.align.has_value() == true) {
    bj_object.insert(
        std::make_pair(i3bar_protocol::block::k_align_str,
                       boost::json::string{std::move(block.align.value())}));
  }
  if (block.name.has_value() == true) {
    bj_object.insert(
        std::make_pair(i3bar_protocol::block::k_name_str,
                       boost::json::string{std::move(block.name.value())}));
  }
  if (block.instance.has_value() == true) {
    bj_object.insert(
        std::make_pair(i3bar_protocol::block::k_instance_str,
                       boost::json::string{std::move(block.instance.value())}));
  }
  if (block.urgent.has_value() == true) {
    bj_object.insert(std::make_pair(i3bar_protocol::block::k_urgent_str,
                                    bool{std::move(block.urgent.value())}));
  }
  if (block.separator.has_value() == true) {
    bj_object.insert(std::make_pair(i3bar_protocol::block::k_separator_str,
                                    bool{std::move(block.separator.value())}));
  }
  if (block.separator_block_width.has_value() == true) {
    bj_object.insert(std::make_pair(
        i3bar_protocol::block::k_separator_block_width_str,
        std::int64_t{std::move(block.separator_block_width.value())}));
  }
  if (block.markup.has_value() == true) {
    bj_object.insert(
        std::make_pair(i3bar_protocol::block::k_markup_str,
                       boost::json::string{std::move(block.markup.value())}));
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
      .name{boost_json_string_to_std_string(
          bj_object.at(i3bar_protocol::click_event::k_name_str).as_string())},
      .instance{boost_json_string_to_std_string(
          bj_object.at(i3bar_protocol::click_event::k_instance_str)
              .as_string())},
      .x{bj_object.at(i3bar_protocol::click_event::k_x_str).as_int64()},
      .y{bj_object.at(i3bar_protocol::click_event::k_y_str).as_int64()},
      .button{static_cast<decltype(click_event.button)>(
          bj_object.at(i3bar_protocol::click_event::k_button_str).as_int64())},
      .relative_x{bj_object.at(i3bar_protocol::click_event::k_relative_x_str)
                      .as_int64()},
      .relative_y{bj_object.at(i3bar_protocol::click_event::k_relative_y_str)
                      .as_int64()},
      .output_x{
          bj_object.at(i3bar_protocol::click_event::k_output_x_str).as_int64()},
      .output_y{
          bj_object.at(i3bar_protocol::click_event::k_output_y_str).as_int64()},
      .width{bj_object.at(i3bar_protocol::click_event::k_width_str).as_int64()},
      .height{
          bj_object.at(i3bar_protocol::click_event::k_height_str).as_int64()},
      .modifiers{[&boost_json_string_to_std_string](
                     const boost::json::array &bj_array) {
        std::vector<std::string> ret_val{};
        ret_val.reserve(bj_array.size());
        for (auto p{bj_array.begin()}; p != bj_array.end(); ++p) {
          ret_val.emplace_back(boost_json_string_to_std_string(p->as_string()));
        }
        return ret_val;
      }(bj_object.at(i3bar_protocol::click_event::k_modifiers_str)
                     .as_array())}};

  return click_event;
}
