#include "module_handle.hpp"

#include "dyn_load_lib.hpp"
#include "generic_callback.hpp"
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

decltype(thread_comm::state_change_callback::func)
    module_handle::m_s_thread_comm_state_change_callback{
        [](void *userdata,
           thread_comm::shared_state_state::type state) -> void {
          state_change_callback *scc{
              static_cast<state_change_callback *>(userdata)};
          switch (state) {
          case thread_comm::shared_state_state::none: {
          } break;
          case thread_comm::shared_state_state::read: {
          } break;
          case thread_comm::shared_state_state::unread: {
            scc->call(state_change_type::new_block);
          } break;
          case thread_comm::shared_state_state::exception: {
            scc->call(state_change_type::new_exception);
          } break;
          default: {
          } break;
          }
        }};

module_handle::module_handle(const module_id::type id, std::string &&file_path,
                             libconfigfile::map_node &&conf,
                             state_change_callback &&state_change_callback)
    : m_id{id}, m_name{}, m_file_path{std::move(file_path)}, m_click_events{},
      m_state_change_callback{std::move(state_change_callback)},
      m_dyn_lib{m_file_path, dyn_load_lib::dlopen_flags::lazy},
      m_module{nullptr, nullptr}, m_thread_comm_producer{},
      m_thread_comm_consumer{}, m_thread{} {
  do_ctor(std::move(conf));
}

module_handle::module_handle(const module_id::type id,
                             const std::string &file_path,
                             const libconfigfile::map_node &conf,
                             const state_change_callback &state_change_callback)
    : m_id{id}, m_name{}, m_file_path{file_path}, m_click_events{},
      m_state_change_callback{state_change_callback},
      m_dyn_lib{m_file_path, dyn_load_lib::dlopen_flags::lazy},
      m_module{nullptr, nullptr}, m_thread_comm_producer{},
      m_thread_comm_consumer{}, m_thread{} {
  do_ctor(std::remove_cvref_t<decltype(conf)>{conf});
}

module_handle::module_handle(module_handle &&other) noexcept
    : m_id{other.m_id}, m_name{std::move(other.m_name)},
      m_file_path{std::move(other.m_file_path)},
      m_click_events{other.m_click_events},
      m_state_change_callback{std::move(other.m_state_change_callback)},
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
    m_click_events = other.m_click_events;
    m_state_change_callback = std::move(other.m_state_change_callback);
    m_dyn_lib = std::move(other.m_dyn_lib);
    m_module = std::move(other.m_module);
    m_thread_comm_producer = std::move(other.m_thread_comm_producer);
    m_thread_comm_consumer = std::move(other.m_thread_comm_consumer);
    m_thread = std::move(other.m_thread);
  }
  return *this;
}

void module_handle::do_ctor(libconfigfile::map_node &&conf) {
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

  thread_comm::t_pair<module_api::block> tc_pair{
      thread_comm::make_pair<module_api::block>(
          {m_s_thread_comm_state_change_callback,
           static_cast<void *>(&m_state_change_callback)},
          thread_comm::shared_state_state::unread |
              thread_comm::shared_state_state::exception)};
  m_thread_comm_producer = std::move(tc_pair.first);
  module_api mod_api{
      m_thread_comm_producer,
  };
  m_thread_comm_consumer = std::move(tc_pair.second);

  try {
    module_api::config_out conf_out{
        m_module->init(std::move(mod_api), std::move(conf))};
    m_name = std::move(conf_out.name);
    m_click_events = conf_out.click_events;

    if (m_name.empty()) {
      throw module_error{m_id, m_name, m_file_path, "empty name"};
    } else if (m_name.find_first_not_of(
                   module_api::config_out::k_valid_name_chars) !=
               std::string::npos) {
      throw module_error{m_id, m_name, m_file_path,
                         "invalid character in name"};
    }
  } catch (const std::exception &ex) {
    throw module_error{m_id, "UNKNOWN", m_file_path, ex.what()};
  } catch (...) {
    throw module_error{m_id, "UNKNOWN", m_file_path, "UNKNOWN"};
  }
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

module_id::type module_handle::get_id() const { return m_id; }

const std::string &module_handle::get_name() const { return m_name; }

const std::string &module_handle::get_file_path() const { return m_file_path; }

thread_comm::consumer<module_api::block> &module_handle::get_comm() {
  return m_thread_comm_consumer;
}
