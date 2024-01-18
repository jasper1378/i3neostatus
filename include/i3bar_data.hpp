#ifndef I3NEOSTATUS_I3BAR_DATA_HPP
#define I3NEOSTATUS_I3BAR_DATA_HPP

#include "module_id.hpp"

#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace i3bar_data {
using pixel_count_t = long;

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

    static constexpr std::string k_name_str{"name"};
    static constexpr std::string k_instance_str{"instance"};
  };
  using struct_id = struct id;

  struct content {
    std::string full_text;
    std::optional<std::string> short_text;
    std::optional<std::string> color;
    std::optional<std::string> background;
    std::optional<std::string> border;
    std::optional<pixel_count_t> border_top;
    std::optional<pixel_count_t> border_right;
    std::optional<pixel_count_t> border_bottom;
    std::optional<pixel_count_t> border_left;
    std::optional<std::variant<pixel_count_t, std::string>> min_width;
    std::optional<std::string> align;
    std::optional<bool> urgent;
    std::optional<bool> separator;
    std::optional<pixel_count_t> separator_block_width;
    std::optional<std::string> markup;

    static constexpr std::string k_full_text_str{"full_text"};
    static constexpr std::string k_short_text_str{"short_text"};
    static constexpr std::string k_color_str{"color"};
    static constexpr std::string k_background_str{"background"};
    static constexpr std::string k_border_str{"border"};
    static constexpr std::string k_border_top_str{"border_top"};
    static constexpr std::string k_border_right_str{"border_right"};
    static constexpr std::string k_border_bottom_str{"border_bottom"};
    static constexpr std::string k_border_left_str{"border_left"};
    static constexpr std::string k_min_width_str{"min_width"};
    static constexpr std::string k_align_str{"align"};
    static constexpr std::string k_urgent_str{"urgent"};
    static constexpr std::string k_separator_str{"separator"};
    static const std::string k_separator_block_width_str;
    static constexpr std::string k_markup_str{"markup"};
  };
  using struct_content = struct content;

  id id;
  content content;
};

struct click_event {
  struct id {
    std::string name;
    module_id::type instance;

    static constexpr std::string k_name_str{"name"};
    static constexpr std::string k_instance_str{"instance"};
  };
  using struct_id = struct id;

  struct content {
    pixel_count_t x;
    pixel_count_t y;
    int button;
    pixel_count_t relative_x;
    pixel_count_t relative_y;
    pixel_count_t output_x;
    pixel_count_t output_y;
    pixel_count_t width;
    pixel_count_t height;
    std::vector<std::string> modifiers;

    static constexpr std::string k_x_str{"x"};
    static constexpr std::string k_y_str{"y"};
    static constexpr std::string k_button_str{"button"};
    static constexpr std::string k_relative_x_str{"relative_x"};
    static constexpr std::string k_relative_y_str{"relative_y"};
    static constexpr std::string k_output_x_str{"output_x"};
    static constexpr std::string k_output_y_str{"output_y"};
    static constexpr std::string k_width_str{"width"};
    static constexpr std::string k_height_str{"height"};
    static constexpr std::string k_modifiers_str{"modifiers"};
  };
  using struct_content = struct content;

  struct id id;
  struct content content;
};

} // namespace i3bar_data

#endif
