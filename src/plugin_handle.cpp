#include "plugin_handle.hpp"

#include "generic_callback.hpp"
#include "plugin_api.hpp"
#include "plugin_base.hpp"
#include "plugin_error.hpp"
#include "plugin_id.hpp"
#include "thread_comm.hpp"

#include "libconfigfile/libconfigfile.hpp"

#include <filesystem>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <variant>

const decltype(i3neostatus::thread_comm::state_change_callback::func)
    i3neostatus::plugin_handle::m_k_thread_comm_state_change_callback{
        [](void *userdata, thread_comm::shared_state_state state) -> void {
          state_change_callback *scc{
              static_cast<state_change_callback *>(userdata)};
          switch (state) {
          case thread_comm::shared_state_state::null:
          case thread_comm::shared_state_state::empty:
          default:
            break;
          case thread_comm::shared_state_state::value: {
            scc->call(state_change_type::new_block);
          } break;
          case thread_comm::shared_state_state::exception: {
            scc->call(state_change_type::new_exception);
          } break;
          }
        }};

const i3neostatus::thread_comm::shared_state_state
    i3neostatus::plugin_handle::m_k_state_change_subscribed_events{
        thread_comm::shared_state_state::value |
        thread_comm::shared_state_state::exception};

i3neostatus::plugin_handle::plugin_handle(
    const plugin_id::type id,
    std::variant<std::filesystem::path, std::string> &&path_or_name,
    libconfigfile::map_node &&conf,
    state_change_callback &&state_change_callback)
    : m_id{id}, m_path_or_name{std::move(path_or_name)},
      m_click_events_enabled{},
      m_state_change_callback{std::move(state_change_callback)},
      m_plugin{m_path_or_name, m_id},
      m_thread_comm_producer{
          thread_comm::make<plugin_api::block, thread_comm::producer>(
              {m_k_thread_comm_state_change_callback,
               static_cast<void *>(&m_state_change_callback)},
              m_k_state_change_subscribed_events)},
      m_thread_comm_consumer{thread_comm::make_from<thread_comm::consumer>(
          m_thread_comm_producer)},
      m_thread_comm_producer_plugin{
          thread_comm::make_from<thread_comm::producer>(
              m_thread_comm_consumer)},
      m_plugin_api{&m_thread_comm_producer_plugin}, m_plugin_thread{} {
  do_ctor(std::move(conf));
}

i3neostatus::plugin_handle::plugin_handle(plugin_handle &&other) noexcept
    : m_id{other.m_id}, m_path_or_name{std::move(other.m_path_or_name)},
      m_click_events_enabled{other.m_click_events_enabled},
      m_state_change_callback{std::move(other.m_state_change_callback)},
      m_plugin{std::move(other.m_plugin)},
      m_thread_comm_producer{std::move(other.m_thread_comm_producer)},
      m_thread_comm_consumer{std::move(other.m_thread_comm_consumer)},
      m_thread_comm_producer_plugin{
          std::move(other.m_thread_comm_producer_plugin)},
      m_plugin_api{std::move(other.m_plugin_api)},
      m_plugin_thread{std::move(other.m_plugin_thread)} {}

i3neostatus::plugin_handle::~plugin_handle() {
  try {
    m_plugin.get().term();
  } catch (const std::exception &ex) {
    m_thread_comm_producer.put_exception(
        std::make_exception_ptr(plugin_error{m_id, m_path_or_name, ex.what()}));
  } catch (...) {
    m_thread_comm_producer.put_exception(
        std::make_exception_ptr(plugin_error{m_id, m_path_or_name, "UNKNOWN"}));
  }
  m_plugin_thread.join();
}

i3neostatus::plugin_handle &
i3neostatus::plugin_handle::operator=(plugin_handle &&other) noexcept {
  if (this != &other) {
    m_id = other.m_id;
    m_path_or_name = std::move(other.m_path_or_name);
    m_click_events_enabled = other.m_click_events_enabled;
    m_state_change_callback = std::move(other.m_state_change_callback);
    m_plugin = std::move(other.m_plugin);
    m_thread_comm_producer = std::move(other.m_thread_comm_producer);
    m_thread_comm_consumer = std::move(other.m_thread_comm_consumer);
    m_thread_comm_producer_plugin =
        std::move(other.m_thread_comm_producer_plugin);
    m_plugin_api = std::move(other.m_plugin_api);
    m_plugin_thread = std::move(other.m_plugin_thread);
  }
  return *this;
}

void i3neostatus::plugin_handle::do_ctor(libconfigfile::map_node &&conf) {
  const plugin_api plugin_api{&m_thread_comm_producer_plugin};

  try {
    plugin_api::config_out conf_out{
        m_plugin.get().init(&m_plugin_api, std::move(conf))};
    m_click_events_enabled = conf_out.click_events_enabled;
  } catch (const std::exception &ex) {
    throw plugin_error{m_id, m_path_or_name, ex.what()};
  } catch (...) {
    throw plugin_error{m_id, m_path_or_name, "UNKNOWN"};
  }
}

void i3neostatus::plugin_handle::run() {
  m_plugin_thread = std::thread{[this]() {
    try {
      m_plugin.get().run();
    } catch (const std::exception &ex) {
      m_thread_comm_producer.put_exception(std::make_exception_ptr(
          plugin_error{m_id, m_path_or_name, ex.what()}));
    } catch (...) {
      m_thread_comm_producer.put_exception(std::make_exception_ptr(
          plugin_error{m_id, m_path_or_name, "UNKNOWN"}));
    }
  }};
}

void i3neostatus::plugin_handle::send_click_event(
    plugin_api::click_event &&click_event) {
  try {
    m_plugin.get().on_click_event(std::move(click_event));
  } catch (const std::exception &ex) {
    m_thread_comm_producer.put_exception(
        std::make_exception_ptr(plugin_error{m_id, m_path_or_name, ex.what()}));
  } catch (...) {
    m_thread_comm_producer.put_exception(
        std::make_exception_ptr(plugin_error{m_id, m_path_or_name, "UNKNOWN"}));
  }
}

i3neostatus::plugin_id::type i3neostatus::plugin_handle::get_id() const {
  return m_id;
}

const std::variant<std::filesystem::path, std::string> &
i3neostatus::plugin_handle::get_path_or_name() const {
  return m_path_or_name;
}

bool i3neostatus::plugin_handle::get_click_events_enabled() const {
  return m_click_events_enabled;
}

i3neostatus::thread_comm::consumer<i3neostatus::plugin_api::block> &
i3neostatus::plugin_handle::get_comm() {
  return m_thread_comm_consumer;
}
