#include "module_handle.hpp"

#include "dyn_load_lib.hpp"
#include "generic_callback.hpp"
#include "module_api.hpp"
#include "module_base.hpp"
#include "module_error.hpp"
#include "module_id.hpp"
#include "thread_comm.hpp"

#include "libconfigfile/libconfigfile.hpp"

#include <memory>
#include <string>
#include <thread>
#include <utility>

const decltype(i3neostatus::thread_comm::state_change_callback::func)
    i3neostatus::module_handle::m_k_thread_comm_state_change_callback{
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
    i3neostatus::module_handle::m_k_state_change_subscribed_events{
        thread_comm::shared_state_state::value |
        thread_comm::shared_state_state::exception};

i3neostatus::module_handle::module_handle(
    const module_id::type id, std::string &&file_path,
    libconfigfile::map_node &&conf,
    state_change_callback &&state_change_callback)
    : m_id{id}, m_name{}, m_file_path{std::move(file_path)},
      m_click_events_enabled{},
      m_state_change_callback{std::move(state_change_callback)},
      m_dyn_lib{m_file_path, dyn_load_lib::dlopen_flags::lazy},
      m_module{nullptr, nullptr},
      m_thread_comm_producer{
          thread_comm::make<module_api::block, thread_comm::producer>(
              {m_k_thread_comm_state_change_callback,
               static_cast<void *>(&m_state_change_callback)},
              m_k_state_change_subscribed_events)},
      m_thread_comm_consumer{thread_comm::make_from<thread_comm::consumer>(
          m_thread_comm_producer)},
      m_thread_comm_producer_module{
          thread_comm::make_from<thread_comm::producer>(
              m_thread_comm_consumer)},
      m_module_api{&m_thread_comm_producer_module}, m_module_thread{} {
  do_ctor(std::move(conf));
}

i3neostatus::module_handle::module_handle(
    const module_id::type id, const std::string &file_path,
    const libconfigfile::map_node &conf,
    const state_change_callback &state_change_callback)
    : m_id{id}, m_name{}, m_file_path{file_path}, m_click_events_enabled{},
      m_state_change_callback{state_change_callback},
      m_dyn_lib{m_file_path, dyn_load_lib::dlopen_flags::lazy},
      m_module{nullptr, nullptr},
      m_thread_comm_producer{
          thread_comm::make<module_api::block, thread_comm::producer>(
              {m_k_thread_comm_state_change_callback,
               static_cast<void *>(&m_state_change_callback)},
              m_k_state_change_subscribed_events)},
      m_thread_comm_consumer{thread_comm::make_from<thread_comm::consumer>(
          m_thread_comm_producer)},
      m_thread_comm_producer_module{
          thread_comm::make_from<thread_comm::producer>(
              m_thread_comm_consumer)},
      m_module_api{&m_thread_comm_producer_module}, m_module_thread{} {
  do_ctor(std::remove_cvref_t<decltype(conf)>{conf});
}

i3neostatus::module_handle::module_handle(module_handle &&other) noexcept
    : m_id{other.m_id}, m_name{std::move(other.m_name)},
      m_file_path{std::move(other.m_file_path)},
      m_click_events_enabled{other.m_click_events_enabled},
      m_state_change_callback{std::move(other.m_state_change_callback)},
      m_dyn_lib{std::move(other.m_dyn_lib)},
      m_module{std::move(other.m_module)},
      m_thread_comm_producer{std::move(other.m_thread_comm_producer)},
      m_thread_comm_consumer{std::move(other.m_thread_comm_consumer)},
      m_thread_comm_producer_module{
          std::move(other.m_thread_comm_producer_module)},
      m_module_api{std::move(other.m_module_api)},
      m_module_thread{std::move(other.m_module_thread)} {}

i3neostatus::module_handle::~module_handle() {
  try {
    m_module->term();
  } catch (const std::exception &ex) {
    m_thread_comm_producer.put_exception(std::make_exception_ptr(
        module_error{m_id, m_name, m_file_path, ex.what()}));
  } catch (...) {
    m_thread_comm_producer.put_exception(std::make_exception_ptr(
        module_error{m_id, m_name, m_file_path, "UNKNOWN"}));
  }
  m_module_thread.join();
}

i3neostatus::module_handle &
i3neostatus::module_handle::operator=(module_handle &&other) noexcept {
  if (this != &other) {
    m_id = other.m_id;
    m_name = std::move(other.m_name);
    m_file_path = std::move(other.m_file_path);
    m_click_events_enabled = other.m_click_events_enabled;
    m_state_change_callback = std::move(other.m_state_change_callback);
    m_dyn_lib = std::move(other.m_dyn_lib);
    m_module = std::move(other.m_module);
    m_thread_comm_producer = std::move(other.m_thread_comm_producer);
    m_thread_comm_consumer = std::move(other.m_thread_comm_consumer);
    m_thread_comm_producer_module =
        std::move(other.m_thread_comm_producer_module);
    m_module_api = std::move(other.m_module_api);
    m_module_thread = std::move(other.m_module_thread);
  }
  return *this;
}

void i3neostatus::module_handle::do_ctor(libconfigfile::map_node &&conf) {
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

  module_api mod_api{&m_thread_comm_producer_module};

  try {
    module_api::config_out conf_out{
        m_module->init(&m_module_api, std::move(conf))};
    m_name = std::move(conf_out.name);
    m_click_events_enabled = conf_out.click_events_enabled;

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

void i3neostatus::module_handle::run() {
  m_module_thread = std::thread{[this]() {
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

void i3neostatus::module_handle::send_click_event(
    module_api::click_event &&click_event) {
  try {
    m_module->on_click_event(std::move(click_event));
  } catch (const std::exception &ex) {
    m_thread_comm_producer.put_exception(std::make_exception_ptr(
        module_error{m_id, m_name, m_file_path, ex.what()}));
  } catch (...) {
    m_thread_comm_producer.put_exception(std::make_exception_ptr(
        module_error{m_id, m_name, m_file_path, "UNKNOWN"}));
  }
}

i3neostatus::module_id::type i3neostatus::module_handle::get_id() const {
  return m_id;
}

const std::string &i3neostatus::module_handle::get_name() const {
  return m_name;
}

const std::string &i3neostatus::module_handle::get_file_path() const {
  return m_file_path;
}

bool i3neostatus::module_handle::get_click_events_enabled() const {
  return m_click_events_enabled;
}

i3neostatus::thread_comm::consumer<i3neostatus::module_api::block> &
i3neostatus::module_handle::get_comm() {
  return m_thread_comm_consumer;
}
