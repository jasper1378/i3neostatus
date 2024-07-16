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

static_assert(block_state::idle < block_state::info &&
              block_state::info < block_state::good &&
              block_state::good < block_state::warning &&
              block_state::warning < block_state::critical &&
              block_state::critical < block_state::error);
static constexpr theme k_default{
    .state_dependent_color_foreground{
        color{{0xff, 0xff, 0xff}, 0xff}, // idle
        color{{0xff, 0xff, 0xff}, 0xff}, // info
        color{{0xff, 0xff, 0xff}, 0xff}, // good
        color{{0xff, 0xff, 0xff}, 0xff}, // warning
        color{{0xff, 0xff, 0xff}, 0xff}, // critical
        color{{0xff, 0xff, 0xff}, 0xff}, // error
    },
    .state_dependent_color_background{
        color{{0x00, 0x00, 0x00}, 0xff}, // idle
        color{{0x00, 0x00, 0x00}, 0xff}, // info
        color{{0x00, 0x00, 0x00}, 0xff}, // good
        color{{0x00, 0x00, 0x00}, 0xff}, // warning
        color{{0x00, 0x00, 0x00}, 0xff}, // critical
        color{{0x00, 0x00, 0x00}, 0xff}, // error
    },
    .state_dependent_color_border{
        special_border_color{special_border_color::background}, // idle
        special_border_color{special_border_color::background}, // info
        special_border_color{special_border_color::background}, // good
        special_border_color{special_border_color::background}, // warning
        special_border_color{special_border_color::background}, // critical
        special_border_color{special_border_color::background}, // error
    },

    .alternating_tint_color_foreground{color{{0x00, 0x00, 0x00}, 0x00}},
    .alternating_tint_color_background{color{{0x00, 0x00, 0x00}, 0x00}},
    .alternating_tint_color_border{color{{0x00, 0x00, 0x00}, 0x00}},

    .separator_middle_sequence{" | "},
    .separator_middle_color_foreground{color{{0xff, 0xff, 0xff}, 0xff}},
    .separator_middle_color_background{color{{0x00, 0x00, 0x00}, 0xff}},

    .separator_begin_sequence{""},
    .separator_begin_color_foreground{color{{0xff, 0xff, 0xff}, 0xff}},
    .separator_begin_color_background{color{{0x00, 0x00, 0x00}, 0xff}},

    .separator_end_sequence{""},
    .separator_end_color_foreground{color{{0xff, 0xff, 0xff}, 0xff}},
    .separator_end_color_background{color{{0x00, 0x00, 0x00}, 0xff}},

    .border_width_top{1},
    .border_width_right{1},
    .border_width_bottom{1},
    .border_width_left{1},
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
