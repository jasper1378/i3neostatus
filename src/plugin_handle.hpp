#ifndef I3NEOSTATUS_PLUGIN_HANDLE_HPP
#define I3NEOSTATUS_PLUGIN_HANDLE_HPP

#include "plugin_api.hpp"
#include "plugin_base.hpp"
#include "plugin_id.hpp"
#include "plugin_loader.hpp"
#include "thread_comm.hpp"

#include "bits-and-bytes/generic_callback.hpp"
#include "libconfigfile/libconfigfile.hpp"

#include <filesystem>
#include <memory>
#include <string>
#include <thread>
#include <variant>

namespace i3neostatus {

class plugin_handle {
public:
  enum class state_change_type {
    new_block,
    new_exception,
  };

  using state_change_callback =
      bits_and_bytes::generic_callback<state_change_type>;

private:
  plugin_id::type m_id;
  std::variant<std::filesystem::path, std::string> m_path_or_name;
  bool m_click_events_enabled;
  state_change_callback m_state_change_callback;
  plugin_loader m_plugin;
  thread_comm::producer<plugin_api::block> m_thread_comm_producer;
  thread_comm::consumer<plugin_api::block> m_thread_comm_consumer;
  thread_comm::producer<plugin_api::block> m_thread_comm_producer_plugin;
  plugin_api m_plugin_api;
  std::thread m_plugin_thread;

private:
  static const decltype(thread_comm::state_change_callback::func)
      m_k_thread_comm_state_change_callback;
  static const thread_comm::shared_state_state
      m_k_state_change_subscribed_events;

public:
  plugin_handle(const plugin_id::type id,
                std::variant<std::filesystem::path, std::string> &&path_or_name,
                libconfigfile::map_node &&conf,
                state_change_callback &&state_change_callback);
  plugin_handle(plugin_handle &&other) noexcept;
  plugin_handle(const plugin_handle &other) = delete;

public:
  ~plugin_handle();

public:
  plugin_handle &operator=(plugin_handle &&other) noexcept;
  plugin_handle &operator=(const plugin_handle &other) = delete;

private:
  void do_ctor(libconfigfile::map_node &&conf);

public:
  void run();

  void send_click_event(plugin_api::click_event &&click_event);

  plugin_id::type get_id() const;
  const std::variant<std::filesystem::path, std::string> &
  get_path_or_name() const;
  bool get_click_events_enabled() const;

  thread_comm::consumer<plugin_api::block> &get_comm();
};

} // namespace i3neostatus
#endif
