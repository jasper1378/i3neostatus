#ifndef MODULE_API_HPP
#define MODULE_API_HPP

#include "i3bar_protocol.hpp"
#include "thread_comm.hpp"

#include "libconfigfile.hpp"

#include <exception>
#include <memory>
#include <optional>
#include <string>

class module_api {
public:
  using config_in = libconfigfile::map_node;

  struct config_out {
    std::string m_name;
    bool click_events_enabled;
  };

  struct block {
    using pixel_count_t = i3bar_protocol::pixel_count_t;
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

  struct click_event {
    using pixel_count_t = i3bar_protocol::pixel_count_t;
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

private:
  thread_comm::producer<block> m_thread_comm_producer;

public:
  module_api();

  module_api(thread_comm::producer<block> &&thread_comm_producer);

  module_api(module_api &&other) noexcept;

  module_api(const module_api &other) = delete;

  ~module_api();

  module_api &operator=(module_api &&other) noexcept;

  module_api &operator=(const module_api &other) = delete;

public:
  template <typename... t_args>
  std::unique_ptr<block> make_block(t_args &&...args) {
    return std::make_unique<block>(
        std::forward<t_args>(args)...); // TODO reuse buffer
  }

  void set_block(std::unique_ptr<block> block);

  void set_error(std::exception_ptr error);

  void hide();
};

#endif
