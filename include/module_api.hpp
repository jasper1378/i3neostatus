#ifndef MODULE_API_HPP
#define MODULE_API_HPP

#include "i3bar_protocol.hpp"
#include "thread_comm.hpp"

#include "libconfigfile.hpp"

#include <exception>
#include <memory>
#include <string>

class module_api {
public:
  using config_in = libconfigfile::map_node;

  struct config_out {
    std::string m_name;
    bool click_events_enabled;
  };

  using block = struct i3bar_protocol::block::content;

  using click_event = struct i3bar_protocol::click_event::content;

  struct runtime_settings {
    std::atomic<bool> hidden;

    runtime_settings(std::atomic<bool> hidden = {}) : hidden{hidden.load()} {};

    runtime_settings(const runtime_settings &other);
    runtime_settings(runtime_settings &&other) noexcept;

    ~runtime_settings() = default;

    runtime_settings &operator=(const runtime_settings &other);
    runtime_settings &operator=(runtime_settings &&other) noexcept;
  };

private:
  thread_comm::producer<block> m_thread_comm_producer;
  runtime_settings *m_runtime_settings;

public:
  module_api();

  module_api(const thread_comm::producer<block> &thread_comm_producer,
             runtime_settings *runtime_settings);

  module_api(thread_comm::producer<block> &&thread_comm_producer,
             runtime_settings *runtime_settings);

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
