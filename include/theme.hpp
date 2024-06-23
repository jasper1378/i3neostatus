#ifndef I3NEOSTATUS_THEME_HPP
#define I3NEOSTATUS_THEME_HPP

#include "block_state.hpp"
#include "i3bar_data.hpp"

#include <variant>

namespace i3neostatus {

struct theme {
  using t_color = i3bar_data::types::color;

  enum class special_separator_color {
    left,
    right,
  };

  enum class special_border_color {
    foreground,
    background,
  };

  t_color idle_color_foreground;
  t_color idle_color_background;
  std::variant<t_color, special_border_color> idle_color_border;

  t_color info_color_foreground;
  t_color info_color_background;
  std::variant<t_color, special_border_color> info_color_border;

  t_color good_color_foreground;
  t_color good_color_background;
  std::variant<t_color, special_border_color> good_color_border;

  t_color warning_color_foreground;
  t_color warning_color_background;
  std::variant<t_color, special_border_color> warning_color_border;

  t_color critical_color_foreground;
  t_color critical_color_background;
  std::variant<t_color, special_border_color> critical_color_border;

  t_color error_color_foreground;
  t_color error_color_background;
  std::variant<t_color, special_border_color> error_color_border;

  t_color alternating_tint_color_foreground;
  t_color alternating_tint_color_background;
  std::variant<t_color, special_border_color> alternating_tint_border;

  std::string separator_seq; // TODO unicode
  std::variant<t_color, special_separator_color> separator_color_foreground;
  std::variant<t_color, special_separator_color> separator_color_background;

  std::string separator_seq_begin; // TODO unicode
  std::variant<t_color, special_separator_color>
      separator_begin_color_foreground;
  std::variant<t_color, special_separator_color>
      separator_begin_color_background;

  std::string separator_seq_end; // TODO unicode
  std::variant<t_color, special_separator_color> separator_end_color_foreground;
  std::variant<t_color, special_separator_color> separator_end_color_background;

  i3bar_data::types::pixel_count_t border_width_top;
  i3bar_data::types::pixel_count_t border_width_right;
  i3bar_data::types::pixel_count_t border_width_bottom;
  i3bar_data::types::pixel_count_t border_width_left;

  i3bar_data::block::content::theme set_block(const block_state state) {
    switch (state) {
    case block_state::idle: {
    } break;
    case block_state::info: {
    } break;
    case block_state::good: {
    } break;
    case block_state::warning: {
    } break;
    case block_state::critical: {
    } break;
    case block_state::error: {
    } break;
    }
  }
  // TODO
};
} // namespace i3neostatus

#endif
