#ifndef I3NEOSTATUS_THEME_HPP
#define I3NEOSTATUS_THEME_HPP

#include "block_state.hpp"
#include "i3bar_data.hpp"

#include <array>
#include <cstddef>
#include <string>
#include <variant>

namespace i3neostatus {
namespace theme {
using pixel_count_t = i3bar_data::types::pixel_count_t;
using color = i3bar_data::types::color;

enum class special_border_color {
  foreground,
  background,
};

enum class special_separator_color_begin { right = 0 };
enum class special_separator_color_end { left = 1 };
enum class special_separator_color {
  right = static_cast<int>(special_separator_color_begin::right),
  left = static_cast<int>(special_separator_color_end::left)
};

enum class separator_type {
  begin,
  middle,
  end,
};

struct theme {
  std::array<color, static_cast<std::size_t>(block_state::max)>
      state_dependent_color_foreground;
  std::array<color, static_cast<std::size_t>(block_state::max)>
      state_dependent_color_background;
  std::array<std::variant<color, special_border_color>,
             static_cast<std::size_t>(block_state::max)>
      state_dependent_color_border;

  color alternating_tint_color_foreground;
  color alternating_tint_color_background;
  std::variant<color, special_border_color> alternating_tint_color_border;

  std::string separator_middle_sequence; // TODO unicode
  std::variant<color, special_separator_color>
      separator_middle_color_foreground;
  std::variant<color, special_separator_color>
      separator_middle_color_background;

  std::string separator_begin_sequence; // TODO unicode
  std::variant<color, special_separator_color_begin>
      separator_begin_color_foreground;
  std::variant<color, special_separator_color_begin>
      separator_begin_color_background;

  std::string separator_end_sequence; // TODO unicode
  std::variant<color, special_separator_color_end>
      separator_end_color_foreground;
  std::variant<color, special_separator_color_end>
      separator_end_color_background;

  pixel_count_t border_width_top;
  pixel_count_t border_width_right;
  pixel_count_t border_width_bottom;
  pixel_count_t border_width_left;
};

namespace impl {
template <separator_type type>
struct separator_type_enum_to_special_separator_color_enum;
template <>
struct separator_type_enum_to_special_separator_color_enum<
    separator_type::begin> {
  using type = special_separator_color_begin;
};
template <>
struct separator_type_enum_to_special_separator_color_enum<
    separator_type::end> {
  using type = special_separator_color_end;
};
template <>
struct separator_type_enum_to_special_separator_color_enum<
    separator_type::middle> {
  using type = special_separator_color;
};
template <separator_type type>
using separator_type_enum_to_special_separator_color_enum_t =
    separator_type_enum_to_special_separator_color_enum<type>::type;
} // namespace impl
} // namespace theme
} // namespace i3neostatus

#endif
