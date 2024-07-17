#ifndef I3NEOSTATUS_PLUGIN_API_HPP
#define I3NEOSTATUS_PLUGIN_API_HPP

#include "block_state.hpp"
#include "i3bar_data.hpp"

#include "libconfigfile/libconfigfile.hpp"

#include <exception>
#include <string>
#include <utility>

namespace i3neostatus {

namespace thread_comm {
template <typename t_value> class producer;
}

class plugin_api {
public:
  using config_in = libconfigfile::map_node;

  struct config_out {
    std::string name;
    bool click_events_enabled;

    static const std::string k_valid_name_chars;
  };

  using content = struct i3bar_data::block::data::plugin;
  using block = std::pair<content, block_state>;
  using click_event = struct i3bar_data::click_event::data;

private:
  thread_comm::producer<block> *m_thread_comm_producer;

public:
  plugin_api(thread_comm::producer<block> *thread_comm_producer);
  plugin_api(plugin_api &&other) noexcept;
  plugin_api(const plugin_api &other) = delete;

  ~plugin_api();

  plugin_api &operator=(plugin_api &&other) noexcept;
  plugin_api &operator=(const plugin_api &other) = delete;

public:
  void put_block(const block &block);
  void put_block(block &&block);

  void put_error(const std::exception_ptr &error);
  void put_error(std::exception_ptr &&error);
  void put_error(const std::exception &error);
  void put_error(std::exception &&error);

  void hide();
};

} // namespace i3neostatus
#endif
