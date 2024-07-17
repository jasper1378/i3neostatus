#ifndef I3NEOSTATUS_I3BAR_DATA_HPP
#define I3NEOSTATUS_I3BAR_DATA_HPP

#include "plugin_id.hpp"

#include "bits-and-bytes/enum_flag_operators.hpp"
#include "libconfigfile/color.hpp"

#include <optional>
#include <string>
#include <variant>

namespace i3neostatus {
namespace i3bar_data {
namespace types {
using pixel_count_t = long;
using color = libconfigfile::color::rgba;

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
BITS_AND_BYTES_DEFINE_ENUM_FLAG_OPERATORS_FOR_TYPE(click_modifiers);
} // namespace types

struct header {
  int version;
  int stop_signal;
  int cont_signal;
  bool click_events;
};

struct block {
  struct id {
    std::string name;
    plugin_id::type instance{plugin_id::null};
  };

  struct data {
    struct program {
      struct global {
        bool separator;
        types::pixel_count_t separator_block_width{9};
      };

      struct theme {
        types::color color;
        types::color background;
        types::color border;
        types::pixel_count_t border_top;
        types::pixel_count_t border_right;
        types::pixel_count_t border_bottom;
        types::pixel_count_t border_left;
      };

      struct global global;
      struct theme theme;
    };

    struct plugin {
      std::string full_text;
      std::optional<std::string> short_text;
      std::optional<std::variant<types::pixel_count_t, std::string>> min_width;
      std::optional<types::text_align> align;
      std::optional<bool> urgent;
      std::optional<types::markup> markup;
    };

    struct program program;
    struct plugin plugin;
  };

  struct id id;
  struct data data;
};

struct click_event {
  struct id {
    std::string name;
    plugin_id::type instance;
  };

  struct data {
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

  struct id id;
  struct data data;
};

} // namespace i3bar_data

} // namespace i3neostatus
#endif
