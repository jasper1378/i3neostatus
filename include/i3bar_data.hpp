#ifndef I3NEOSTATUS_I3BAR_DATA_HPP
#define I3NEOSTATUS_I3BAR_DATA_HPP

#include "module_id.hpp"

#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace i3neostatus {

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
  };
  using struct_content = struct content;

  id id;
  content content;
};

struct click_event {
  struct id {
    std::string name;
    module_id::type instance;
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
  };
  using struct_content = struct content;

  struct id id;
  struct content content;
};

} // namespace i3bar_data

} // namespace i3neostatus
#endif
