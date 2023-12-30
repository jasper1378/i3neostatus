#include "module_handle.hpp"

#include "dyn_load_lib.hpp"
#include "module_api.hpp"
#include "module_base.hpp"
#include "module_error.hpp"
#include "module_id.hpp"
#include "thread_comm.hpp"

#include "libconfigfile.hpp"

#include <memory>
#include <string>
#include <thread>
#include <utility>

module_handle::module_handle(const module_id::type id, std::string &&file_path,
                             libconfigfile::map_node &&conf,
                             module_api::runtime_settings &&runtime_settings)
    : module_handle{id, std::move(file_path), std::move(conf),
                    std::move(runtime_settings),
                    thread_comm::make_pair<module_api::block>()} {}

module_handle::module_handle(
    const module_id::type id, const std::string &file_path,
    const libconfigfile::map_node &conf,
    const module_api::runtime_settings &runtime_settings)
    : module_handle{id, file_path, conf, runtime_settings,
                    thread_comm::make_pair<module_api::block>()} {}

module_handle::
    module_handle(const module_id::type id, std::string &&file_path,
                  libconfigfile::map_node &&conf,
                  module_api::runtime_settings &&runtime_settings,
                  const thread_comm::state_change_callback
                      state_change_callback,
                  const thread_comm::
                      shared_state_state::type state_change_subscribed_events /*= thread_comm::shared_state_state::all*/)
    : module_handle{
          id, std::move(file_path), std::move(conf),
          std::move(runtime_settings),
          thread_comm::make_pair<module_api::block>(
              state_change_callback, state_change_subscribed_events)} {}

module_handle::
    module_handle(const module_id::type id, const std::string &file_path,
                  const libconfigfile::map_node &conf,
                  const module_api::runtime_settings &runtime_settings,
                  const thread_comm::state_change_callback
                      state_change_callback,
                  const thread_comm::
                      shared_state_state::type state_change_subscribed_events /*= thread_comm::shared_state_state::all*/)
    : module_handle{
          id, file_path, conf, runtime_settings,
          thread_comm::make_pair<module_api::block>(
              state_change_callback, state_change_subscribed_events)} {}

module_handle::module_handle(const module_id::type id, std::string &&file_path,
                             libconfigfile::map_node &&conf,
                             module_api::runtime_settings &&runtime_settings,
                             thread_comm::t_pair<module_api::block> &&tc_pair)
    : m_id{id}, m_name{}, m_file_path{std::move(file_path)},
      m_click_events_enabled{false},
      m_runtime_settings{std::move(runtime_settings)},
      m_dyn_lib{m_file_path, dyn_load_lib::dlopen_flags::lazy},
      m_module{nullptr, nullptr}, m_thread_comm_producer{},
      m_thread_comm_consumer{}, m_thread{} {
  do_ctor(std::move(conf), std::move(tc_pair));
}

module_handle::module_handle(
    const module_id::type id, const std::string &file_path,
    const libconfigfile::map_node &conf,
    const module_api::runtime_settings &runtime_settings,
    thread_comm::t_pair<module_api::block> &&tc_pair)
    : m_id{id}, m_name{}, m_file_path{std::move(file_path)},
      m_click_events_enabled{false},
      m_runtime_settings{std::move(runtime_settings)},
      m_dyn_lib{m_file_path, dyn_load_lib::dlopen_flags::lazy},
      m_module{nullptr, nullptr}, m_thread_comm_producer{},
      m_thread_comm_consumer{}, m_thread{} {
  do_ctor(libconfigfile::map_node{conf}, std::move(tc_pair));
}

module_handle::module_handle(module_handle &&other) noexcept
    : m_id{other.m_id}, m_name{std::move(other.m_name)},
      m_file_path{std::move(other.m_file_path)},
      m_click_events_enabled{other.m_click_events_enabled},
      m_runtime_settings{std::move(other.m_runtime_settings)},
      m_dyn_lib{std::move(other.m_dyn_lib)},
      m_module{std::move(other.m_module)},
      m_thread_comm_producer{std::move(other.m_thread_comm_producer)},
      m_thread_comm_consumer{std::move(other.m_thread_comm_consumer)},
      m_thread{std::move(other.m_thread)} {}

module_handle::~module_handle() {
  m_module->term();
  m_thread.join();
}

module_handle &module_handle::operator=(module_handle &&other) noexcept {
  if (this != &other) {
    m_id = other.m_id;
    m_name = std::move(other.m_name);
    m_file_path = std::move(other.m_file_path);
    m_click_events_enabled = other.m_click_events_enabled;
    m_dyn_lib = std::move(other.m_dyn_lib);
    m_module = std::move(other.m_module);
    m_thread_comm_producer = std::move(other.m_thread_comm_producer);
    m_thread_comm_consumer = std::move(other.m_thread_comm_consumer);
    m_thread = std::move(other.m_thread);
  }
  return *this;
}

void module_handle::do_ctor(libconfigfile::map_node &&conf,
                            thread_comm::t_pair<module_api::block> &&tc_pair) {
  module_base::allocator_func_ptr_t mod_alloc{
      m_dyn_lib.get_symbol<module_base::allocator_func_t>(
          module_base::allocator_func_str)};
  module_base::deleter_func_ptr_t mod_delete{
      m_dyn_lib.get_symbol<module_base::deleter_func_t>(
          module_base::deleter_func_str)};
  m_module = {mod_alloc(), mod_delete};
  if (!m_module) {
    throw module_error{m_id, "UNKNOWN", m_file_path, "allocator() failed"};
  }

  m_thread_comm_producer = std::move(tc_pair.first);
  module_api mod_api{m_thread_comm_producer, &m_runtime_settings};
  m_thread_comm_consumer = std::move(tc_pair.second);

  try {
    module_api::config_out conf_out{
        m_module->init(std::move(mod_api), std::move(conf))};
    m_name = std::move(conf_out.m_name);

    if (m_name.empty()) {
      throw module_error{m_id, m_name, m_file_path, "empty name"};
    } else if (m_name.find_first_not_of(
                   module_api::config_out::k_valid_name_chars) !=
               std::string::npos) {
      throw module_error{m_id, m_name, m_file_path,
                         "invalid character in name"};
    }
    m_click_events_enabled = conf_out.click_events_enabled;
  } catch (const std::exception &ex) {
    throw module_error{m_id, "UNKNOWN", m_file_path, ex.what()};
  } catch (...) {
    throw module_error{m_id, "UNKNOWN", m_file_path, "UNKNOWN"};
  }
}

std::string module_handle::get_name() const { return m_name; }

std::string module_handle::get_file_path() const { return m_file_path; }

bool module_handle::get_click_events_enabled() const {
  return m_click_events_enabled;
}

module_api::runtime_settings &module_handle::get_runtime_settings() {
  return m_runtime_settings;
}

void module_handle::run() {
  m_thread = std::thread{[this]() {
    try {
      m_module->run();
    } catch (const std::exception &ex) {
      m_thread_comm_producer.put_exception(std::make_exception_ptr(
          module_error{m_id, m_name, m_file_path, ex.what()}));
    } catch (...) {
      m_thread_comm_producer.put_exception(std::make_exception_ptr(
          module_error{m_id, m_name, m_file_path, "UNKNOWN"}));
    }
  }};
}

thread_comm::consumer<module_api::block> &module_handle::get_comm() {
  return m_thread_comm_consumer;
}
