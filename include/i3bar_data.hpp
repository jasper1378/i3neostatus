#ifndef I3NEOSTATUS_I3BAR_DATA_HPP
#define I3NEOSTATUS_I3BAR_DATA_HPP

#include "color.hpp"
#include "define_enum_flag_operators.hpp"
#include "module_id.hpp"

#include <optional>
#include <string>
#include <variant>

namespace i3neostatus {

namespace i3bar_data {
namespace types {
using pixel_count_t = long;

using color = color::rgb;

enum class text_align {
  none = 0,
  center = 1,
  right = 2,
  left = 3,
  max = 4,
};

enum class markup {
  none = 0,
  pango = 1,
  max = 2,
};

enum class click_modifiers {
  none = 0b00000000,
  mod1 = 0b00000001,
  mod2 = 0b00000010,
  mod3 = 0b00000100,
  mod4 = 0b00001000,
  mod5 = 0b00010000,
  shift = 0b00100000,
  control = 0b01000000,
  lock = 0b10000000,
};
DEFINE_ENUM_FLAG_OPERATORS_FOR_TYPE(click_modifiers);
} // namespace types

struct header {
  int version;
  std::optional<int> stop_signal;
  std::optional<int> cont_signal;
  std::optional<bool> click_events;
};

struct block {
  struct id {
    std::optional<std::string> name;
    std::optional<module_id::type> instance;
  };
  using struct_id = struct id;

  struct content {
    struct theme {
      std::optional<types::color> color;
      std::optional<types::color> background;
      std::optional<types::color> border;
      std::optional<types::pixel_count_t> border_top;
      std::optional<types::pixel_count_t> border_right;
      std::optional<types::pixel_count_t> border_bottom;
      std::optional<types::pixel_count_t> border_left;
    };
    using struct_theme = struct theme;

    struct global {
      std::optional<bool> separator;
      std::optional<types::pixel_count_t> separator_block_width;
    };
    using struct_global = struct global;

    struct local {
      struct theme theme;
      std::string full_text;
      std::optional<std::string> short_text;
      std::optional<std::variant<types::pixel_count_t, std::string>> min_width;
      std::optional<types::text_align> align;
      std::optional<bool> urgent;
      std::optional<types::markup> markup;
    };
    using struct_local = struct local;

    struct global global;
    struct local local;
  };
  using struct_content = struct content;

  struct id id;
  struct content content;
};

struct click_event {
  struct id {
    std::string name;
    module_id::type instance;
  };
  using struct_id = struct id;

  struct content {
    types::pixel_count_t x;
    types::pixel_count_t y;
    int button;
    types::pixel_count_t relative_x;
    types::pixel_count_t relative_y;
    types::pixel_count_t output_x;
    types::pixel_count_t output_y;
    types::pixel_count_t width;
    types::pixel_count_t height;
    types::click_modifiers modifiers;
  };
  using struct_content = struct content;

  struct id id;
  struct content content;
};

} // namespace i3bar_data

} // namespace i3neostatus
#endif
